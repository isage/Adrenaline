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

#include <common.h>
#include <pspmodulemgr.h>
#include <psploadcore.h>
#include <adrenaline_log.h>

#include "main.h"
#include "plugin.h"

// init.prx Custom sceKernelStartModule Handler
int (* custom_start_module_handler)(int modid, SceSize argsize, void * argp, int * modstatus, SceKernelSMOption * opt) = NULL;

SceUID (* sceKernelLoadModuleMs2Handler)(const char *path, int flags, SceKernelLMOption *option);
SceUID (* LoadModuleBufferAnchorInBtcnf)(void *buf, int a1);

u32 init_addr = 0;
int leda_apitype;

int plugindone = 0;

////////////////////////////////////////////////////////////////////////////////
// HELPERS
////////////////////////////////////////////////////////////////////////////////

static void loadXmbControl(){
	int apitype = sceKernelInitApitype();
	if (apitype == 0x200 || apitype ==  0x210 || apitype ==  0x220 || apitype == 0x300){
		// load XMB Control Module
		int modid = sceKernelLoadModule("ms0:/__ADRENALINE__/flash0/vsh/module/xmbctrl.prx", 0, NULL);
		if (modid < 0) {
		  	modid = sceKernelLoadModule("flash0:/vsh/module/xmbctrl.prx", 0, NULL);
		}
		sceKernelStartModule(modid, 0, NULL, NULL, NULL);
	}
}

////////////////////////////////////////////////////////////////////////////////
// PATCHED IMPLEMENTATIONS
////////////////////////////////////////////////////////////////////////////////

SceUID sceKernelLoadModuleMs2Leda(const char *path, int flags, SceKernelLMOption *option) {
	return sceKernelLoadModuleMs2(leda_apitype, path, flags, option);
}

SceUID sceKernelLoadModuleMs2Init(int apitype, const char *path, int flags, SceKernelLMOption *option) {
	leda_apitype = apitype;
	return sceKernelLoadModuleMs2Handler(path, flags, option);
}

SceUID sceKernelLoadModuleBufferBootInitBtcnfPatched(SceLoadCoreBootModuleInfo *info, void *buf, int flags, SceKernelLMOption *option) {
	PSPKeyConfig app_type = sceKernelApplicationType();

	if (config.use_sony_psposk) {
		if (strcmp(info->name, "/kd/kermit_utility.prx") == 0) {
			info->name = "/kd/utility.prx";
		}
	}

	if (config.use_ge2 && app_type != PSP_INIT_KEYCONFIG_VSH) {
		if (strcmp(info->name, "/kd/ge.prx") == 0) {
			info->name = "/kd/ge_2.prx";

			char* path = "flash0:/kd/ge_2.prx";
			SceUID mod = sceKernelLoadModule(path, 0, NULL);
			logmsg4("[DEBUG]: sceKernelLoadModule: path=%s -> 0x%08X\n", path, mod);
			if (mod >= 0) {
				return mod;
			}
		}
	}

	if (config.use_me2 && app_type != PSP_INIT_KEYCONFIG_VSH) {
		if (strcmp(info->name, "/kd/kermit_me_wrapper.prx") == 0) {
			info->name = "/kd/kermit_me_wrapper_2.prx";

			char* path = "flash0:/kd/kermit_me_wrapper_2.prx";
			SceUID mod = sceKernelLoadModule(path, 0, NULL);
			logmsg4("[DEBUG]: sceKernelLoadModule: path=%s -> 0x%08X\n", path, mod);
			if (mod >= 0) {
				return mod;
			}
		}
	}

	char path[64];
	sprintf(path, "ms0:/__ADRENALINE__/flash0%s", info->name); //not use flash0 cause of cxmb

	SceUID mod = sceKernelLoadModule(path, 0, NULL);
	logmsg4("[DEBUG]: sceKernelLoadModule: path=%s -> 0x%08X\n", info->name, mod);
	if (mod >= 0) {
		return mod;
	}

	return sceKernelLoadModuleBufferBootInitBtcnf(info->size, buf, flags, option);
}

SceUID LoadModuleBufferAnchorInBtcnfPatched(void *buf, SceLoadCoreBootModuleInfo *info) {
	char path[64];
	sprintf(path, "ms0:/__ADRENALINE__/flash0%s", info->name);

	SceUID mod = sceKernelLoadModule(path, 0, NULL);
	if (mod >= 0) {
		return mod;
	}

	return LoadModuleBufferAnchorInBtcnf(buf, (info->attr >> 8) & 1);
}

int sceKernelStartModulePatched(SceUID modid, SceSize argsize, void *argp, int *status, SceKernelSMOption *option) {
	SceModule *mod = sceKernelFindModuleByUID(modid);
	SceUID fpl = -1;
	char *plug_buf = NULL;
	int res;

	if (mod == NULL) {
		goto START_MODULE;
	}

	if (strcmp(mod->modname, "vsh_module") == 0) {
		if (config.skip_logo || config.startup_program) {
			static u32 vshmain_args[0x100];
			memset(vshmain_args, 0, sizeof(vshmain_args));

			if (argp != NULL && argsize != 0) {
				memcpy(vshmain_args, argp, argsize);
			}

			// Init param
			vshmain_args[0] = sizeof(vshmain_args);
			vshmain_args[1] = 0x20;
			vshmain_args[16] = 1;

			if(config.startup_program && argsize == 0) {
				LoadExecForKernel_AA2029EC();

				struct SceKernelLoadExecVSHParam param;
				memset(&param, 0, sizeof(param));
				param.size = sizeof(param);
				param.args = sizeof("ms0:/PSP/GAME/BOOT/EBOOT.PBP");
				param.argp = "ms0:/PSP/GAME/BOOT/EBOOT.PBP";
				param.key = "game";

				sctrlKernelLoadExecVSHMs2("ms0:/PSP/GAME/BOOT/EBOOT.PBP", &param);
			}

			argsize = sizeof(vshmain_args);
			argp = vshmain_args;

		}
	}

	if (custom_start_module_handler != NULL) {
		res = custom_start_module_handler(modid, argsize, argp, status, option);

		if (res < 0) {
			logmsg3("[INFO]: %s: Custom start module handler failed with res=0x%08X\n", __func__, res);
		}
	}

	if (!plugindone) {
		char *waitmodule;

		if (sceKernelFindModuleByName("sceNp9660_driver")) {
			waitmodule = "sceMeCodecWrapper";
		} else {
			waitmodule = "sceMediaSync";
		}

		if (sceKernelFindModuleByName(waitmodule) != NULL) {
			plugindone = 1;

			int type = sceKernelApplicationType();

			if (type == PSP_INIT_KEYCONFIG_VSH && !config.no_xmbctrl) {
				loadXmbControl();
			}

			if (type == PSP_INIT_KEYCONFIG_VSH && !sceKernelFindModuleByName("scePspNpDrm_Driver")) {
				goto START_MODULE;
			}

			loadPlugins();
		}
	}

START_MODULE:
	res = sceKernelStartModule(modid, argsize, argp, status, option);

	if (plug_buf) {
		sceKernelFreeFpl(fpl, plug_buf);
		sceKernelDeleteFpl(fpl);
	}

	return res;
}

////////////////////////////////////////////////////////////////////////////////
// MODULE PATCHERS
////////////////////////////////////////////////////////////////////////////////

int PatchInit(int (* module_bootstart)(SceSize, void *), void *argp) {
	init_addr = ((u32)module_bootstart) - 0x1A54;

	// Ignore StopInit
	MAKE_NOP(init_addr + 0x18EC);

	// Redirect load functions to load from MS
	LoadModuleBufferAnchorInBtcnf = (void *)init_addr + 0x1038;
	MAKE_CALL(init_addr + 0x17E4, LoadModuleBufferAnchorInBtcnfPatched);
	MAKE_INSTRUCTION(init_addr + 0x17E8, 0x02402821); // move $a1, $s2

	MAKE_INSTRUCTION(init_addr + 0x1868, 0x02402021); // move $a0, $s2
	MAKE_CALL(init_addr + 0x1878, sceKernelLoadModuleBufferBootInitBtcnfPatched);

	MAKE_JUMP(init_addr + 0x1C5C, sceKernelStartModulePatched);

	sctrlFlushCache();

	return module_bootstart(4, argp);
}

////////////////////////////////////////////////////////////////////////////////
// CFW API (That depends on this file globals/statics)
////////////////////////////////////////////////////////////////////////////////

int sctrlHENRegisterHomebrewLoader(int (* handler)(const char *path, int flags, SceKernelLMOption *option)) {
	sceKernelLoadModuleMs2Handler = handler;
	u32 text_addr = ((u32)handler) - 0xCE8;

	// Remove version check
	MAKE_NOP(text_addr + 0xC58);

	// Remove patch of sceKernelGetUserLevel on sceLFatFs_Driver
	MAKE_NOP(text_addr + 0x1140);

	// Fix sceKernelLoadModuleMs2 call
	MAKE_JUMP(text_addr + 0x2E28, sceKernelLoadModuleMs2Leda);
	MAKE_JUMP(init_addr + 0x1C84, sceKernelLoadModuleMs2Init);

	sctrlFlushCache();

	return 0;
}