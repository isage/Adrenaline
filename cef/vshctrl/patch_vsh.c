/*
	Adrenaline
	Copyright (C) 2016-2018, TheFloW
	Copyright (C) 2024-2025, isage
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

#include <stdio.h>
#include <string.h>

#include <pspreg.h>
#include <pspsysmem.h>
#include <psploadexec.h>
#include <pspumd.h>

#include <cfwmacros.h>
#include <systemctrl.h>
#include <systemctrl_se.h>

#include <systemctrl_adrenaline.h>
#include <adrenaline_log.h>

#include "virtualpbpmgr.h"
#include "patch_io.h"
#include "externs.h"

#include "../../adrenaline_version.h"

////////////////////////////////////////////////////////////////////////////////
// HELPERS
////////////////////////////////////////////////////////////////////////////////

#define BOOT_BIN "disc0:/PSP_GAME/SYSDIR/BOOT.BIN"
#define EBOOT_BIN "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN"
#define EBOOT_OLD "disc0:/PSP_GAME/SYSDIR/EBOOT.OLD"

static int g_exec_boot_bin = 0;

// Credits: ARK CFW
static inline void ascii2utf16(char *dest, const char *src) {
    while(*src != '\0') {
        *dest++ = *src;
        *dest++ = '\0';
        src++;
    }
    *dest++ = '\0';
    *dest++ = '\0';
}

static void CheckControllerInput() {
	SceCtrlData pad_data;
	sceCtrlPeekBufferPositive(&pad_data, 1);
	if ((pad_data.Buttons & PSP_CTRL_RTRIGGER) == PSP_CTRL_RTRIGGER) {
		g_exec_boot_bin = 1;
		logmsg2("[INFO]: Set to exec BOOT.BIN (if exist) by holding `R` at the ISO application start\n");
	}
}

////////////////////////////////////////////////////////////////////////////////
// PATCHED IMPLEMENTATIONS
////////////////////////////////////////////////////////////////////////////////

int sceCtrlReadBufferPositivePatched(SceCtrlData *pad_data, int count) {
	static SceUID modid = -1;
	int res = sceCtrlReadBufferPositive(pad_data, count);
	int k1 = pspSdkSetK1(0);

	if (!g_set && g_cfw_config.vsh_cpu_speed != 0) {
		u32 curtick = sceKernelGetSystemTimeLow();
		curtick -= g_firsttick;

		u32 t = (u32)curtick;
		if (t >= (10 * 1000 * 1000)) {
			g_set = 1;
			sctrlHENSetSpeed(g_cpu_list[g_cfw_config.vsh_cpu_speed % N_CPU], g_bus_list[g_cfw_config.vsh_cpu_speed % N_CPU]);
		}
	}

	if (!sceKernelFindModuleByName("EPI-VshCtrlSatelite")) {
		if (pad_data->Buttons & PSP_CTRL_SELECT) {
			if (!sceKernelFindModuleByName("htmlviewer_plugin_module")
				&& !sceKernelFindModuleByName("sceVshOSK_Module")
				&& !sceKernelFindModuleByName("camera_plugin_module")) {
				modid = sceKernelLoadModule("flash0:/vsh/module/satelite.prx", 0, NULL);

				if (modid >= 0) {
					sceKernelDelayThread(4000);
					sceKernelStartModule(modid, 0, NULL, NULL, NULL);
					pad_data->Buttons &= ~PSP_CTRL_SELECT;
				}
			}
		}
	} else {
		if (g_vshmenu_ctrl) {
			g_vshmenu_ctrl(pad_data, count);
		} else if (modid >= 0) {
			if (sceKernelStopModule(modid, 0, NULL, NULL, NULL) >= 0) {
				sceKernelUnloadModule(modid);
			}
		}
	}

	pspSdkSetK1(k1);
	return res;
}

int InitUsbPatched() {
	return sctrlStartUsb();
}

int ShutdownUsbPatched() {
	return sctrlStopUsb();
}

int GetUsbStatusPatched() {
	int state = sctrlGetUsbState();

	if (state & 0x20)
		return 1; // Connected

	return 2; // Not connected
}

int SetDefaultNicknamePatched() {
    int k1 = pspSdkSetK1(0);

    struct RegParam reg;
    REGHANDLE h;
    memset(&reg, 0, sizeof(reg));
    reg.regtype = 1;
    reg.namelen = strlen("flash1:/registry/system");
    reg.unk2 = 1;
    reg.unk3 = 1;
    strcpy(reg.name, "flash1:/registry/system");

    if(sceRegOpenRegistry(&reg, 2, &h) == 0) {
        REGHANDLE hd;
        if(!sceRegOpenCategory(h, "/CONFIG/SYSTEM", 2, &hd)) {
            char* name = (char*)0xa83ff050;
            if(sceRegSetKeyValue(hd, "owner_name", name, strlen(name)) == 0) {
                printf("Set registry value\n");
                sceRegFlushCategory(hd);
            }
            sceRegCloseCategory(hd);
        }
        sceRegFlushRegistry(h);
        sceRegCloseRegistry(h);
    }

    pspSdkSetK1(k1);
    return 0;
}

int sceUpdateDownloadSetVersionPatched(int version) {
	int k1 = pspSdkSetK1(0);

	int (* sceUpdateDownloadSetVersion)(int version) = (void *)sctrlHENFindFunction("SceUpdateDL_Library", "sceLibUpdateDL", 0xC1AF1076);
	int (* sceUpdateDownloadSetUrl)(const char *url) = (void *)sctrlHENFindFunction("SceUpdateDL_Library", "sceLibUpdateDL", 0xF7E66CB4);

	sceUpdateDownloadSetUrl("http://adrenaline.sarcasticat.com/psp-updatelist.txt");
	int res = sceUpdateDownloadSetVersion(sctrlSEGetVersion());

	pspSdkSetK1(k1);
	return res;
}

int LoadExecVSHCommonPatched(int apitype, char *file, SceKernelLoadExecVSHParam *param, int unk2) {
	int k1 = pspSdkSetK1(0);

	VshCtrlSetUmdFile("");

	int index = GetIsoIndex(file);
	if (index >= 0) {
		int has_pboot = 0;
		CheckControllerInput();
		VshCtrlSetUmdFile(virtualpbp_getfilename(index));

		// Set UMD type on ISO
		sctrlSESetDiscType(PSP_UMD_TYPE_GAME);

		u32 opn_type = virtualpbp_get_isotype(index);
		SceGameInfo *info = sceKernelGetGameInfo();
		if (opn_type) {
			info->opnssmp_ver = opn_type;
		}

		int uses_prometheus = 0;
		int has_bootbin = 0;

		// Execute patched ISOs
		if (isofs_init() < 0) {
			isofs_exit();
			return -1;
		}

		SceIoStat stat;
		if (isofs_getstat("/PSP_GAME/SYSDIR/EBOOT.OLD", &stat) >= 0) {
			uses_prometheus = 1;
		}

		if (isofs_getstat("/PSP_GAME/SYSDIR/BOOT.BIN", &stat) >= 0) {
			has_bootbin = 1;
		}

		isofs_exit();


        if(strstr( param->argp , "PBOOT.PBP") != NULL) {
            has_pboot = 1;
        }

        if (!has_pboot) {
			if (uses_prometheus) {
				param->argp = EBOOT_OLD;
			} else {
				if ((g_cfw_config.execute_boot_bin || g_exec_boot_bin) && has_bootbin) {
					param->argp = BOOT_BIN;
				} else {
					param->argp = EBOOT_BIN;
				}
			}
		}

		// Update path and key
		file = param->argp;
		param->key = "umdemu";

		// Set umd_mode
		if (g_cfw_config.umd_mode == ISO_MODE_INFERNO) {
			logmsg2("[INFO]: Launching with Inferno Driver\n");
			sctrlSESetBootConfFileIndex(MODE_INFERNO);
		} else if (g_cfw_config.umd_mode == ISO_MODE_MARCH33) {
			logmsg2("[INFO]: Launching with March33 Driver\n");
			sctrlSESetBootConfFileIndex(MODE_MARCH33);
		} else if (g_cfw_config.umd_mode == ISO_MODE_NP9660) {
			logmsg2("[INFO]: Launching with NP9660 Driver\n");
			sctrlSESetBootConfFileIndex(MODE_NP9660);
		}

		if (has_pboot) {
			apitype = PSP_INIT_APITYPE_UMD_EMU_MS2;
		} else {
			apitype = PSP_INIT_APITYPE_UMD_EMU_MS1;
		}

		param->args = strlen(param->argp) + 1; //Update length

		pspSdkSetK1(k1);

		return sctrlKernelLoadExecVSHWithApitype(apitype, file, param);
	}

	// Enable 1.50 homebrews boot
	char *perc = strchr(param->argp, '%');
	if (perc) {
		strcpy(perc, perc + 1);
		file = param->argp;
		param->args = strlen(param->argp) + 1; //Update length
	}

	pspSdkSetK1(k1);

	return sctrlKernelLoadExecVSHWithApitype(apitype, file, param);
}

////////////////////////////////////////////////////////////////////////////////
// MODULE PATCHERS
////////////////////////////////////////////////////////////////////////////////

void PatchVshMain(SceModule* mod) {
	u32 text_addr = mod->text_addr;

	// Allow old sfo's
	MAKE_NOP(text_addr + 0x122B0);
	MAKE_NOP(text_addr + 0x12058); //DISC_ID
	MAKE_NOP(text_addr + 0x12060); //DISC_ID

	IoPatches();

	SceModule *vsh_bridge_mod = sceKernelFindModuleByName("sceVshBridge_Driver");
	sctrlHookImportByNID(vsh_bridge_mod, "sceCtrl_driver", 0xBE30CED0, sceCtrlReadBufferPositivePatched);
	sctrlHENPatchSyscall((void*)K_EXTRACT_IMPORT(&sceCtrlReadBufferPositive), sceCtrlReadBufferPositivePatched);

	// Dummy usb detection functions
	// Those break camera, but doesn't seem to affect usb connection
	//	MAKE_DUMMY_FUNCTION(text_addr + 0x38C94, 0);
	//	MAKE_DUMMY_FUNCTION(text_addr + 0x38D68, 0);

	if (g_cfw_config.skip_game_boot_logo) {
		// Disable sceDisplaySetHoldMode
		MAKE_NOP(text_addr + 0xCA88);
	}

	if (g_cfw_config.extended_colors == 1) {
		VWRITE16(text_addr + 0x3174A, 0x1000);
	}

	sctrlFlushCache();
}

void PatchSysconfPlugin(SceModule* mod) {
	static wchar_t macinfo[] = L"00:00:00:00:00:00";

	u32 text_addr = mod->text_addr;

	char verinfo[50] = {0};
	#ifdef NIGHTLY
	sprintf(verinfo, "6.61 Epinephrine-%d.%d.%d-%s", ADRENALINE_VERSION_MAJOR, ADRENALINE_VERSION_MINOR, ADRENALINE_VERSION_MICRO, EPI_NIGHTLY_VER );
	#else
	sprintf(verinfo, "6.61 Epinephrine-%d.%d.%d", ADRENALINE_VERSION_MAJOR, ADRENALINE_VERSION_MINOR, ADRENALINE_VERSION_MICRO );
	#endif

	ascii2utf16( (char*)((void *)text_addr + 0x2A62C), verinfo);

	MAKE_INSTRUCTION(text_addr + 0x192E0, 0x3C020000 | ((u32)(text_addr + 0x2A62C) >> 16));
	MAKE_INSTRUCTION(text_addr + 0x192E4, 0x34420000 | ((u32)(text_addr + 0x2A62C) & 0xFFFF));

	if (g_cfw_config.hide_mac_addr) {
		memcpy((void *)text_addr + 0x2E9A0, macinfo, sizeof(macinfo));
	}

	// Allow slim colors
	if (g_cfw_config.extended_colors != 0) {
		MAKE_INSTRUCTION(text_addr + 0x76EC, VREAD32(text_addr + 0x76F0));
		MAKE_INSTRUCTION(text_addr + 0x76F0, LI_V0(1));
	}

	// Dummy all vshbridge usbstor functions
	MAKE_INSTRUCTION(text_addr + 0xCD78, LI_V0(1));   // sceVshBridge_ED978848 - vshUsbstorMsSetWorkBuf
	MAKE_INSTRUCTION(text_addr + 0xCDAC, MOVE_V0_ZR); // sceVshBridge_EE59B2B7
	MAKE_INSTRUCTION(text_addr + 0xCF0C, MOVE_V0_ZR); // sceVshBridge_6032E5EE - vshUsbstorMsSetProductInfo
	MAKE_INSTRUCTION(text_addr + 0xD218, MOVE_V0_ZR); // sceVshBridge_360752BF - vshUsbstorMsSetVSHInfo

	// Dummy LoadUsbModules, UnloadUsbModules
	MAKE_DUMMY_FUNCTION(text_addr + 0xCC70, 0);
	MAKE_DUMMY_FUNCTION(text_addr + 0xD2C4, 0);

	// Redirect USB functions
	REDIRECT_FUNCTION(text_addr + 0xAE9C, sctrlHENMakeSyscallStub(InitUsbPatched));
	REDIRECT_FUNCTION(text_addr + 0xAFF4, sctrlHENMakeSyscallStub(ShutdownUsbPatched));
	REDIRECT_FUNCTION(text_addr + 0xB4A0, sctrlHENMakeSyscallStub(GetUsbStatusPatched));

	// Ignore wait thread end failure
	MAKE_NOP(text_addr + 0xB264);

	// Do not set nickname to PXXX on initial setup/reset
	REDIRECT_FUNCTION(text_addr + 0x1520, sctrlHENMakeSyscallStub(SetDefaultNicknamePatched));

	sctrlFlushCache();
}

void PatchGamePlugin(SceModule* mod) {
	u32 text_addr = mod->text_addr;

	// Allow homebrew launch
	MAKE_DUMMY_FUNCTION(text_addr + 0x20528, 0);

	// Allow PSX launch
	MAKE_DUMMY_FUNCTION(text_addr + 0x20E6C, 0);

	// Allow custom multi-disc PSX
	MAKE_NOP(text_addr + 0x14850);

	// if check patch
	MAKE_INSTRUCTION(text_addr + 0x20620, MOVE_V0_ZR);

	if (g_cfw_config.hide_pic0pic1 != PICS_OPT_DISABLED) {
		if (g_cfw_config.hide_pic0pic1 == PICS_OPT_BOTH || g_cfw_config.hide_pic0pic1 == PICS_OPT_PIC0_ONLY) {
			MAKE_INSTRUCTION(text_addr + 0x1D858, 0x00601021);
		}

		if (g_cfw_config.hide_pic0pic1 == PICS_OPT_BOTH || g_cfw_config.hide_pic0pic1 == PICS_OPT_PIC1_ONLY) {
			MAKE_INSTRUCTION(text_addr + 0x1D864, 0x00601021);
		}
	}

	if (g_cfw_config.skip_game_boot_logo) {
		MAKE_CALL(text_addr + 0x19130, text_addr + 0x194B0);
		MAKE_INSTRUCTION(text_addr + 0x19134, 0x24040002);
	}

	sctrlFlushCache();
}

void PatchUpdatePlugin(SceModule* mod) {
	u32 text_addr = mod->text_addr;

	MAKE_CALL(text_addr + 0x82A8, sctrlHENMakeSyscallStub(sceUpdateDownloadSetVersionPatched));
	sctrlFlushCache();
}

void PatchLoadExec() {
	SceModule *mod = sceKernelFindModuleByName("sceLoadExec");
	u32 text_addr = mod->text_addr;

	MAKE_CALL(text_addr + 0x1DC0, LoadExecVSHCommonPatched); //sceKernelLoadExecVSHMs2
	MAKE_CALL(text_addr + 0x1BE0, LoadExecVSHCommonPatched); //sceKernelLoadExecVSHMsPboot
	MAKE_CALL(text_addr + 0x1BB8, LoadExecVSHCommonPatched); //sceKernelLoadExecVSHUMDEMUPboot

	sctrlFlushCache();
}