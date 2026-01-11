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

#include <common.h>
#include <pspusbcam.h>
#include <pspdisplay.h>

#include <infernoctrl.h>

#define _ADRENALINE_LOG_IMPL_
#include <adrenaline_log.h>

#include "main.h"
#include "adrenaline.h"
#include "modulepatches.h"
#include "nodrm.h"
#include "malloc.h"
#include "ttystdio.h"
#include "gameinfo.h"
#include "plugin.h"
#include "utils.h"
#include "storage_cache.h"
#include "../../adrenaline_compat.h"

PSP_MODULE_INFO("SystemControl", 0x1007, 1, 1);

RebootexConfig rebootex_config;
SEConfigADR config;

int idle = 0;

int cpu_list[] = { 0, 20, 75, 100, 133, 222, 266, 300, 333 };
int bus_list[] = { 0, 10, 37,  50,  66, 111, 133, 150, 166 };
u32 cache_size_list[] = { 0, 1024, 2*1024, 4*1024, 8*1024, 16*1024, 32*1024, 64*1024 };
u8 cache_num_list[] = { 0, 1, 2, 4, 8, 16, 32, 64, 128 };

#define N_CPU (sizeof(cpu_list) / sizeof(int))
#define N_CACHE_SIZE (sizeof(cache_size_list) / sizeof(u32))
#define N_CACHE_NUM (sizeof(cache_num_list) / sizeof(u8))

static void OnSystemStatusIdle() {
	SceAdrenaline *adrenaline = (SceAdrenaline *)ADRENALINE_ADDRESS;

	initAdrenalineInfo();
	PatchVolatileMemBug();

	if (config.no_ms_cache == 0) {
		storageCacheInit("ms");
	}

	// March33 UMD seek/speed simulation
	SceModule* march33_mod = sceKernelFindModuleByName("EPI-March33Driver");

	if (march33_mod != NULL) {
		// Handle March33's UMD seek and UMD speed setting
		if (config.umd_seek > 0 || config.umd_speed > 0) {
			config.iso_cache = CACHE_CONFIG_OFF;

			void (*SetUmdDelay)(int, int) = (void*)sctrlHENFindFunctionInMod(march33_mod, "march33_driver", 0xFAEC97D6);

			if (SetUmdDelay != NULL) {
				SetUmdDelay(config.umd_seek, config.umd_speed);
				logmsg3("[INFO]: March33 ISO UMD seek/speed factor: %d seek factor; %d speed factor\n", config.umd_seek, config.umd_speed);
			}
		}
	}

	// NP9660 UMD seek/speed simulation
	SceModule* galaxy_mod = sceKernelFindModuleByName("EPI-GalaxyController");

	if (galaxy_mod != NULL) {
		// Handle March33's UMD seek and UMD speed setting
		if (config.umd_seek > 0 || config.umd_speed > 0) {
			config.iso_cache = CACHE_CONFIG_OFF;

			void (*SetUmdDelay)(int, int) = (void*)sctrlHENFindFunctionInMod(galaxy_mod, "galaxy_driver", 0xFAEC97D6);

			if (SetUmdDelay != NULL) {
				SetUmdDelay(config.umd_seek, config.umd_speed);
				logmsg3("[INFO]: NP9660 ISO UMD seek/speed factor: %d seek factor; %d speed factor\n", config.umd_seek, config.umd_speed);
			}
		}
	}

	// Inferno cache config and UMD seek/speed simulation
	SceModule* inferno_mod = sceKernelFindModuleByName("EPI-InfernoDriver");

	// Inferno driver is loaded
	if (inferno_mod != NULL) {
		// Handle Inferno's UMD seek and UMD speed setting
		if (config.umd_seek > 0 || config.umd_speed > 0) {
			config.iso_cache = CACHE_CONFIG_OFF;

			void (*SetUmdDelay)(int, int) = (void*)sctrlHENFindFunctionInMod(inferno_mod, "inferno_driver", 0xB6522E93);

			if (SetUmdDelay != NULL) {
				SetUmdDelay(config.umd_seek, config.umd_speed);
				logmsg3("[INFO]: Inferno ISO UMD seek/speed factor: %d seek factor; %d speed factor\n", config.umd_seek, config.umd_speed);
			}
		}

		// Handle Inferno Iso cache
		if (config.iso_cache != CACHE_CONFIG_OFF) {
			if (rebootex_config.ram2 > 24 || config.force_high_memory != HIGHMEM_OPT_OFF) {
				config.iso_cache_partition = 2;
			} else {
				config.iso_cache_partition = 11;
			}

			int (*CacheInit)(int, int, int) = (void*)sctrlHENFindFunctionInMod(inferno_mod, "inferno_driver", 0x8CDE7F95);
			if (CacheInit != NULL) {
				u32 cache_size = (config.iso_cache_size == ISO_CACHE_SIZE_AUTO) ? 32*1024 : cache_size_list[config.iso_cache_size%N_CACHE_SIZE];
				u8 cache_num = (config.iso_cache_num == ISO_CACHE_NUM_AUTO) ? 32 : cache_num_list[config.iso_cache_num%N_CACHE_NUM];
				CacheInit(cache_size, cache_num, config.iso_cache_partition);
				logmsg3("[INFO]: Inferno ISO cache: %d caches of %ld KiB in partition %d — Total: %ld KiB\n", cache_num, cache_size/1024, config.iso_cache_partition, (cache_num*cache_size)/1024);
			}

			int (*CacheSetPolicy)(int) = (void*)sctrlHENFindFunctionInMod(inferno_mod, "inferno_driver", 0xC0736FD6);
			if (CacheSetPolicy != NULL) {
				if (config.iso_cache == CACHE_CONFIG_LRU) {
					CacheSetPolicy(INFERNO_CACHE_LRU);
					logmsg3("[INFO]: Inferno ISO cache policy: LRU\n");
				} else if (config.iso_cache == CACHE_CONFIG_RR) {
					CacheSetPolicy(INFERNO_CACHE_RR);
					logmsg3("[INFO]: Inferno ISO cache policy: RR\n");
				}
			}
		}
	}

	// Set CPU/BUS speed on apps/games
	PSPBootFrom medium_type = sceKernelBootFrom();
	PSPKeyConfig app_type = sceKernelApplicationType();
	u8 is_correct_medium = (medium_type == PSP_BOOT_DISC || medium_type == PSP_BOOT_MS || medium_type == PSP_BOOT_EF);

	if (app_type != PSP_INIT_KEYCONFIG_VSH && is_correct_medium) {
		sctrlHENSetSpeed(cpu_list[config.app_cpu_speed % N_CPU], bus_list[config.app_cpu_speed % N_CPU]);
	}

	// Set fake framebuffer so that cwcheat can be displayed
	if (adrenaline->pops_mode) {
		sceDisplaySetFrameBuf((void *)NATIVE_FRAMEBUFFER, PSP_SCREEN_LINE, PSP_DISPLAY_PIXEL_FORMAT_8888, PSP_DISPLAY_SETBUF_NEXTFRAME);
		memset((void *)NATIVE_FRAMEBUFFER, 0, SCE_PSPEMU_FRAMEBUFFER_SIZE);
	} else {
		SendAdrenalineCmd(ADRENALINE_VITA_CMD_RESUME_POPS, 0);
	}
	SendAdrenalineCmd(ADRENALINE_VITA_CMD_APP_STARTED, 0);
}

static int OnModuleStart(SceModule *mod) {
	static u8 ready_gamepatch_mod = 0;
	static u8 load_file_config = 0;

	PatchGameInfoGetter(mod);

	char *modname = mod->modname;
	u32 text_addr = mod->text_addr;

	// Game/App module patches
	if (ready_gamepatch_mod) {
		if (sceKernelApplicationType() != PSP_INIT_KEYCONFIG_VSH) {
			logmsg3("[INFO]: Title Module Name: %s\n", modname);
			PatchDrmGameModule(mod);
			PatchHideCfwFolders(mod);
			PatchGamesByMod(mod);
		}
		ready_gamepatch_mod = 0;
	}

	// Third-party Plugins modules
	if (isLoadingPlugins()) {
		// Fix 6.60 plugins on 6.61
		sctrlHookImportByNID(mod, "SysMemForKernel", 0x3FC9AE6A, &sctrlHENFakeDevkitVersion);
        sctrlHookImportByNID(mod, "SysMemUserForUser", 0x3FC9AE6A, &sctrlHENFakeDevkitVersion);
	}

	if (load_file_config) {
		SEConfig* res = sctrlSEGetConfig((SEConfig*)&config);
		logmsg("[DEBUG]: sctrlSEGetConfig -> 0x%08X\n", res);
		if (res == &config) {
			load_file_config = 0;
		}
	}

	if (strcmp(modname, "sceLowIO_Driver") == 0) {
		extern int mallocinit();
		if (mallocinit() < 0) {
			while (1) {
				VWRITE32(0, 0);
			}
		}

		// Protect pops memory
		if (sceKernelApplicationType() == PSP_INIT_KEYCONFIG_POPS) {
			sceKernelAllocPartitionMemory(6, "", PSP_SMEM_Addr, 0x80000, (void *)0x09F40000);
		}

		memset((void *)0x49F40000, 0, 0x80000);
		memset((void *)0xABCD0000, 0, 0x1B0);

		PatchLowIODriver2(mod);

	} else if (strcmp(mod->modname, "sceKermitMsfs_driver") == 0) {
		load_file_config = 1;

	} else if (strcmp(mod->modname, "sceController_Service") == 0) {
		PatchController(mod);

	} else if (strcmp(modname, "sceLoadExec") == 0) {
		PatchLoadExec(mod);

		#if defined(DEBUG) && DEBUG >= 3
		extern int (* _runExec)(RunExecParams*);
		extern int runExecPatched(RunExecParams* args);
		HIJACK_FUNCTION(text_addr+0x2148, runExecPatched, _runExec);
		#endif // defined(DEBUG) && DEBUG >= 3

		// Hijack all execute calls
		extern int (* _sceLoadExecVSHWithApitype)(int, const char*, struct SceKernelLoadExecVSHParam*, unsigned int);
		u32 _LoadExecVSHWithApitype = findFirstJAL(sctrlHENFindFunctionInMod(mod, "LoadExecForKernel", 0xD8320A28));
		HIJACK_FUNCTION(_LoadExecVSHWithApitype, sctrlKernelLoadExecVSHWithApitype, _sceLoadExecVSHWithApitype);

		// Hijack exit calls
		extern int (*_sceKernelExitVSH)(void*);
		u32 _KernelExitVSH = sctrlHENFindFunctionInMod(mod, "LoadExecForKernel", 0x08F7166C);
		HIJACK_FUNCTION(_KernelExitVSH, sctrlKernelExitVSH, _sceKernelExitVSH);

	} else if (strcmp(modname, "scePower_Service") == 0) {
		logmsg3("[INFO]: Built: %s %s\n", __DATE__, __TIME__);
		logmsg3("[INFO]: Boot From: 0x%X\n", sceKernelBootFrom());
		logmsg3("[INFO]: App Type: 0x%X\n", sceKernelApplicationType());
		logmsg3("[INFO]: Apitype: 0x%X\n", sceKernelInitApitype());
		logmsg3("[INFO]: Filename: %s\n", sceKernelInitFileName());

		if (sceKernelApplicationType() == PSP_INIT_KEYCONFIG_GAME  && config.force_high_memory != HIGHMEM_OPT_OFF) {
			if (config.force_high_memory == HIGHMEM_OPT_STABLE) {
				sctrlHENSetMemory(40, 9);
			} else if (config.force_high_memory == HIGHMEM_OPT_MAX) {
				sctrlHENSetMemory(52, 0);
			}
			ApplyMemory();
		} else {
			// If not force-high-memory. Make sure to make p11 as big as
			// possible (stable), but in a state that if a game request
			// high-memory, re-setting and re-applying is possible.
			sctrlHENSetMemory(24, 16);
			ApplyAndResetMemory();
		}

		PatchPowerService(mod);
		PatchPowerService2(mod);

	} else if (strcmp(modname, "sceChkreg") == 0) {
		PatchChkreg();

	} else if (strcmp(modname, "sceMesgLed") == 0) {
		PatchMesgLed(mod);

	} else if (strcmp(modname, "scePspNpDrm_Driver") == 0) {
		PatchNpDrmDriver(mod);

	} else if (strcmp(modname, "sceNp9660_driver") == 0) {
		PatchNp9660Driver(mod);

	} else if (strcmp(modname, "sceUmd_driver") == 0) {
		PatchUmdDriver(mod);

	} else if(strcmp(modname, "sceMeCodecWrapper") == 0) {
		PatchMeCodecWrapper(mod);

	} else if (strcmp(modname, "sceUtility_Driver") == 0) {
		PatchUtility();
		findAndSetTitleId();
		logmsg3("[INFO]: Title ID: %s\n", rebootex_config.title_id);
		CheckControllerInput();

	} else if (strcmp(modname, "sceImpose_Driver") == 0) {
		PatchImposeDriver(mod);

	} else if (strcmp(modname, "sceMediaSync") == 0) {
		PatchMediaSync(mod);

	} else if (strcmp(modname, "sceNpSignupPlugin_Module") == 0) {
		// ImageVersion = 0x10000000
		MAKE_INSTRUCTION(text_addr + 0x38CBC, 0x3C041000);
		sctrlFlushCache();

	} else if (strcmp(modname, "sceVshNpSignin_Module") == 0) {
		// Kill connection error
		MAKE_INSTRUCTION(text_addr + 0x6CF4, 0x10000008);

		// ImageVersion = 0x10000000
		MAKE_INSTRUCTION(text_addr + 0x96C4, 0x3C041000);

		sctrlFlushCache();

	} else if (strcmp(modname, "sceSAScore") == 0) {
		PatchSasCore(mod);

	} else if (strcmp(modname, "sceUSBCam_Driver") == 0) {
		PatchUSBCamDriver(mod);

	} else if (strcmp(modname, "sceKernelLibrary") == 0) { // last kernel module to load before user/game
		ready_gamepatch_mod = 1;
		PatchGameByTitleId();

	} else if (strcmp(modname, "sceMp3_Library") == 0 || (strcmp(modname, "sceVshOSK_Module") == 0 && config.use_sony_psposk)) {
		// Unlock mp3 variable bitrate and qwerty osk on old games/homebrew
		sctrlHookImportByNID(mod, "SysMemUserForUser", 0xFC114573, &sctrlHENFakeDevkitVersion);
		sctrlFlushCache();

	} else if (strcmp(modname, "vsh_module") == 0) {
		PatchVshForDrm(mod);
		PatchDrmOnVsh();

	} else if (strcmp(modname, "sysconf_plugin_module") == 0) {
		PatchSysconfForDrm(mod);

	} else if (strcmp(modname, "VLF_Module") == 0) {
		PatchVlfLib(mod);

	} else if (strcmp(mod->modname, "CWCHEATPRX") == 0) {
		PatchCwCheatPlugin(mod);
	}

	if (!idle) {
		if (sctrlHENIsSystemBooted()) {
			idle = 1;
			OnSystemStatusIdle();
		}
	}

	logmsg4("[DEBUG]: %s: mod_name=%s — text_addr=0x%08X\n", __func__, modname, text_addr);

	return 0;
}

int module_start(SceSize args, void *argp) {
	logInit("ms0:/log_systemctrl.txt");
	logmsg("SystemControl started...\n")

	PatchSysmem();
	PatchLoadCore();
	PatchInterruptMgr();
	PatchIoFileMgr();
	PatchMemlmd();
	PatchModuleMgr();
	sctrlFlushCache();

	// Restore CFW config from the RAM backup in the start (if it exists)
	if (IS_ADR_SECONFIG(EPI_CONFIG_ADDR)) {
		memcpy(&config, (void *)EPI_CONFIG_ADDR, sizeof(SEConfigADR));
	}

	sctrlHENSetStartModuleHandler((STMOD_HANDLER)OnModuleStart);

	UnprotectExtraMemory();

	initAdrenaline();

	memcpy(&rebootex_config, (void *)EPI_REBOOTEX_CFG_ADDR, sizeof(RebootexConfig));

	tty_init();

	return 0;
}
