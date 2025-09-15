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

#include <common.h>
#include <adrenaline_log.h>

#include "main.h"
#include "adrenaline.h"
#include "../../adrenaline_compat.h"

#include "rebootex.h"
#include "plugin.h"

#define EXIT_TO_VSH_MASK (PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER | PSP_CTRL_SELECT | PSP_CTRL_DOWN)
#define EXIT_TO_VSH2_MASK (PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER | PSP_CTRL_START | PSP_CTRL_DOWN)

STMOD_HANDLER module_handler = NULL;


int (* RunReboot)(u32 *params) = NULL;
int (* DecodeKL4E)(void *dest, u32 size_dest, void *src, u32 size_src) = NULL;
int (* SetIdleCallback)(int flags) = NULL;

int (* _sceChkregGetPsCode)(u8 *pscode) = NULL;
int (* _sceSystemFileGetIndex)(void *sfo, void *a1, void *a2) = NULL;
int (*_sceCtrlPeekBufferPositive)(SceCtrlData *pad_data, int count) = NULL;
int (*_sceCtrlPeekBufferNegative)(SceCtrlData *pad_data, int count) = NULL;
int (*_sceCtrlReadBufferPositive)(SceCtrlData *pad_data, int count) = NULL;
int (*_sceCtrlReadBufferNegative)(SceCtrlData *pad_data, int count) = NULL;
int (* _scePowerSetClockFrequency_k)(int cpufreq, int ramfreq, int busfreq) = NULL;

typedef struct PartitionData {
	u32 unk[5];
	u32 size;
} PartitionData;

typedef struct SysMemPartition {
	struct SysMemPartition *next;
	u32	address;
	u32 size;
	u32 attributes;
	PartitionData *data;
} SysMemPartition;

////////////////////////////////////////////////////////////////////////////////
// HELPERS
////////////////////////////////////////////////////////////////////////////////

static u32 FindPowerFunction(u32 nid) {
	return FindProc("scePower_Service", "scePower", nid);
}

static int protect_pspemu_mem() {
	u32 ram2 = rebootex_config.ram2;
	u32 ram11 = rebootex_config.ram11;

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
	if (rebootex_config.ram2 != 0 && (rebootex_config.ram2 + rebootex_config.ram11) <= 52) {
		SysMemPartition *(* GetPartition)(int partition) = NULL;
		SysMemPartition *partition;
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

		user_size = (rebootex_config.ram2 * 1024 * 1024);
		partition = GetPartition(PSP_MEMORY_PARTITION_USER);
		partition->size = user_size;
		partition->data->size = (((user_size >> 8) << 9) | 0xFC);

		partition = GetPartition(11);
		partition->size = (rebootex_config.ram11 * 1024 * 1024);
		partition->address = 0x88800000 + user_size;
		partition->data->size = (((partition->size >> 8) << 9) | 0xFC);

		// Protects the regions with pspemu addresses
		protect_pspemu_mem();
	}

	return 0;
}

void ApplyAndResetMemory() {
	ApplyMemory();
	rebootex_config.ram2 = 0;
}

void UnprotectExtraMemory() {
	u32 *prot = (u32 *)0xBC000040;

	for (int i = 0; i < 0x10; i++) {
		prot[i] = 0xFFFFFFFF;
	}
}

static int exit_callback(int arg1, int arg2, void *common) {
	sceKernelSuspendAllUserThreads();
	SceAdrenaline *adrenaline = (SceAdrenaline *)ADRENALINE_ADDRESS;
	adrenaline->pops_mode = 0;
	SendAdrenalineCmd(ADRENALINE_VITA_CMD_RESUME_POPS);

	static u32 vshmain_args[0x100];
	memset(vshmain_args, 0, sizeof(vshmain_args));

	vshmain_args[0] = sizeof(vshmain_args);
	vshmain_args[1] = 0x20;
	vshmain_args[16] = 1;

	SceKernelLoadExecVSHParam param;

	memset(&param, 0, sizeof(param));
	param.size = sizeof(param);
	param.argp = NULL;
	param.args = 0;
	param.vshmain_args = vshmain_args;
	param.vshmain_args_size = sizeof(vshmain_args);
	param.key = "vsh";

	sctrlKernelExitVSH(&param);

	return 0;
}

static int CallbackThread(SceSize args, void *argp) {
	SceUID cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	if (cbid < 0) {
		return cbid;
	}

	int (* sceKernelRegisterExitCallback)(SceUID cbid) = (void *)FindProc("sceLoadExec", "LoadExecForUser", 0x4AC57943);
	sceKernelRegisterExitCallback(cbid);

	sceKernelSleepThreadCB();

	return 0;
}

static SceUID SetupCallbacks() {
	SceUID thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
	if (thid >= 0) {
		sceKernelStartThread(thid, 0, 0);
	}
	return thid;
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
    sctrlSESetBootConfFileIndex(BOOT_NORMAL);

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
		disable_plugins = 1;
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
	if (apitype == SCE_APITYPE_MS5 || apitype == SCE_APITYPE_EF5 || apitype >= SCE_APITYPE_VSH_KERNEL) {
		return -1;
	}

	// Checks for unlock state
	if (p2 > 24) {
		// already enabled
		if (rebootex_config.ram2 > 24) {
			return -2;
		}
	} else if (p2 == 24) { // Checks for default state
		// already enabled
		if (rebootex_config.ram2 == 24) {
			return -2;
		}
	}

	int k1 = pspSdkSetK1(0);

	rebootex_config.ram2 = p2;
	rebootex_config.ram11 = p11;

	pspSdkSetK1(k1);
	return 0;
}

// ARK-4 compat
int sctrlHENApplyMemory(u32 p2) {
	if (p2 > 52) {
		p2 = 52;
	}

	int res = sctrlHENSetMemory(p2, 52-p2);
	if (res < 0) {
		return res;
	}

	res = ApplyMemory();
	// Revert back on fail
	if (res < 0) {
		sctrlHENSetMemory(24, 16);
		ApplyAndResetMemory();
	}

	return res;
}

void SetSpeed(int cpu, int bus) {
	if (cpu == 20 || cpu == 75 || cpu == 100 || cpu == 133 || cpu == 333 || cpu == 300 || cpu == 266 || cpu == 222) {
		_scePowerSetClockFrequency_k = (void *)FindPowerFunction(0x737486F2);
		_scePowerSetClockFrequency_k(cpu, cpu, bus);

		if (sceKernelApplicationType() != SCE_APPTYPE_VSH) {
			MAKE_DUMMY_FUNCTION((u32)_scePowerSetClockFrequency_k, 0);
			MAKE_DUMMY_FUNCTION((u32)FindPowerFunction(0x545A7F3C), 0);
			MAKE_DUMMY_FUNCTION((u32)FindPowerFunction(0xB8D7B3FB), 0);
			MAKE_DUMMY_FUNCTION((u32)FindPowerFunction(0x843FBF43), 0);
			MAKE_DUMMY_FUNCTION((u32)FindPowerFunction(0xEBD177D6), 0);
			sctrlFlushCache();
		}
	}
}

void sctrlHENSetSpeed(int cpu, int bus) {
	int k1 = pspSdkSetK1(0);
	SetSpeed(cpu, bus);
	pspSdkSetK1(k1);
}

////////////////////////////////////////////////////////////////////////////////
// PATCHED IMPLEMENTATIONS
////////////////////////////////////////////////////////////////////////////////

int sceSystemFileGetIndexPatched(void *sfo, void *a1, void *a2) {
	int largememory = 0;

	if (rebootex_config.ram2 == 0) {
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
		if (rebootex_config.bootfileindex != BOOT_RECOVERY) {
			rebootex_config.bootfileindex = BOOT_NORMAL;
		}
		memset(rebootex_config.umdfilename, 0, 256);
	}

	return RunReboot(params);
}

int DecodeKL4EPatched(void *dest, u32 size_dest, void *src, u32 size_src) {
	memcpy((void *)0x88FC0000, rebootex, size_rebootex);
	memcpy((void *)0x88FB0000, &rebootex_config, sizeof(RebootexConfig));
	return DecodeKL4E(dest, size_dest, src, size_src);
}

int sceUmdRegisterUMDCallBackPatched(int cbid) {
	int k1 = pspSdkSetK1(0);
	int res = sceKernelNotifyCallback(cbid, PSP_UMD_NOT_PRESENT);
	pspSdkSetK1(k1);
	return res;
}

int sceChkregGetPsCodePatched(u8 *pscode) {
	int res = _sceChkregGetPsCode(pscode);

	pscode[0] = 0x01;
	pscode[1] = 0x00;

	if (config.fake_region) {
		pscode[2] = config.fake_region < 12 ? config.fake_region + 2 : config.fake_region - 11;
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

int SetIdleCallbackPatched(int flags) {
	// Only allow idle callback for music player sleep-timer
	if (flags & 8) {
		return SetIdleCallback(flags);
	}

	return 0;
}

int sceKernelWaitEventFlagPatched(int evid, u32 bits, u32 wait, u32 *outBits, SceUInt *timeout) {
	int res = sceKernelWaitEventFlag(evid, bits, wait, outBits, timeout);

	if (*outBits & 0x1) {
		SendAdrenalineCmd(ADRENALINE_VITA_CMD_PAUSE_POPS);
	} else if (*outBits & 0x2) {
		SendAdrenalineCmd(ADRENALINE_VITA_CMD_RESUME_POPS);
	}

	return res;
}

int memcmp_patched(const void *b1, const void *b2, size_t len) {
	u32 tag = 0x4C9494F0;
	if (memcmp(&tag, b2, len) == 0) {
		static u8 kernel661_keys[0x10] = { 0x76, 0xF2, 0x6C, 0x0A, 0xCA, 0x3A, 0xBA, 0x4E, 0xAC, 0x76, 0xD2, 0x40, 0xF5, 0xC3, 0xBF, 0xF9 };
		memcpy((void *)0xBFC00220, kernel661_keys, sizeof(kernel661_keys));
		return 0;
	}

	return memcmp(b1, b2, len);
}

int sceResmgrDecryptIndexPatched(void *buf, int size, int *retSize) {
	int k1 = pspSdkSetK1(0);
	*retSize = ReadFile("flash0:/vsh/etc/version.txt", buf, size);
	pspSdkSetK1(k1);
	return 0;
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

int (* _sceMeAudio_driver_C300D466)(int codec, int unk, void *info);
int sceMeAudio_driver_C300D466_Patched(int codec, int unk, void *info) {
	int res = _sceMeAudio_driver_C300D466(codec, unk, info);

	if (res < 0 && codec == 0x1002 && unk == 2) {
		return 0;
	}

	return res;
}

////////////////////////////////////////////////////////////////////////////////
// MODULE PATCHERS
////////////////////////////////////////////////////////////////////////////////

void PatchLoadExec(SceModule* mod) {
	u32 text_addr = mod->text_addr;
	u32 text_size = mod->text_size;

	u32 jump = 0;

	// Allow loadexec in whatever user level. Ignore K1 Check
	VWRITE16(text_addr + 0x16A6, 0x1000);
	VWRITE16(text_addr + 0x241E, 0x1000);
	VWRITE16(text_addr + 0x2622, 0x1000);

	for (int i = 0; i < text_size; i += 4) {
		u32 addr = text_addr + i;
		u32 data = VREAD32(addr);

		// Allow loadexec in whatever user level. Make sceKernelGetUserLevel return 4
		if (data == 0x1445FFF4) {
			MAKE_DUMMY_FUNCTION(K_EXTRACT_CALL(addr - 0x10), 4);
			continue;
		}

		// Remove apitype check in FW's above 2.60
		if (data == 0x24070200) {
			memset((void *)addr, 0, 0x20);
			continue;
		}

		// Patch to do things before reboot
		if (data == 0x02202021 && VREAD32(addr + 4) == 0x00401821) {
			K_HIJACK_CALL(addr - 4, RunRebootPatched, RunReboot);
			continue;
		}

		// Ignore kermit calls
		if (data == 0x17C001D3) {
			MAKE_NOP(addr);
			jump = addr + 8;
			continue;
		}

		// Redirect pointer to 0x88FC0000
		if (data == 0x04400020) {
			K_HIJACK_CALL(addr - 8, DecodeKL4EPatched, DecodeKL4E);
			VWRITE8(addr + 0x44, 0xFC);
			continue;
		}

		// Fix type check
		if (data == 0x34650002) {
			MAKE_INSTRUCTION(addr, 0x24050002); // ori $a1, $v1, 0x2 -> li $a1, 2
			MAKE_INSTRUCTION(addr + 4, 0x12E500B7); // bnez $s7, loc_XXXXXXXX -> beq $s7, $a1, loc_XXXXXXXX
			MAKE_INSTRUCTION(addr + 8, 0xAC570018); // sw $a1, 24($v0) -> sw $s7, 24($v0)
			continue;
		}

		if (data == 0x24100200) {
			// Some registers are reserved. Use other registers to avoid malfunction
			MAKE_INSTRUCTION(addr, 0x24050200); // li $s0, 0x200 -> li $a1, 0x200
			MAKE_INSTRUCTION(addr + 4, 0x12650003); // beq $s3, $s0, loc_XXXXXXXX - > beq $s3, $a1, loc_XXXXXXXX
			MAKE_INSTRUCTION(addr + 8, 0x241E0210); // li $s5, 0x210 -> li $fp, 0x210
			MAKE_INSTRUCTION(addr + 0xC, 0x567EFFDE); // bne $s3, $s5, loc_XXXXXXXX -> bne $s3, $fp, loc_XXXXXXXX

			// Allow LoadExecVSH type 1. Ignore peripheralCommon KERMIT_CMD_ERROR_EXIT
			MAKE_JUMP(addr + 0x14, jump);
			MAKE_INSTRUCTION(addr + 0x18, 0x24170001); // li $s7, 1

			continue;
		}
	}

	sctrlFlushCache();
}

void PatchChkreg() {
	MAKE_DUMMY_FUNCTION(K_EXTRACT_IMPORT(&sceChkregCheckRegion), 1);
	HIJACK_FUNCTION(K_EXTRACT_IMPORT(&sceChkregGetPsCode), sceChkregGetPsCodePatched, _sceChkregGetPsCode);
	sctrlFlushCache();
}

void PatchImposeDriver(SceModule* mod) {
	u32 text_addr = mod->text_addr;

	// Hide volume bar
	MAKE_NOP(text_addr + 0x4AEC);

	HIJACK_FUNCTION(text_addr + 0x381C, SetIdleCallbackPatched, SetIdleCallback);

	if (sceKernelApplicationType() == SCE_APPTYPE_POPS) {
		SetupCallbacks();
		MAKE_DUMMY_FUNCTION(text_addr + 0x91C8, SCE_APPTYPE_GAME);
	}

	REDIRECT_FUNCTION(text_addr + 0x92B0, sceKernelWaitEventFlagPatched);

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

void PatchMemlmd() {
	SceModule *mod = sceKernelFindModuleByName("sceMemlmd");
	u32 text_addr = mod->text_addr;

	// Allow 6.61 kernel modules
	MAKE_CALL(text_addr + 0x2C8, memcmp_patched);
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

	for (int i = 0; i < sizeof(nids) / sizeof(u32); i++) {
		u32 addr = FindFirstBEQ(FindProc("sceSystemMemoryManager", "SysMemUserForUser", nids[i]));
		if (addr) {
			VWRITE16(addr + 2, 0x1000);
		}
	}
}

void PatchUmdDriver(SceModule* mod) {
	u32 text_addr = mod->text_addr;

	REDIRECT_FUNCTION(text_addr + 0xC80, sceUmdRegisterUMDCallBackPatched);
	sctrlFlushCache();
}

void PatchMeCodecWrapper(SceModule* mod) {
	HIJACK_FUNCTION(FindProcInMod(mod, "sceMeAudio_driver", 0xC300D466), sceMeAudio_driver_C300D466_Patched, _sceMeAudio_driver_C300D466);
	sctrlFlushCache();
}

void PatchMesgLed(SceModule* mod) {
	REDIRECT_FUNCTION(FindProcInMod(mod, "sceResmgr_driver", 0x9DC14891), sceResmgrDecryptIndexPatched);
	sctrlFlushCache();
}