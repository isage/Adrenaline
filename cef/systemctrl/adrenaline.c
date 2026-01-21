/*
	Adrenaline
	Copyright (C) 2016-2018, TheFloW

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <string.h>

#include <pspinit.h>
#include <pspkermit.h>
#include <pspsysmem.h>
#include <pspsysevent.h>
#include <pspiofilemgr.h>
#include <pspsysmem_kernel.h>

#include <cfwmacros.h>
#include <pspextratypes.h>
#include <systemctrl_adrenaline.h>

#include <adrenaline_log.h>

#include "main.h"
#include "binary.h"

#include "../../adrenaline_compat.h"

typedef struct {
	void *sasCore;
	int grainSamples;
	int maxVoices;
	int outMode;
	int sampleRate;
} SasInitArguments;

static SasInitArguments g_sas_args;
static int g_sas_inited = 0;


static SceUID adrenaline_semaid = -1;

static int (* _scePowerSuspendOperation)(int a1);

static int (* SetFlag1)();
static int (* SetFlag2)();
static int (* sceKermitSyncDisplay)();

static int (* uiResumePoint)(u32 *data);
static void (* VitaSync)();

static int (* sceSasCoreInit)();
static int (* sceSasCoreExit)();

static int (* __sceSasInit)(void *sasCore, int grainSamples, int maxVoices, int outMode, int sampleRate);

SceAdrenaline *g_adrenaline = (SceAdrenaline *)ADRENALINE_ADDRESS;

int SendAdrenalineCmd(int cmd, u32 args) {
	int k1 = pspSdkSetK1(0);

	char buf[sizeof(SceKermitRequest) + 0x40];
	SceKermitRequest *request_aligned = (SceKermitRequest *)ALIGN((u32)buf, 0x40);
	SceKermitRequest *request_uncached = (SceKermitRequest *)((u32)request_aligned | 0x20000000);
	sceKernelDcacheInvalidateRange(request_aligned, sizeof(SceKermitRequest));

	u64 resp;
	sceKermitSendRequest(request_uncached, KERMIT_MODE_EXTRA_2, cmd, args, 0, &resp);

	pspSdkSetK1(k1);
	return resp;
}

int getSfoTitle(char *title, int n) {
	SceUID fd = -1;
	int size = 0;

	memset(title, 0, n);

	int bootfrom = sceKernelBootFrom();
	if (bootfrom == PSP_BOOT_DISC) {
        fd = sceIoOpen("disc0:/PSP_GAME/PARAM.SFO", PSP_O_RDONLY, 0);
		if (fd < 0) {
			return fd;
		}

		size = sceIoLseek(fd, 0, PSP_SEEK_END);
		sceIoLseek(fd, 0, PSP_SEEK_SET);
	} else if (bootfrom == PSP_BOOT_MS) {
		char *filename = sceKernelInitFileName();
		if (!filename) {
			return -1;
		}

		fd = sceIoOpen(filename, PSP_O_RDONLY, 0);
		if (fd < 0) {
			return fd;
		}

		PBPHeader pbp_header;
		sceIoRead(fd, &pbp_header, sizeof(PBPHeader));

		if (pbp_header.magic == PBP_MAGIC) {
			size = pbp_header.icon0_offset-pbp_header.param_offset;
			sceIoLseek(fd, pbp_header.param_offset, PSP_SEEK_SET);
		} else {
			sceIoClose(fd);
			return -2;
		}
	}

	// Allocate buffer
	SceUID blockid = sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_KERNEL, "", PSP_SMEM_Low, size, NULL);
	if (blockid < 0)
		return blockid;

	void *sfo = sceKernelGetBlockHeadAddr(blockid);

	// Read file
	sceIoRead(fd, sfo, size);
	sceIoClose(fd);

	// Get SFO title
	SFOHeader *header = (SFOHeader *)sfo;
	SFODir *entries = (SFODir *)(sfo + sizeof(SFOHeader));

	for (int i = 0; i < header->nitems; i++) {
		if (strcmp(sfo + header->fields_table_offs + entries[i].field_offs, "TITLE") == 0) {
			strncpy(title, sfo + header->values_table_offs + entries[i].val_offs, n);
			break;
		}
	}

	sceKernelFreePartitionMemory(blockid);

	return 0;
}

void initAdrenalineInfo() {
	memset(g_adrenaline, 0, sizeof(SceAdrenaline));

	int keyconfig = sceKernelApplicationType();
	if (keyconfig == PSP_INIT_KEYCONFIG_GAME || keyconfig == PSP_INIT_KEYCONFIG_POPS) {
		getSfoTitle(g_adrenaline->title, 128);
	} else if (keyconfig == PSP_INIT_KEYCONFIG_VSH) {
		strcpy(g_adrenaline->title, "XMB\xE2\x84\xA2");
	} else {
		strcpy(g_adrenaline->title, "Unknown");
	}

	SceGameInfo *game_info = sceKernelGetGameInfo();
	if (game_info) {
		strcpy(g_adrenaline->titleid, game_info->title_id);
	}

	char *filename = sceKernelInitFileName();
	if (filename) {
		strcpy(g_adrenaline->filename, filename);
	}

	g_adrenaline->pops_mode = sceKernelApplicationType() == PSP_INIT_KEYCONFIG_POPS;
}

int adrenaline_interrupt() {
	// Signal adrenaline semaphore
	sceKernelSignalSema(adrenaline_semaid, 1);
	return 0;
}

int adrenaline_thread(SceSize args, void *argp) {
	while (1) {
		// Wait for semaphore signal
		sceKernelWaitSema(adrenaline_semaid, 1, NULL);

		switch (g_adrenaline->psp_cmd) {
			case ADRENALINE_PSP_CMD_REINSERT_MS:
				sceIoDevctl("fatms0:", 0x0240D81E, NULL, 0, NULL, 0);
				break;

			case ADRENALINE_PSP_CMD_SAVESTATE:
				g_adrenaline->savestate_mode = SAVESTATE_MODE_SAVE;
				_scePowerSuspendOperation(0x202);
				break;

			case ADRENALINE_PSP_CMD_LOADSTATE:
				g_adrenaline->savestate_mode = SAVESTATE_MODE_LOAD;
				_scePowerSuspendOperation(0x202);
				break;
		}
	}

	return 0;
}

int __sceSasInitPatched(void *sasCore, int grainSamples, int maxVoices, int outMode, int sampleRate) {
	g_sas_args.sasCore = sasCore;
	g_sas_args.grainSamples = grainSamples;
	g_sas_args.maxVoices = maxVoices;
	g_sas_args.outMode = outMode;
	g_sas_args.sampleRate = sampleRate;

	g_sas_inited = 1;

	return __sceSasInit(sasCore, grainSamples, maxVoices, outMode, sampleRate);
}

void ReInitSasCore() {
	if (__sceSasInit && g_sas_inited) {
		sceSasCoreExit();
		sceSasCoreInit();
		__sceSasInit(g_sas_args.sasCore, g_sas_args.grainSamples, g_sas_args.maxVoices, g_sas_args.outMode, g_sas_args.sampleRate);
	}
}

int SysEventHandler(int ev_id, char *ev_name, void *param, int *result) {
	// Resume completed
	if (ev_id == 0x400000) {
		if (g_adrenaline->savestate_mode != SAVESTATE_MODE_NONE) {
			g_adrenaline->savestate_mode = SAVESTATE_MODE_NONE;
			ReInitSasCore();

			if (g_adrenaline->pops_mode) {
				int (* sceKermitPeripheralInitPops)() = (void *)sctrlHENFindFunction("sceKermitPeripheral_Driver", "sceKermitPeripheral", 0xC0EBC631);
				if (sceKermitPeripheralInitPops) {
					sceKermitPeripheralInitPops();
				}
			}
		}
	}

	return 0;
}

void VitaSyncPatched() {
	if (g_adrenaline->savestate_mode != SAVESTATE_MODE_NONE) {
		void (* SaveStateBinary)() = (void *)0x00010000;
		memcpy((void *)SaveStateBinary, binary, size_binary);
		sctrlFlushCache();

		SaveStateBinary();

		// Param for uiResumePoint
		u32 data[53];
		memset(data, 0, sizeof(data));
		data[0] = sizeof(data);
		data[8] = 0xFFFF;
		data[9] = 0x2;
		data[12] = 0x4B0;
		uiResumePoint(data);

		while(1);
	}

	VitaSync();
}

int SetFlag1Patched() {
	if (g_adrenaline->savestate_mode != SAVESTATE_MODE_NONE) {
		return 0;
	}

	return SetFlag1();
}

int SetFlag2Patched() {
	if (g_adrenaline->savestate_mode != SAVESTATE_MODE_NONE) {
		return 0;
	}

	return SetFlag2();
}

int sceKermitSyncDisplayPatched() {
	if (g_adrenaline->savestate_mode != SAVESTATE_MODE_NONE) {
		return 0;
	}

	return sceKermitSyncDisplay();
}

void PatchSasCore(SceModule* mod) {
	sceSasCoreInit = (void *)sctrlHENFindFunctionInMod(mod, "sceSasCore_driver", 0xB0F9F98F);
	sceSasCoreExit = (void *)sctrlHENFindFunctionInMod(mod, "sceSasCore_driver", 0xE143A1EA);

	HIJACK_FUNCTION(sctrlHENFindFunctionInMod(mod, "sceSasCore", 0x42778A9F), __sceSasInitPatched, __sceSasInit);

	sctrlFlushCache();
}

void PatchLowIODriver2(SceModule* mod) {
	u32 text_addr = mod->text_addr;

	HIJACK_FUNCTION(text_addr + 0x880, SetFlag1Patched, SetFlag1);
	HIJACK_FUNCTION(text_addr + 0xCD8, SetFlag2Patched, SetFlag2);
	HIJACK_FUNCTION(sctrlHENFindFunction("sceKermit_Driver", "sceKermit_driver", 0xD69C50BB), sceKermitSyncDisplayPatched, sceKermitSyncDisplay);

	sctrlFlushCache();
}

void PatchPowerService2(SceModule* mod) {
	u32 text_addr = mod->text_addr;

	// Patch to inject binary and to call uiResumePoint
	uiResumePoint = (void *)text_addr + 0x24C0;
	K_HIJACK_CALL(text_addr + 0x22FC, VitaSyncPatched, VitaSync);

	_scePowerSuspendOperation = (void *)text_addr + 0x1710;

	sctrlFlushCache();
}

int initAdrenaline() {
	// Register sysevent handler
	static PspSysEventHandler event_handler = {
		sizeof(PspSysEventHandler),
		"EPI_SysEvent",
		0x00FFFF00,
		SysEventHandler
	};

	sceKernelRegisterSysEventHandler(&event_handler);

	// Register adrenaline interrupt
	sceKermitRegisterVirtualIntrHandler(KERMIT_VIRTUAL_INTR_IMPOSE_CH1, adrenaline_interrupt);

	// Create adrenaline semaphore
	adrenaline_semaid = sceKernelCreateSema("EPI_Semaphore", 0, 0, 1, NULL);
	if (adrenaline_semaid < 0) {
		return adrenaline_semaid;
	}

	// Create and start adrenaline thread
	SceUID thid = sceKernelCreateThread("EPI_Thread", adrenaline_thread, 0x10, 0x4000, 0, NULL);
	if (thid < 0) {
		return thid;
	}

	sceKernelStartThread(thid, 0, NULL);

  *(u32 *)DRAW_NATIVE = 0;

	return 0;
}