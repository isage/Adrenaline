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

#include <common.h>
#include <adrenaline_log.h>

#include "utils.h"
#include "main.h"

extern RebootexConfig rebootex_config;
extern AdrenalineConfig config;

STMOD_HANDLER game_previous = NULL;

////////////////////////////////////////////////////////////////////////////////
// HELPERS
////////////////////////////////////////////////////////////////////////////////

static void SetUmdEmuSpeed(u8 seek, u8 read) {
	void (*SetUmdDelay)(int, int) = NULL;
	int (*CacheInit)(int, int, int) = NULL;

	// Config in `Auto` mode
	if (config.umd_seek == 0 && config.umd_speed == 0) {
		if (rebootex_config.bootfileindex == BOOT_INFERNO) {
			SceModule* inferno_mod = sceKernelFindModuleByName("EPI-InfernoDriver");

			SetUmdDelay = (void*)sctrlHENFindFunctionInMod(inferno_mod, "inferno_driver", 0xB6522E93);
			CacheInit = (void*)sctrlHENFindFunctionInMod(inferno_mod, "inferno_driver", 0x8CDE7F95);
		} else if (rebootex_config.bootfileindex == BOOT_MARCH33) {
			SetUmdDelay = (void*)sctrlHENFindFunction("EPI-March33Driver", "march33_driver", 0xFAEC97D6);
		} else if (rebootex_config.bootfileindex == BOOT_NP9660) {
			SetUmdDelay = (void*)sctrlHENFindFunction("EPI-GalaxyController", "galaxy_driver", 0xFAEC97D6);
		}

		if (SetUmdDelay != NULL) {
			SetUmdDelay(seek, read);
		}

		if (CacheInit != NULL && config.iso_cache != CACHE_CONFIG_OFF) {
			// Disable Inferno cache
			CacheInit(0, 0, 0);
		}
	}
}

static void DisableInfernoCache() {
	int (*CacheInit)(int, int, int) = NULL;
	if (rebootex_config.bootfileindex == BOOT_INFERNO) {
		CacheInit = (void*)sctrlHENFindFunction("EPI-InfernoDriver", "inferno_driver", 0x8CDE7F95);

		if (CacheInit != NULL) {
			CacheInit(0, 0, 0);
		}

		// Modify the CFW setting at runtime without saving.
		// Avoids the CFW to try to re-enable the cache once again
		config.iso_cache = CACHE_CONFIG_OFF;
	}
}

////////////////////////////////////////////////////////////////////////////////
// PATCHED IMPLEMENTATIONS
////////////////////////////////////////////////////////////////////////////////

static int (*_utilityGetParam)(int, int*) = NULL;
static int utilityGetParamPatched_ULJM05221(int param, int* value) {
	int res = _utilityGetParam(param, value);
	if (param == PSP_SYSTEMPARAM_ID_INT_LANGUAGE && *value > 1) {
		*value = 0;
	}
	return res;
}

int moduleLoaderJackass(char* name, int value) {
	char path[256] = {0};

	SceKernelLMOption option;
	memset(&option, 0, sizeof(option));
	option.size = sizeof(option);
	option.mpidtext = 2;
	option.mpiddata = 2;
	option.position = 0;
	option.access = 1;

	int res = sceKernelLoadModule(path, 0, &option);
	if (res >= 0) {
		int status;
		res = sceKernelStartModule(res,0,0,&status,0);
	}
	return res;
}

static int wweModuleOnStart(SceModule * mod) {
	// Boot Complete Action not done yet
	if (strcmp(mod->modname, "mainPSP") == 0) {
		sctrlHENHookImportByNID(mod, "scePower", 0x34F9C463, NULL, 222); // scePowerGetPllClockFrequencyInt
		sctrlHENHookImportByNID(mod, "scePower", 0x843FBF43, NULL, 0);   // scePowerSetCpuClockFrequency
		sctrlHENHookImportByNID(mod, "scePower", 0xFDB5BFE9, NULL, 222); // scePowerGetCpuClockFrequencyInt
		sctrlHENHookImportByNID(mod, "scePower", 0xBD681969, NULL, 111); // scePowerGetBusClockFrequencyInt
	}

	// Call Previous Module Start Handler
	if(game_previous) {
		game_previous(mod);
	}

	return 0;
}

// Reimplement the function without cache
int sceWlanGetSwitchStatePatched() {
	int k1 = pspSdkSetK1(0);

	char buf[sizeof(SceKermitRequest) + 0x40];
	SceKermitRequest *request_aligned = (SceKermitRequest *)ALIGN((u32)buf, 0x40);
	SceKermitRequest *request_uncached = (SceKermitRequest *)((u32)request_aligned | 0x20000000);
	sceKernelDcacheInvalidateRange(request_aligned, sizeof(SceKermitRequest));

	u64 resp;
	int res = sceKermitSendRequest(request_uncached, KERMIT_MODE_WLAN, 0x49, 0, 0, &resp);

	if (res == 0) {
		res = resp;
	}

	res = res & 1;

	logmsg4("[DEBUG]: %s: () -> 0x%02X\n", __func__, res);
	pspSdkSetK1(k1);
	return res;
}

////////////////////////////////////////////////////////////////////////////////
// MODULE PATCHERS
////////////////////////////////////////////////////////////////////////////////

void PatchGameByTitleIdOnLoadExec() {
	char* title_id = rebootex_config.title_id;

	if (strcasecmp("ULUS10295", title_id) == 0 || strcasecmp("ULES00975", title_id) == 0 || strcasecmp("ULAS42115", title_id) == 0) {
		// Fix The Simpsons Game
		config.use_sony_psposk = 1;
		rebootex_config.overwrite_use_psposk = 1;
		rebootex_config.overwrite_use_psposk_to = 1;
	}
}

void PatchGameByTitleId() {
	char* title_id = rebootex_config.title_id;

	if (strcasecmp("ULJM05221", title_id) == 0) {
		// Fix TwinBee Portable when not using English or Japanese language
		_utilityGetParam = (void*)FindProc("sceUtility_Driver", "sceUtility", 0xA5DA2406);
		sctrlHENPatchSyscall((u32)_utilityGetParam, utilityGetParamPatched_ULJM05221);

	} else if (strcasecmp("ULKS46195", title_id) == 0 || strcasecmp("ULUS10384", title_id) == 0 || strcasecmp("ULES01165", title_id) == 0
		|| strcasecmp("ULES01339", title_id) == 0 || strcasecmp("ULUS10452", title_id) == 0
		|| strcasecmp("ULES01472", title_id) == 0 || strcasecmp("ULUS10543", title_id) == 0) {
		// Patch Smakdown vs RAW 2009/2010/2011 anti-CFW check (CPU speed)
		game_previous = sctrlHENSetStartModuleHandler(wweModuleOnStart);

	} else if (strcasecmp("ULES00590", title_id) == 0 || strcasecmp("ULJM05075", title_id) == 0) {
		// Patch Aces of War anti-CFW check (UMD speed)
		SetUmdEmuSpeed(1, 1);

	} else if (strcasecmp("ULUS10201", title_id) == 0 || strcasecmp("ULUS10328", title_id) == 0 || strcasecmp("ULES00968", title_id) == 0) {
		// Patch Luxor - The Wrath of Set and Flat-Out Head On (US/EU) as they
		// don't play well with Inferno cache
		DisableInfernoCache();

	}
}

void PatchGamesByMod(SceModule* mod) {
	char *modname = mod->modname;

	if (strcmp(modname, "DJMAX") == 0 || strcmp(modname, "djmax") == 0) {
		// Fix Anti-CFW checks on `DJ Max` games
		// Stops it trying to find and maybe deleting CFW folders and their contents
		sctrlHENHookImportByNID(mod, "IoFileMgrForUser", 0xE3EB004C, NULL, 0);

	} else if (strcmp(mod->modname, "ATVPRO") == 0){
		// Remove "overclock" message in `ATV PRO`
		// scePowerSetCpuClockFrequency, scePowerGetCpuClockFrequencyInt and scePowerGetBusClockFrequencyInt respectively
		sctrlHENHookImportByNID(mod, "scePower", 0x843FBF43, NULL, 0);
		sctrlHENHookImportByNID(mod, "scePower", 0xFDB5BFE9, NULL, 222);
		sctrlHENHookImportByNID(mod, "scePower", 0xBD681969, NULL, 111);

	} else if (strcmp(modname, "tekken") == 0) {
		// Fix back screen on `Tekken 6`
		// scePowerGetPllClockFrequencyInt
		sctrlHENHookImportByNID(mod, "scePower", 0x34F9C463, NULL, 222);

	} else if (strcmp(modname, "KHBBS_patch") == 0) {
		MAKE_DUMMY_FUNCTION(mod->entry_addr, 1);

	} else if (strcmp(modname, "Jackass") == 0) {
		// Fix infinite loading screen on `Jackass: The Game`
		char* title_id = rebootex_config.title_id;
		if (strcasecmp("ULES00897", title_id) == 0) { // PAL
			logmsg4("%s: [DEBUG]: Patching Jackass PAL\n", __func__);
			REDIRECT_FUNCTION(mod->text_addr + 0x35A204, sctrlHENMakeSyscallStub(moduleLoaderJackass));

		} else if (strcasecmp("ULUS10303", title_id) == 0) { // NTSC
			logmsg4("%s: [DEBUG]: Patching Jackass NTSC\n", __func__);
			REDIRECT_FUNCTION(mod->text_addr + 0x357B54, sctrlHENMakeSyscallStub(moduleLoaderJackass));
		}

	} else if (strcmp(modname, "projectg_psp") == 0) {
		// Fix black screen on `Pangya Golf Fantasy`
		char* title_id = rebootex_config.title_id;
		u32 addrs[4] = {0};
		if (strcasecmp("ULUS10438", title_id) == 0) { // USA
			addrs[0] = mod->text_addr + 0x35fd88;
			addrs[1] = addrs[0] + 8;
			addrs[2] = addrs[0] + 16;

		} else if (strcasecmp("ULJM05440", title_id) == 0) { // JAPAN
			addrs[0] = mod->text_addr + 0x366498;
			addrs[1] = addrs[0] + 8;
			addrs[2] = addrs[0] + 16;

		} else if (strcasecmp("ULKS46164", title_id) == 0) { // KOREA
			addrs[0] = mod->text_addr + 0x363c50;
			addrs[1] = addrs[0] + 8;
		}

		for (int i = 0; i < 4; i++) {
			u32 addr = addrs[i];
			if (addr != 0) {
				VWRITE32(addr, 0x00000000);
			}
		}

	} else if (strcmp(modname, "BAMG") == 0) {
		// Fix freeze on `Bust-A-Move Deluxe`
		SetUmdEmuSpeed(2, 2);

	} else if (strcmp(modname, "starwars_psp") == 0) {
		// Fix `Lego Star Wars 2` WLAN switch state returning `off` due to cache syncronization issues on ePSP implementation.
		sctrlHENHookImportByNID(mod, "sceWlanDrv", 0xD7763699, sceWlanGetSwitchStatePatched, 0);
	}

	sctrlFlushCache();
}