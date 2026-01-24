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

#include <string.h>
#include <systemctrl_adrenaline.h>
#include <pspusbcam.h>
#include <pspdisplay.h>

#include <cfwmacros.h>
#include <systemctrl.h>
#include <infernoctrl.h>

#define _ADRENALINE_LOG_IMPL_
#include <adrenaline_log.h>

#include "main.h"
#include "modulepatches.h"
#include "nodrm.h"
#include "malloc.h"
#include "gameinfo.h"
#include "plugin.h"
#include "utils.h"
#include "storage_cache.h"

#include <systemctrl_adrenaline.h>

PSP_MODULE_INFO("SystemControl", 0x1007, 1, 1);

RebootexConfigEPI g_rebootex_config;
SEConfigEPI g_cfw_config;
SceAdrenaline *g_adrenaline = (SceAdrenaline *)ADRENALINE_ADDRESS;

static int g_idle = 0;

static int g_cpu_list[] = { 0, 20, 75, 100, 133, 222, 266, 300, 333 };
static int g_bus_list[] = { 0, 10, 37,  50,  66, 111, 133, 150, 166 };
static u32 g_cache_size_list[] = { 0, 1024, 2*1024, 4*1024, 8*1024, 16*1024, 32*1024, 64*1024 };
static u8 g_cache_num_list[] = { 0, 1, 2, 4, 8, 16, 32, 64, 128 };

#define N_CPU (sizeof(g_cpu_list) / sizeof(int))
#define N_CACHE_SIZE (sizeof(g_cache_size_list) / sizeof(u32))
#define N_CACHE_NUM (sizeof(g_cache_num_list) / sizeof(u8))

int mallocinit();

static void OnSystemStatusIdle() {

	if (g_cfw_config.no_ms_cache == 0) {
		sctrlMsCacheInit("ms", MSCACHE_BUFSIZE_MIN);
	}

	// March33 UMD seek/speed simulation
	SceModule* march33_mod = sceKernelFindModuleByName("EPI-March33Driver");

	if (march33_mod != NULL) {
		// Handle March33's UMD seek and UMD speed setting
		if (g_cfw_config.umd_seek > 0 || g_cfw_config.umd_speed > 0) {
			g_cfw_config.iso_cache = CACHE_CONFIG_OFF;

			void (*SetUmdDelay)(int, int) = (void*)sctrlHENFindFunctionInMod(march33_mod, "march33_driver", 0xFAEC97D6);

			if (SetUmdDelay != NULL) {
				SetUmdDelay(g_cfw_config.umd_seek, g_cfw_config.umd_speed);
				logmsg3("[INFO]: March33 ISO UMD seek/speed factor: %d seek factor; %d speed factor\n", g_cfw_config.umd_seek, g_cfw_config.umd_speed);
			}
		}
	}

	// NP9660 UMD seek/speed simulation
	SceModule* galaxy_mod = sceKernelFindModuleByName("EPI-GalaxyController");

	if (galaxy_mod != NULL) {
		// Handle March33's UMD seek and UMD speed setting
		if (g_cfw_config.umd_seek > 0 || g_cfw_config.umd_speed > 0) {
			g_cfw_config.iso_cache = CACHE_CONFIG_OFF;

			void (*SetUmdDelay)(int, int) = (void*)sctrlHENFindFunctionInMod(galaxy_mod, "galaxy_driver", 0xFAEC97D6);

			if (SetUmdDelay != NULL) {
				SetUmdDelay(g_cfw_config.umd_seek, g_cfw_config.umd_speed);
				logmsg3("[INFO]: NP9660 ISO UMD seek/speed factor: %d seek factor; %d speed factor\n", g_cfw_config.umd_seek, g_cfw_config.umd_speed);
			}
		}
	}

	// Inferno cache config and UMD seek/speed simulation
	SceModule* inferno_mod = sceKernelFindModuleByName("EPI-InfernoDriver");

	// Inferno driver is loaded
	if (inferno_mod != NULL) {
		// Handle Inferno's UMD seek and UMD speed setting
		if (g_cfw_config.umd_seek > 0 || g_cfw_config.umd_speed > 0) {
			g_cfw_config.iso_cache = CACHE_CONFIG_OFF;

			void (*SetUmdDelay)(int, int) = (void*)sctrlHENFindFunctionInMod(inferno_mod, "inferno_driver", 0xB6522E93);

			if (SetUmdDelay != NULL) {
				SetUmdDelay(g_cfw_config.umd_seek, g_cfw_config.umd_speed);
				logmsg3("[INFO]: Inferno ISO UMD seek/speed factor: %d seek factor; %d speed factor\n", g_cfw_config.umd_seek, g_cfw_config.umd_speed);
			}
		}

		// Handle Inferno Iso cache
		if (g_cfw_config.iso_cache != CACHE_CONFIG_OFF) {
			if (g_rebootex_config.ram2 > 24 || g_cfw_config.force_high_memory != HIGHMEM_OPT_OFF) {
				g_cfw_config.iso_cache_partition = 2;
			} else {
				g_cfw_config.iso_cache_partition = 11;
			}

			int (*CacheInit)(int, int, int) = (void*)sctrlHENFindFunctionInMod(inferno_mod, "inferno_driver", 0x8CDE7F95);
			if (CacheInit != NULL) {
				u32 cache_size = (g_cfw_config.iso_cache_size == ISO_CACHE_SIZE_AUTO) ? 32*1024 : g_cache_size_list[g_cfw_config.iso_cache_size%N_CACHE_SIZE];
				u8 cache_num = (g_cfw_config.iso_cache_num == ISO_CACHE_NUM_AUTO) ? 32 : g_cache_num_list[g_cfw_config.iso_cache_num%N_CACHE_NUM];
				CacheInit(cache_size, cache_num, g_cfw_config.iso_cache_partition);
				logmsg3("[INFO]: Inferno ISO cache: %d caches of %ld KiB in partition %d — Total: %ld KiB\n", cache_num, cache_size/1024, g_cfw_config.iso_cache_partition, (cache_num*cache_size)/1024);
			}

			int (*CacheSetPolicy)(int) = (void*)sctrlHENFindFunctionInMod(inferno_mod, "inferno_driver", 0xC0736FD6);
			if (CacheSetPolicy != NULL) {
				if (g_cfw_config.iso_cache == CACHE_CONFIG_LRU) {
					CacheSetPolicy(INFERNO_CACHE_LRU);
					logmsg3("[INFO]: Inferno ISO cache policy: LRU\n");
				} else if (g_cfw_config.iso_cache == CACHE_CONFIG_RR) {
					CacheSetPolicy(INFERNO_CACHE_RR);
					logmsg3("[INFO]: Inferno ISO cache policy: RR\n");
				}
			}
		}
	}

	// Set CPU/BUS speed on apps/games
	int medium_type = sceKernelBootFrom();
	int app_type = sceKernelApplicationType();
	u8 is_correct_medium = (medium_type == PSP_BOOT_DISC || medium_type == PSP_BOOT_MS || medium_type == PSP_BOOT_EF);

	if (app_type != PSP_INIT_KEYCONFIG_VSH && is_correct_medium) {
		sctrlHENSetSpeed(g_cpu_list[g_cfw_config.app_cpu_speed % N_CPU], g_bus_list[g_cfw_config.app_cpu_speed % N_CPU]);
	}

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
			PatchHideCfwFiles(mod);
			PatchGamesByMod(mod);
		}
		ready_gamepatch_mod = 0;
	}

	// Third-party Plugins modules
	if (sctrlIsLoadingPlugins()) {
		// Fix 6.60 plugins on 6.61
		sctrlHookImportByNID(mod, "SysMemForKernel", 0x3FC9AE6A, &sctrlHENFakeDevkitVersion);
        sctrlHookImportByNID(mod, "SysMemUserForUser", 0x3FC9AE6A, &sctrlHENFakeDevkitVersion);
	}

	if (load_file_config) {
		int res = sctrlSEGetConfig((SEConfig*)&g_cfw_config);
		logmsg("[DEBUG]: sctrlSEGetConfig -> 0x%08X\n", res);
		if (res == 0) {
			load_file_config = 0;
			// configure Pentazemin
			PentazeminConfig penconfig = {
				.osk_type = g_cfw_config.use_sony_psposk,
				.ge_type = g_cfw_config.use_ge2,
				.me_type = g_cfw_config.use_me2,
			};
			sctrlPentazeminConfigure(&penconfig);
		}
	}

	if (strcmp(modname, "sceLowIO_Driver") == 0) {
		if (mallocinit() < 0) {
			while (1) {
				VWRITE32(0, 0);
			}
		}

	} else if (strcmp(modname, "sceKermitMsfs_driver") == 0) {
		load_file_config = 1;

	} else if (strcmp(modname, "sceController_Service") == 0) {
		PatchController(mod);

	} else if (strcmp(modname, "sceLoadExec") == 0) {
		PatchLoadExec(mod);

		#if defined(DEBUG) && DEBUG >= 4
		extern int (* _runExec)(RunExecParams*);
		extern int runExecPatched(RunExecParams* args);
		HIJACK_FUNCTION(text_addr+0x2148, runExecPatched, _runExec);
		#endif // defined(DEBUG) && DEBUG >= 3

		// Hijack all execute calls
		extern int (* _sceLoadExecVSHWithApitype)(int, const char*, SceKernelLoadExecVSHParam*, unsigned int);
		extern int sctrlKernelLoadExecVSHWithApitype(int apitype, const char * file, SceKernelLoadExecVSHParam * param);
		u32 _LoadExecVSHWithApitype = sctrlHENFindFirstJAL(sctrlHENFindFunctionInMod(mod, "LoadExecForKernel", 0xD8320A28));
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

		if (sceKernelApplicationType() == PSP_INIT_KEYCONFIG_GAME  && g_cfw_config.force_high_memory != HIGHMEM_OPT_OFF) {
			if (g_cfw_config.force_high_memory == HIGHMEM_OPT_STABLE) {
				sctrlHENSetMemory(40, 0);
			} else if (g_cfw_config.force_high_memory == HIGHMEM_OPT_MAX) {
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

	} else if (strcmp(modname, "sceChkreg") == 0) {
		PatchChkreg();

	} else if (strcmp(modname, "scePspNpDrm_Driver") == 0) {
		PatchNpDrmDriver(mod);

	} else if (strcmp(modname, "sceNp9660_driver") == 0) {
		PatchNp9660Driver(mod);

	} else if (strcmp(modname, "sceUtility_Driver") == 0) {
		findAndSetTitleId();
		logmsg3("[INFO]: Title ID: %s\n", g_rebootex_config.title_id);
		CheckControllerInput();

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

	} else if (strcmp(modname, "sceKernelLibrary") == 0) { // last kernel module to load before user/game
		ready_gamepatch_mod = 1;
		PatchGameByTitleId();

	} else if (strcmp(modname, "sceMp3_Library") == 0 || (strcmp(modname, "sceVshOSK_Module") == 0 && g_cfw_config.use_sony_psposk)) {
		// Unlock mp3 variable bitrate and qwerty osk on old games/homebrew
		sctrlHookImportByNID(mod, "SysMemUserForUser", 0xFC114573, &sctrlHENFakeDevkitVersion);
		sctrlFlushCache();

	} else if (strcmp(modname, "vsh_module") == 0) {
		PatchVshForDrm(mod);
		PatchDrmOnVsh();

	} else if (strcmp(modname, "sysconf_plugin_module") == 0) {
		PatchSysconfForDrm(mod);

	}

	if (!g_idle) {
		if (sctrlHENIsSystemBooted()) {
			g_idle = 1;
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
	PatchModuleMgr();
	sctrlFlushCache();

	// Restore CFW config from the RAM backup in the start (if it exists)
	if (IS_ADR_SECONFIG(EPI_CONFIG_ADDR)) {
		memcpy(&g_cfw_config, (void *)EPI_CONFIG_ADDR, sizeof(SEConfigEPI));
	}

	sctrlHENSetStartModuleHandler((STMOD_HANDLER)OnModuleStart);

	UnprotectExtraMemory();

	memcpy(&g_rebootex_config, (void *)REBOOTEX_CONFIG, sizeof(RebootexConfigEPI));

	return 0;
}
