/*
	Adrenaline
	Copyright (C) 2016-2018, TheFloW
	Copyright (C) 2025, GrayJack

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

/**
 * Module patches for system modules
 */

#include <string.h>

#include <pspumd.h>
#include <pspctrl.h>
#include <psperror.h>
#include <pspextratypes.h>
#include <pspsysmem_kernel.h>

#include <bootloadex.h>
#include <systemctrl.h>
#include <systemctrl_se.h>

#include <adrenaline_log.h>

#include "externs.h"
#include <systemctrl_adrenaline.h>

#include "rebootex.h"
#include "plugin.h"

#define EXIT_TO_VSH_MASK (PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER | PSP_CTRL_SELECT | PSP_CTRL_DOWN)
#define EXIT_TO_VSH2_MASK (PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER | PSP_CTRL_START | PSP_CTRL_DOWN)

STMOD_HANDLER g_module_handler = NULL;


static int (* RunReboot)(u32 *params) = NULL;
static int (* DecodeKL4E)(void *dest, u32 size_dest, void *src, u32 size_src) = NULL;

static int (* _sceChkregGetPsCode)(u8 *pscode) = NULL;
static int (* _sceSystemFileGetIndex)(void *sfo, void *a1, void *a2) = NULL;
static int (*_sceCtrlPeekBufferPositive)(SceCtrlData *pad_data, int count) = NULL;
static int (*_sceCtrlPeekBufferNegative)(SceCtrlData *pad_data, int count) = NULL;
static int (*_sceCtrlReadBufferPositive)(SceCtrlData *pad_data, int count) = NULL;
static int (*_sceCtrlReadBufferNegative)(SceCtrlData *pad_data, int count) = NULL;
static int (* _scePowerSetClockFrequency_k)(int cpufreq, int ramfreq, int busfreq) = NULL;


////////////////////////////////////////////////////////////////////////////////
// HELPERS
////////////////////////////////////////////////////////////////////////////////

static u32 FindPowerFunction(u32 nid) {
	return sctrlHENFindFunction("scePower_Service", "scePower", nid);
}

static int protect_pspemu_mem() {
	u32 ram2 = g_rebootex_config.ram2;
	u32 ram11 = g_rebootex_config.ram11;

	// Needs to protect FLASH0 VRAM
	if (ram2 + ram11 >= 49) {
		int partition = (ram2 >= 49) ? PSP_MEMORY_PARTITION_USER : 11;
		int res = sceKernelAllocPartitionMemory(partition, "SCE_PSPEMU_FLASH0", PSP_SMEM_Addr, 1024*1024, (void*)SCE_PSPEMU_FLASH0_RAMFS);

		if (res < 0) {
			return res;
		}
	}

	return 0;
}

int ApplyMemory() {
	if (g_rebootex_config.ram2 != 0 && (g_rebootex_config.ram2 + g_rebootex_config.ram11) <= 52) {
		PspSysMemPartition *(* GetPartition)(int partition) = NULL;
		PspSysMemPartition *partition;
		u32 user_size;

		for (u32 i = 0; i < 0x4000; i += 4) {
			u32 addr = 0x88000000 + i;
			u32 data = VREAD32(addr);

			if (data == 0x2C85000D) {
				GetPartition = (void *)(addr - 4);
				break;
			}
		}

		if (!GetPartition) {
			return SCE_KERR_ILLEGAL_ADDR;
		}

		user_size = (g_rebootex_config.ram2 * 1024 * 1024);
		partition = GetPartition(PSP_MEMORY_PARTITION_USER);
		partition->size = user_size;
		partition->data->size = (((user_size >> 8) << 9) | 0xFC);

		partition = GetPartition(11);
		partition->size = (g_rebootex_config.ram11 * 1024 * 1024);
		partition->address = 0x88800000 + user_size;
		partition->data->size = (((partition->size >> 8) << 9) | 0xFC);

		// Protects the regions with pspemu addresses
		protect_pspemu_mem();
	}

	return 0;
}

void ApplyAndResetMemory() {
	ApplyMemory();
	g_rebootex_config.ram2 = 0;
}

void UnprotectExtraMemory() {
	u32 *prot = (u32 *)0xBC000040;

	for (int i = 0; i < 0x10; i++) {
		prot[i] = 0xFFFFFFFF;
	}
}

static int exitToVsh(SceSize args, void *argp) {
    int k1 = pspSdkSetK1(0);

    // Refuse operation in Save dialog
    if(sceKernelFindModuleByName("sceVshSDUtility_Module") != NULL) {
		return 0;
	}

    // Refuse operation in Dialog
    if(sceKernelFindModuleByName("sceDialogmain_Module") != NULL) {
		return 0;
	}

    int (*_sceDisplaySetHoldMode)(int) = (void*)sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0x7ED59BC4);
    if (_sceDisplaySetHoldMode) _sceDisplaySetHoldMode(0);

    // reset some flags
    SetUmdFile("");
    sctrlSESetBootConfFileIndex(MODE_UMD);

    int res = sctrlKernelExitVSH(NULL);

    pspSdkSetK1(k1);
    return res;
}

static void startExitThread(){
	int k1 = pspSdkSetK1(0);
	int intc = pspSdkDisableInterrupts();
	if (sctrlGetThreadUIDByName("ExitGamePollThread") >= 0){
		pspSdkEnableInterrupts(intc);
		return; // already exiting
	}
	int uid = sceKernelCreateThread("ExitGamePollThread", exitToVsh, 1, 4096, 0, NULL);
	pspSdkEnableInterrupts(intc);
	sceKernelStartThread(uid, 0, NULL);
	sceKernelWaitThreadEnd(uid, NULL);
	sceKernelDeleteThread(uid);
	pspSdkSetK1(k1);
}

/** Checks controller input and modifies CFW context when certain buttons are hold at the start of an application */
void CheckControllerInput() {
	SceCtrlData pad_data;
	_sceCtrlPeekBufferPositive(&pad_data, 1);
	if ((pad_data.Buttons & PSP_CTRL_LTRIGGER) == PSP_CTRL_LTRIGGER) {
		g_disable_plugins = 1;
		logmsg2("[INFO]: Plugins disabled by holding `L` at the application start\n");
	}
}

////////////////////////////////////////////////////////////////////////////////
// CFW API (That depends on this file globals/statics)
////////////////////////////////////////////////////////////////////////////////

int sctrlHENSetMemory(u32 p2, u32 p11) {
	if ((p2 == 0) || ((p2 + p11) > 52)) {
		return SCE_ERR_INMODE;
	}

	// Disallow setting after game boot
	if (sctrlHENIsSystemBooted()) {
		return -3;
	};

	// Do not allow in pops and vsh
	int apitype = sceKernelInitApitype();
	if (apitype == PSP_INIT_APITYPE_MS5 || apitype == PSP_INIT_APITYPE_EF5 || apitype >= PSP_INIT_APITYPE_VSH_KERNEL) {
		return -1;
	}

	// Checks for unlock state
	if (p2 > 24) {
		// already enabled
		if (g_rebootex_config.ram2 > 24) {
			return -2;
		}
	} else if (p2 == 24) { // Checks for default state
		// already enabled
		if (g_rebootex_config.ram2 == 24) {
			return -2;
		}
	}

	int k1 = pspSdkSetK1(0);

	g_rebootex_config.ram2 = p2;
	g_rebootex_config.ram11 = p11;

	pspSdkSetK1(k1);
	return 0;
}

// ARK CFW compat
int sctrlHENApplyMemory(u32 p2) {
	int p11 = 40 - p2;
	int res = sctrlHENSetMemory(p2, (p11 < 0)? 0 : p11);

	// It can't unlock
	if (res < 0) {
		return res;
	}

	return 0;
}

void sctrlHENSetSpeed(int cpu, int bus) {
	int k1 = pspSdkSetK1(0);
	if (cpu == 20 || cpu == 75 || cpu == 100 || cpu == 133 || cpu == 333 || cpu == 300 || cpu == 266 || cpu == 222) {
		_scePowerSetClockFrequency_k = (void *)FindPowerFunction(0x737486F2);
		_scePowerSetClockFrequency_k(cpu, cpu, bus);

		if (sceKernelApplicationType() != PSP_INIT_KEYCONFIG_VSH) {
			MAKE_DUMMY_FUNCTION((u32)_scePowerSetClockFrequency_k, 0);
			MAKE_DUMMY_FUNCTION((u32)FindPowerFunction(0x545A7F3C), 0);
			MAKE_DUMMY_FUNCTION((u32)FindPowerFunction(0xB8D7B3FB), 0);
			MAKE_DUMMY_FUNCTION((u32)FindPowerFunction(0x843FBF43), 0);
			MAKE_DUMMY_FUNCTION((u32)FindPowerFunction(0xEBD177D6), 0);
			sctrlFlushCache();
		}
	}
	pspSdkSetK1(k1);
}

////////////////////////////////////////////////////////////////////////////////
// PATCHED IMPLEMENTATIONS
////////////////////////////////////////////////////////////////////////////////

int sceSystemFileGetIndexPatched(void *sfo, void *a1, void *a2) {
	int largememory = 0;

	if (g_rebootex_config.ram2 == 0) {
		SFOHeader *header = (SFOHeader *)sfo;
		SFODir *entries = (SFODir *)(sfo + sizeof(SFOHeader));

		for (int i = 0; i < header->nitems; i++) {
			if (strcmp(sfo + header->fields_table_offs + entries[i].field_offs, "MEMSIZE") == 0) {
				memcpy(&largememory, sfo + header->values_table_offs + entries[i].val_offs, 4);
			}
		}

		if (largememory == 1) {
			sctrlHENSetMemory(52, 0);
			ApplyMemory();
		} else if (largememory == 2) {
			sctrlHENSetMemory(40, 12);
			ApplyMemory();
		} else if (largememory > 2) {
			// Invalid value, but just to be safe, unlock stable extra RAM, but
			// left in a state that can be re-set
			sctrlHENSetMemory(40, 12);
			ApplyAndResetMemory();
		}
	} else {
		ApplyAndResetMemory();
	}

	return _sceSystemFileGetIndex(sfo, a1, a2);
}

int RunRebootPatched(u32 *params) {
	if ((char *)params[2] == NULL) {
		if (g_rebootex_config.bootfileindex != MODE_RECOVERY) {
			g_rebootex_config.bootfileindex = MODE_UMD;
		}
		memset(g_rebootex_config.umdfilename, 0, 256);
	}

	return RunReboot(params);
}

int DecodeKL4EPatched(void *dest, u32 size_dest, void *src, u32 size_src) {
	memcpy((void *)REBOOTEX_TEXT, rebootex, size_rebootex);
	memcpy((void *)REBOOTEX_CONFIG, &g_rebootex_config, sizeof(RebootexConfigEPI));
	// Backup the CFW config across reboot
	memcpy((void *)EPI_CONFIG_ADDR, &g_cfw_config, sizeof(SEConfigEPI));
	return DecodeKL4E(dest, size_dest, src, size_src);
}

int sceChkregGetPsCodePatched(u8 *pscode) {
	int res = _sceChkregGetPsCode(pscode);

	pscode[0] = 0x01;
	pscode[1] = 0x00;

	if (g_cfw_config.fake_region) {
		pscode[2] = g_cfw_config.fake_region < 12 ? g_cfw_config.fake_region + 2 : g_cfw_config.fake_region - 11;
		if (pscode[2] == 2) {
			pscode[2] = 3;
		}
	}

	pscode[3] = 0x00;
	pscode[4] = 0x01;
	pscode[5] = 0x00;
	pscode[6] = 0x01;
	pscode[7] = 0x00;

	return res;
}

int sceCtrlPeekBufferPositivePatched(SceCtrlData *pad_data, int count) {
	if (_sceCtrlPeekBufferPositive == NULL) {
		return SCE_KERR_ILLEGAL_ADDR;
	}

	count = _sceCtrlPeekBufferPositive(pad_data, count);

	if ((pad_data->Buttons & EXIT_TO_VSH_MASK) == EXIT_TO_VSH_MASK || (pad_data->Buttons & EXIT_TO_VSH2_MASK) == EXIT_TO_VSH2_MASK) {
		startExitThread();
	}

	return count;
}

int sceCtrlPeekBufferNegativePatched(SceCtrlData *pad_data, int count) {
	if (_sceCtrlPeekBufferNegative == NULL) {
		return SCE_KERR_ILLEGAL_ADDR;
	}

	count = _sceCtrlPeekBufferNegative(pad_data, count);

	if ((pad_data->Buttons & EXIT_TO_VSH_MASK) == 0 || (pad_data->Buttons & EXIT_TO_VSH2_MASK) == 0) {
		startExitThread();
	}

	return count;
}

int sceCtrlReadBufferPositivePatched(SceCtrlData *pad_data, int count) {
	if (_sceCtrlReadBufferPositive == NULL) {
		return SCE_KERR_ILLEGAL_ADDR;
	}

	count = _sceCtrlReadBufferPositive(pad_data, count);

	if ((pad_data->Buttons & EXIT_TO_VSH_MASK) == EXIT_TO_VSH_MASK || (pad_data->Buttons & EXIT_TO_VSH2_MASK) == EXIT_TO_VSH2_MASK) {
		startExitThread();
	}

	return count;
}

int sceCtrlReadBufferNegativePatched(SceCtrlData *pad_data, int count) {
	if (_sceCtrlReadBufferNegative == NULL) {
		return SCE_KERR_ILLEGAL_ADDR;
	}

	count = _sceCtrlReadBufferNegative(pad_data, count);

	if ((pad_data->Buttons & EXIT_TO_VSH_MASK) == 0 || (pad_data->Buttons & EXIT_TO_VSH2_MASK) == 0) {
		startExitThread();
	}

	return count;
}

////////////////////////////////////////////////////////////////////////////////
// MODULE PATCHERS
////////////////////////////////////////////////////////////////////////////////

void PatchLoadExec(SceModule* mod) {
	u32 text_addr = mod->text_addr;
	u32 text_size = mod->text_size;

	// Allow loadexec in whatever user level. Ignore K1 Check
	VWRITE16(text_addr + 0x16A6, 0x1000);
	VWRITE16(text_addr + 0x241E, 0x1000);
	VWRITE16(text_addr + 0x2622, 0x1000);

	for (int i = 0; i < text_size; i += 4) {
		u32 addr = text_addr + i;
		u32 data = VREAD32(addr);

		// Patch to do things before reboot
		if (data == 0x02202021 && VREAD32(addr + 4) == 0x00401821) {
			K_HIJACK_CALL(addr - 4, RunRebootPatched, RunReboot);
			continue;
		}

		// Redirect pointer to 0x88FC0000
		if (data == 0x04400020) {
			K_HIJACK_CALL(addr - 8, DecodeKL4EPatched, DecodeKL4E);
			VWRITE8(addr + 0x44, 0xFC);
			continue;
		}
	}

	sctrlFlushCache();
}

void PatchChkreg() {
	HIJACK_FUNCTION(K_EXTRACT_IMPORT(&sceChkregGetPsCode), sceChkregGetPsCodePatched, _sceChkregGetPsCode);
	sctrlFlushCache();
}

void PatchMediaSync(SceModule* mod) {
	u32 text_addr = mod->text_addr;

	// Dummy function that checks flash0 files
	MAKE_INSTRUCTION(text_addr + 0xC8, 0x00001021);

	// Fixes: ELF boot, boot not from /PSP/GAME/
	MAKE_INSTRUCTION(text_addr + 0x864, 0x00008821);
	MAKE_INSTRUCTION(text_addr + 0x988, 0x00008821);

	// Avoid SCE_MEDIASYNC_ERROR_INVALID_MEDIA
	VWRITE16(text_addr + 0x3C6, 0x5000);
	VWRITE16(text_addr + 0xDCA, 0x1000);

	K_HIJACK_CALL(text_addr + 0x97C, sceSystemFileGetIndexPatched, _sceSystemFileGetIndex);

	sctrlFlushCache();
}

void PatchController(SceModule* mod) {
	_sceCtrlPeekBufferPositive = (void*)sctrlHENFindFunctionInMod(mod, "sceCtrl_driver", 0x3A622550);
	_sceCtrlPeekBufferNegative = (void*)sctrlHENFindFunctionInMod(mod, "sceCtrl_driver", 0xC152080A);
	_sceCtrlReadBufferPositive = (void*)sctrlHENFindFunctionInMod(mod, "sceCtrl_driver", 0x1F803938);
	_sceCtrlReadBufferNegative = (void*)sctrlHENFindFunctionInMod(mod, "sceCtrl_driver", 0x60B81F86);

	HIJACK_FUNCTION(_sceCtrlPeekBufferPositive, sceCtrlPeekBufferPositivePatched, _sceCtrlPeekBufferPositive);
	HIJACK_FUNCTION(_sceCtrlPeekBufferNegative, sceCtrlPeekBufferNegativePatched, _sceCtrlPeekBufferNegative);
	HIJACK_FUNCTION(_sceCtrlReadBufferPositive, sceCtrlReadBufferPositivePatched, _sceCtrlReadBufferPositive);
	HIJACK_FUNCTION(_sceCtrlReadBufferNegative, sceCtrlReadBufferNegativePatched, _sceCtrlReadBufferNegative);

	sctrlFlushCache();
}

void PatchInterruptMgr() {
	SceModule *mod = sceKernelFindModuleByName("sceInterruptManager");
	u32 text_addr = mod->text_addr;

	// Allow execution of syscalls in kernel mode
	MAKE_INSTRUCTION(text_addr + 0xE98, 0x408F7000);
	MAKE_NOP(text_addr + 0xE9C);
}

void PatchSysmem() {
	u32 nids[] = { 0x7591C7DB, 0x342061E5, 0x315AD3A0, 0xEBD5C3E6, 0x057E7380, 0x91DE343C, 0x7893F79A, 0x35669D4C, 0x1B4217BC, 0x358CA1BB };

	SceModule *mod = sceKernelFindModuleByName("sceSystemMemoryManager");
	for (int i = 0; i < sizeof(nids) / sizeof(u32); i++) {
		u32 addr = sctrlHENFindFirstBEQ(sctrlHENFindFunctionInMod(mod, "SysMemUserForUser", nids[i]));
		if (addr) {
			VWRITE16(addr + 2, 0x1000);
		}
	}

	extern SceSize (* _sceKernelMaxFreeMemSize)(void);
	extern SceSize (* _sceKernelTotalFreeMemSize)(void);

	_sceKernelMaxFreeMemSize = (void*)sctrlHENFindFunctionInMod(mod, "SysMemUserForUser", 0xA291F107);
	_sceKernelTotalFreeMemSize = (void*)sctrlHENFindFunctionInMod(mod, "SysMemUserForUser", 0xF919F628);
}
