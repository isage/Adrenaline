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

#include <psperror.h>
#include <pspsysmem_kernel.h>
#include <pspextratypes.h>

#include <cfwmacros.h>
#include <bootloadex.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <rebootexconfig.h>
#include <systemctrl_adrenaline.h>

#include "libc.h"

#define REBOOT_MODULE "/rtm.prx"

int (* sceReboot)(void *reboot_param, SceKernelLoadExecVSHParam *vsh_param, int api, int initial_rnd) = (void *)0x88600000;
int (* DcacheClear)(void) = (void *)0x886018AC;
int (* IcacheClear)(void) = (void *)0x88601E40;

int (* DecryptExecutable)(void *buf, int size, int *retSize);

void (* SetMemoryPartitionTable)(void *sysmem_config, SceSysmemPartTable *table);
int (* sceKernelBootLoadFile)(BootFile *file, void *a1, void *a2, void *a3, void *t0);

RebootexConfigEPI *g_rebootex_config = (RebootexConfigEPI *)REBOOTEX_CONFIG;

void ClearCaches() {
	DcacheClear();
	IcacheClear();
}

void SetMemoryPartitionTablePatched(void *sysmem_config, SceSysmemPartTable *table) {
	SetMemoryPartitionTable(sysmem_config, table);

	// Add partition 11
	table->extVshell.addr = PSP_EXTRA_RAM;
	table->extVshell.size = 16 * 1024 * 1024;
}

int PatchSysMem(void *a0, void *sysmem_config) {
	int (* module_bootstart)(SceSize args, void *sysmem_config) = (void *)VREAD32((u32)a0 + 0x28);

	for (int i = 0; i < 0x14000; i += 4) {
		u32 addr = 0x88000000 + i;

		// Patch to add new partition
		if (VREAD32(addr) == 0x14600003) {
			K_HIJACK_CALL(addr - 0x1C, SetMemoryPartitionTablePatched, SetMemoryPartitionTable);
			continue;
		}
	}

	ClearCaches();

	return module_bootstart(4, sysmem_config);
}

int DecryptExecutablePatched(void *buf, int size, int *retSize) {
	if (*(u16 *)((u32)buf + 0x150) == 0x8B1F) {
		*retSize = *(u32 *)((u32)buf + 0xB0);
		_memcpy(buf, (void *)((u32)buf + 0x150), *retSize);
		return 0;
	}

	return DecryptExecutable(buf, size, retSize);
}

int PatchLoadCore(int (* module_bootstart)(SceSize args, void *argp), void *argp) {
	u32 text_addr = ((u32)module_bootstart) - 0xAF8;

	for (u32 i = 0; i < 0x8000; i += 4) {
		u32 addr = text_addr + i;

		// Allow custom modules
		if (VREAD32(addr) == 0xAE2D0048) {
			DecryptExecutable = (void *)K_EXTRACT_CALL(addr + 8);
			MAKE_CALL(addr + 8, DecryptExecutablePatched);
			break;
		}
	}

	ClearCaches();

	return module_bootstart(8, argp);
}

int InsertModule(void *buf, char *new_module, char *module_after, int flags) {
	BtcnfHeader *header = (BtcnfHeader *)buf;

	ModuleEntry *modules = (ModuleEntry *)((u32)header + header->modulestart);
	ModeEntry *modes = (ModeEntry *)((u32)header + header->modestart);

	char *modnamestart = (char *)((u32)header + header->modnamestart);
	char *modnameend = (char *)((u32)header + header->modnameend);

	if (header->signature != BTCNF_MAGIC)
		return -1;

	int i = 0;
	for (i = 0; i < header->nmodules; i++) {
		if (_strcmp(modnamestart + modules[i].stroffset, module_after) == 0) {
			break;
		}
	}

	if (i == header->nmodules)
		return -2;

	int len = _strlen(new_module) + 1;

	// Add new_module name at end
	_memcpy((void *)modnameend, (void *)new_module, len);

	// Move module_after forward
	_memmove(&modules[i + 1], &modules[i], (header->nmodules - i) * sizeof(ModuleEntry) + len + modnameend - modnamestart);

	// Add new_module information
	modules[i].stroffset = modnameend - modnamestart;
	modules[i].flags = flags;

	// Update header
	header->nmodules++;
	header->modnamestart += sizeof(ModuleEntry);
	header->modnameend += (len + sizeof(ModuleEntry));

	// Update modes
	int j;
	for (j = 0; j < header->nmodes; j++) {
		modes[j].maxsearch++;
	}

	return 0;
}

int sceKernelCheckPspConfigPatched(void *buf, int size, int flag) {
	if (g_rebootex_config->module_after) {
		InsertModule(buf, REBOOT_MODULE, g_rebootex_config->module_after, g_rebootex_config->flags);
	}

	return 0;
}

int sceKernelBootLoadFilePatched(BootFile *file, void *a1, void *a2, void *a3, void *t0) {
	if (_strcmp(file->name, "pspbtcnf.bin") == 0) {
		char *name = NULL;

		switch(g_rebootex_config->bootfileindex) {
			case MODE_UMD:
				name = "/kd/pspbtjnf.bin";
				break;

			case MODE_INFERNO:
				name = "/kd/pspbtknf.bin";
				break;

			case MODE_MARCH33:
				name = "/kd/pspbtlnf.bin";
				break;

			case MODE_NP9660:
				name = "/kd/pspbtmnf.bin";
				break;

			case MODE_RECOVERY:
				name = "/kd/pspbtrnf.bin";
				break;
		}

		if (g_rebootex_config->bootfileindex == MODE_RECOVERY) {
			g_rebootex_config->bootfileindex = MODE_UMD;
		}

		file->name = name;
	} else if (_strcmp(file->name, REBOOT_MODULE) == 0) {
		file->buffer = (void *)0x89000000;
		file->size = g_rebootex_config->size;
		_memcpy(file->buffer, g_rebootex_config->buf, file->size);
		return 0;
	}

	sceKernelBootLoadFile(file, a1, a2, a3, t0);

	return 0; //always return 0 to allow boot with unsuccessfully loaded files
}

int _start(void *reboot_param, SceKernelLoadExecVSHParam *vsh_param, int api, int initial_rnd) __attribute__((section(".text.start")));
int _start(void *reboot_param, SceKernelLoadExecVSHParam *vsh_param, int api, int initial_rnd) {
	for (u32 i = 0; i < 0x4000; i += 4) {
		u32 addr = 0x88600000 + i;
		u32 data = VREAD32(addr);

		// Patch call to SysMem module_bootstart
		if (data == 0x24040004) {
			MAKE_INSTRUCTION(addr, 0x02402021); // move $a0, $s2
			MAKE_CALL(addr + 0x64, PatchSysMem);
			continue;
		}

		// Patch call to LoadCore module_bootstart
		if (data == 0x00600008) {
			MAKE_INSTRUCTION(addr - 8, 0x00602021); // move $a0, $v1
			MAKE_JUMP(addr, PatchLoadCore);
			continue;
		}

		// Patch sceKernelCheckPspConfig
		if (data == 0x04400022) {
			MAKE_CALL(addr - 8, sceKernelCheckPspConfigPatched);
			continue;
		}

		// Patch sceKernelBootLoadFile
		if (data == 0xAFBF0000 && VREAD32(addr + 8) == 0x00000000) {
			sceKernelBootLoadFile = (void *)K_EXTRACT_CALL(addr + 4);
			MAKE_CALL(addr + 4, sceKernelBootLoadFilePatched);
			continue;
		}
	}

	ClearCaches();

	// Call original function
	return sceReboot(reboot_param, vsh_param, api, initial_rnd);
}