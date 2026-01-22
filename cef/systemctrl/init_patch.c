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

#include <stdio.h>
#include <string.h>

#include <pspinit.h>
#include <psploadcore.h>
#include <pspmodulemgr.h>

#include <cfwmacros.h>
#include <systemctrl_adrenaline.h>

#include <adrenaline_log.h>

#include "main.h"
#include "plugin.h"

// init.prx Custom sceKernelStartModule Handler
int (* custom_start_module_handler)(int modid, SceSize argsize, void * argp, int * modstatus, SceKernelSMOption * opt) = NULL;

static SceUID (* sceKernelLoadModuleMs2Handler)(const char *path, int flags, SceKernelLMOption *option);
static SceUID (* LoadModuleBufferAnchorInBtcnf)(void *buf, int a1);

u32 g_init_addr = 0;

static int g_leda_apitype;
static int g_plugindone = 0;

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
	return sceKernelLoadModuleMs2(g_leda_apitype, path, flags, option);
}

SceUID sceKernelLoadModuleMs2Init(int apitype, const char *path, int flags, SceKernelLMOption *option) {
	g_leda_apitype = apitype;
	return sceKernelLoadModuleMs2Handler(path, flags, option);
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
		if (g_cfw_config.skip_logo || g_cfw_config.startup_program) {
			static u32 vshmain_args[0x100];
			memset(vshmain_args, 0, sizeof(vshmain_args));

			if (argp != NULL && argsize != 0) {
				memcpy(vshmain_args, argp, argsize);
			}

			// Init param
			vshmain_args[0] = sizeof(vshmain_args);
			vshmain_args[1] = 0x20;
			vshmain_args[16] = 1;

			if(g_cfw_config.startup_program && argsize == 0) {
				LoadExecForKernel_AA2029EC();

				SceKernelLoadExecVSHParam param;
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

	if (!g_plugindone) {
		char *waitmodule;

		if (sceKernelFindModuleByName("sceNp9660_driver")) {
			waitmodule = "sceMeCodecWrapper";
		} else {
			waitmodule = "sceMediaSync";
		}

		if (sceKernelFindModuleByName(waitmodule) != NULL) {
			g_plugindone = 1;

			int type = sceKernelApplicationType();

			if (type == PSP_INIT_KEYCONFIG_VSH && !g_cfw_config.no_xmbctrl) {
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
	g_init_addr = ((u32)module_bootstart) - 0x1A54;

	MAKE_JUMP(g_init_addr + 0x1C5C, sceKernelStartModulePatched);

	sctrlFlushCache();

	// configure Pentazemin
	sctrlPentazeminConfigure(g_cfw_config.use_sony_psposk, g_cfw_config.use_ge2, g_cfw_config.use_me2);

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
	MAKE_JUMP(g_init_addr + 0x1C84, sceKernelLoadModuleMs2Init);

	sctrlFlushCache();

	return 0;
}
