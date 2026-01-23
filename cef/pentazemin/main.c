/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#include <string.h>

#include <pspsdk.h>
#include <pspdisplay.h>
#include <pspsysmem_kernel.h>

#include <cfwmacros.h>
#include <systemctrl.h>
#include <systemctrl_adrenaline.h>

#define _ADRENALINE_LOG_IMPL_
#include <adrenaline_log.h>

#include "externs.h"
#include "ttystdio.h"
#include "adrenaline.h"
#include "modulepatches.h"


PSP_MODULE_INFO("Pentazemin", 0x1007, 1, 0);

// Previous Module Start Handler
static STMOD_HANDLER previous = NULL;


void OnSystemStatusIdle() {
	initAdrenalineInfo();
	PatchVolatileMemBug();

	// Set fake framebuffer so that cwcheat can be displayed
	if (g_adrenaline->pops_mode) {
		sceDisplaySetFrameBuf((void *)NATIVE_FRAMEBUFFER, PSP_SCREEN_LINE, PSP_DISPLAY_PIXEL_FORMAT_8888, PSP_DISPLAY_SETBUF_NEXTFRAME);
		memset((void *)NATIVE_FRAMEBUFFER, 0, SCE_PSPEMU_FRAMEBUFFER_SIZE);
	} else {
		sctrlSendAdrenalineCmd(ADRENALINE_VITA_CMD_RESUME_POPS, 0);
	}
	sctrlSendAdrenalineCmd(ADRENALINE_VITA_CMD_APP_STARTED, 0);
}


int PentazeminOnModuleStart(SceModule * mod) {

	// System fully booted Status
	static int booted = 0;

	if (strcmp(mod->modname, "sceLowIO_Driver") == 0) {
		// Protect pops memory
		if (sceKernelApplicationType() == PSP_INIT_KEYCONFIG_POPS) {
			sceKernelAllocPartitionMemory(6, "", PSP_SMEM_Addr, 0x80000, (void *)0x09F40000);
		}

		memset((void *)0x49F40000, 0, 0x80000);
		memset((void *)0xABCD0000, 0, 0x1B0);

		PatchLowIODriver2(mod);

	}else if (strcmp(mod->modname, "sceLoadExec") == 0) {
		PatchLoadExec(mod);

	} else if (strcmp(mod->modname, "scePower_Service") == 0) {
		PatchPowerService(mod);
		PatchPowerService2(mod);

	} else if (strcmp(mod->modname, "sceChkreg") == 0) {
		PatchChkreg(mod);

	} else if (strcmp(mod->modname, "sceMesgLed") == 0) {
		PatchMesgLed(mod);

	} else if (strcmp(mod->modname, "sceUmd_driver") == 0) {
		PatchUmdDriver(mod);

	} else if(strcmp(mod->modname, "sceMeCodecWrapper") == 0) {
		PatchMeCodecWrapper(mod);

	} else if (strcmp(mod->modname, "sceUtility_Driver") == 0) {
		PatchUtility();

	} else if (strcmp(mod->modname, "sceUSBCam_Driver") == 0) {
		PatchUSBCamDriver(mod);

	} else if (strcmp(mod->modname, "sceImpose_Driver") == 0) {
		PatchImposeDriver(mod);

	} else if (strcmp(mod->modname, "sceSAScore") == 0) {
		PatchSasCore(mod);

	} else if (strcmp(mod->modname, "scePops_Manager") == 0){
		PatchPopsMgr(mod);

	} else if (strcmp(mod->modname, "pops") == 0){
		PatchPops(mod);

	} else if (strcmp(mod->modname, "sysconf_plugin_module") == 0){
		PatchSysconfPlugin(mod);

	} else if (strcmp(mod->modname, "CWCHEATPRX") == 0) {
		PatchCwCheatPlugin(mod);

	} else if(strcmp(mod->modname, "VLF_Module") == 0) {
		PatchVlfLib(mod);
	}

	// Boot Complete Action not done yet
	if(booted == 0) {
		// Boot is complete
		if(sctrlHENIsSystemBooted()) {
			// Adrenaline patches
			OnSystemStatusIdle();

			// Boot Complete Action done
			booted = 1;
		}
	}

	// Forward to previous Handler
	if(previous) {
		return previous(mod);
	}

	return 0;
}

// Boot Time Entry Point
int module_start(SceSize args, void * argp) {
	logInit("ms0:/log_pentazemin.txt");
	logmsg("Pentazemin started...\n");

	PatchSysmem();
	PatchLoaderCore();
	PatchIoFileMgr();
	PatchMemlmd();

	// initialize Adrenaline Layer
	initAdrenaline();

	// Register Module Start Handler
	previous = sctrlHENSetStartModuleHandler(PentazeminOnModuleStart);

	PatchMemUnlock();

	tty_init();
	sctrlFlushCache();

	// Return Success
	return 0;
}
