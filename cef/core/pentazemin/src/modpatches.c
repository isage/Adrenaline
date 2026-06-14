#include <string.h>

#include <pspsdk.h>
#include <pspumd.h>
#include <pspinit.h>
#include <psperror.h>
#include <pspsysmem_kernel.h>
#include <psputilsforkernel.h>

#include <cfwmacros.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <systemctrl_adrenaline.h>

#include <adrenaline_log.h>

////////////////////////////////////////////////////////////////////////////////
// HELPERS
////////////////////////////////////////////////////////////////////////////////

static int ReadFile(char *file, void *buf, int size) {
	SceUID fd = sceIoOpen(file, PSP_O_RDONLY, 0);
	if (fd < 0)
		return fd;

	int read = sceIoRead(fd, buf, size);

	sceIoClose(fd);
	return read;
}


////////////////////////////////////////////////////////////////////////////////
// PATCHED IMPLEMENTATIONS
////////////////////////////////////////////////////////////////////////////////

int sceResmgrDecryptIndexPatched(void *buf, int size, int *retSize) {
	int k1 = pspSdkSetK1(0);
	*retSize = ReadFile("flash0:/vsh/etc/version.txt", buf, size);
	pspSdkSetK1(k1);
	return 0;
}

int sceUmdRegisterUMDCallBackPatched(int cbid) {
	int k1 = pspSdkSetK1(0);
	int res = sceKernelNotifyCallback(cbid, PSP_UMD_NOT_PRESENT);
	pspSdkSetK1(k1);
	return res;
}

int sceKernelSuspendThreadPatched(SceUID thid) {
	SceKernelThreadInfo info;
	info.size = sizeof(SceKernelThreadInfo);
	if (sceKernelReferThreadStatus(thid, &info) == 0) {
		if (strcmp(info.name, "popsmain") == 0) {
			sctrlSendAdrenalineCmd(ADRENALINE_VITA_CMD_PAUSE_POPS, 0);
		}
	}

	return sceKernelSuspendThread(thid);
}

int sceKernelResumeThreadPatched(SceUID thid) {
	SceKernelThreadInfo info;
	info.size = sizeof(SceKernelThreadInfo);
	if (sceKernelReferThreadStatus(thid, &info) == 0) {
		if (strcmp(info.name, "popsmain") == 0) {
			sctrlSendAdrenalineCmd(ADRENALINE_VITA_CMD_RESUME_POPS, 0);
		}
	}

	return sceKernelResumeThread(thid);
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

static int (*_sceKernelPowerTick)(u32 tick_type) = NULL;
int sceKernelPowerTickPatched(u32 tick_type) {
	if (_sceKernelPowerTick == NULL) {
		return SCE_KERR_ILLEGAL_ADDR;
	}

	int res = sctrlSendAdrenalineCmd(ADRENALINE_VITA_CMD_POWER_TICK, tick_type);

	if (res < 0) {
		goto exit;
	}

	u32 k1 = pspSdkSetK1(0);
	res = _sceKernelPowerTick(tick_type);
	pspSdkSetK1(k1);

exit:
	logmsg4("[DEBUG]: %s: tick_type=%d -> 0x%08X\n", __func__, tick_type, res);
	return res;
}

static int (* _sceMeAudio_driver_C300D466)(int codec, int unk, void *info) = NULL;
int sceMeAudio_driver_C300D466_Patched(int codec, int unk, void *info) {
	int res = _sceMeAudio_driver_C300D466(codec, unk, info);

	if (res < 0 && codec == 0x1002 && unk == 2) {
		return 0;
	}

	return res;
}

static int (* _sceKernelVolatileMemTryLock)(int unk, void **ptr, int *size) = NULL;
int sceKernelVolatileMemTryLockPatched(int unk, void **ptr, int *size) {
	int res = 0;

	for (int i = 0; i < 0x10; i++) {
		res = _sceKernelVolatileMemTryLock(unk, ptr, size);
		if (res >= 0) {
			break;
		}

		sceKernelDelayThread(100);
	}

	return res;
}


////////////////////////////////////////////////////////////////////////////////
// MODULE PATCHERS
////////////////////////////////////////////////////////////////////////////////

void PatchMemlmd(void) {
	SceModule *mod = (SceModule*)sceKernelFindModuleByName("sceMemlmd");
	u32 text_addr = mod->text_addr;

	// Allow 6.61 kernel modules
	MAKE_CALL(text_addr + 0x2C8, memcmp_patched);

	sctrlFlushCache();
}

void PatchSysmem(void) {
	SceModule *mod = sceKernelFindModuleByName("sceSystemMemoryManager");
	u32 power_tick_addr = sctrlHENFindFunctionInMod(mod, "sceSuspendForKernel", 0x090CCB3F);
	HIJACK_FUNCTION(power_tick_addr, sceKernelPowerTickPatched, _sceKernelPowerTick);
}

void PatchVolatileMemBug() {
	if (sceKernelBootFrom() == PSP_BOOT_DISC) {
		_sceKernelVolatileMemTryLock = (void *)sctrlHENFindFunction("sceSystemMemoryManager", "sceSuspendForUser", 0xA14F40B2);
		sctrlHENPatchSyscall(_sceKernelVolatileMemTryLock, sceKernelVolatileMemTryLockPatched);
		sctrlFlushCache();
	}
}

void PatchChkreg(SceModule* mod) {
	MAKE_DUMMY_FUNCTION(sctrlHENFindFunctionInMod(mod, "sceChkreg_driver", 0x54495B19), 1);
	sctrlFlushCache();
}

void PatchMesgLed(SceModule* mod) {
	REDIRECT_FUNCTION(sctrlHENFindFunctionInMod(mod, "sceResmgr_driver", 0x9DC14891), sceResmgrDecryptIndexPatched);
	sctrlFlushCache();
}

void PatchUmdDriver(SceModule* mod) {
	u32 text_addr = mod->text_addr;

	REDIRECT_FUNCTION(text_addr + 0xC80, sceUmdRegisterUMDCallBackPatched);
	sctrlFlushCache();
}

void PatchLoadExec(SceModule* mod) {
	u32 text_addr = mod->text_addr;
	u32 text_size = mod->text_size;

	u32 jump = 0;

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

		// Ignore kermit calls
		if (data == 0x17C001D3) {
			MAKE_NOP(addr);
			jump = addr + 8;
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

void PatchPopsMgr(SceModule* mod) {
	u32 text_addr = mod->text_addr;

	// Use different mode for SceKermitPocs
	VWRITE32(text_addr + 0x2030, 0x2405000E);
	VWRITE32(text_addr + 0x20F0, 0x2405000E);
	VWRITE32(text_addr + 0x21A0, 0x2405000E);

	// Use different pops register location
	VWRITE32(text_addr + 0x11B4, 0x3C014BCD);

	sctrlFlushCache();
}

void PatchPops(SceModule* mod) {
	// Use different pops register location
	for (u32 i = 0; i < mod->text_size; i += 4) {
		if ((VREAD32(mod->text_addr+i) & 0xFFE0FFFF) == 0x3C0049FE) {
			VWRITE16(mod->text_addr+i, 0x4BCD);
		}
	}

	sctrlFlushCache();
}

void PatchMeCodecWrapper(SceModule* mod) {
	HIJACK_FUNCTION(sctrlHENFindFunctionInMod(mod, "sceMeAudio_driver", 0xC300D466), sceMeAudio_driver_C300D466_Patched, _sceMeAudio_driver_C300D466);
	sctrlFlushCache();
}

void PatchSysconfPlugin(SceModule* mod) {
	u32 text_addr = mod->text_addr;

	// Dummy all vshbridge usbstor functions
	MAKE_INSTRUCTION(text_addr + 0xCD78, LI_V0(1));   // sceVshBridge_ED978848 - vshUsbstorMsSetWorkBuf
	MAKE_INSTRUCTION(text_addr + 0xCDAC, MOVE_V0_ZR); // sceVshBridge_EE59B2B7
	MAKE_INSTRUCTION(text_addr + 0xCF0C, MOVE_V0_ZR); // sceVshBridge_6032E5EE - vshUsbstorMsSetProductInfo
	MAKE_INSTRUCTION(text_addr + 0xD218, MOVE_V0_ZR); // sceVshBridge_360752BF - vshUsbstorMsSetVSHInfo

	// Dummy LoadUsbModules, UnloadUsbModules
	MAKE_DUMMY_FUNCTION(text_addr + 0xCC70, 0);
	MAKE_DUMMY_FUNCTION(text_addr + 0xD2C4, 0);

	// Redirect USB functions
	REDIRECT_SYSCALL(text_addr + 0xAE9C, sctrlStartUsb);
    REDIRECT_SYSCALL(text_addr + 0xAFF4, sctrlStopUsb);
    REDIRECT_SYSCALL(text_addr + 0xB4A0, sctrlGetUsbState);

	// Ignore wait thread end failure
	MAKE_NOP(text_addr + 0xB264);

	sctrlFlushCache();
}

void PatchCwCheatPlugin(SceModule* mod) {
	if (sceKernelApplicationType() == PSP_INIT_KEYCONFIG_POPS) {
		sctrlHookImportByNID(mod, "ThreadManForKernel", 0x9944F31F, sceKernelSuspendThreadPatched);
		sctrlHookImportByNID(mod, "ThreadManForKernel", 0x75156E8F, sceKernelResumeThreadPatched);
		sctrlFlushCache();
	}
}

// Fix VLF Module
void PatchVlfLib(SceModule* mod) {
	static u32 nids[] = { 0x2A245FE6, 0x7B08EAAB, 0x22050FC0, 0x158BE61A, 0xD495179F };

	for (int i = 0; i < (sizeof(nids) / sizeof(u32)); i++) {
		sctrlHookImportByNID(mod, "VlfGui", nids[i], NULL);
	}

	sctrlFlushCache();
}

void PatchGameBoot(SceModule* mod) {
	u32 p1 = 0;
	u32 p2 = 0;

	int patches = 2;
	for (u32 addr=mod->text_addr; addr < mod->text_addr+mod->text_size && patches; addr += 4) {
		u32 data = VREAD32(addr);

		if (data == 0x2C43000D){
			p1 = addr-36;
			patches--;

		} else if (data == 0x27BDFF20 && VREAD32(addr-4) == 0x27BD0040){
			p2 = addr-24;
			patches--;
		}
	}

	MAKE_INSTRUCTION(p2, JAL(p1));
	MAKE_INSTRUCTION(p2 + 4, 0x24040002);

	sctrlFlushCache();
}