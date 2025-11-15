/*
	Adrenaline PopCorn
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

#include <cfwmacros.h>
#include <systemctrl.h>

#define _ADRENALINE_LOG_IMPL_
#include <adrenaline_log.h>

#include "popcorn.h"

PSP_MODULE_INFO("EPI-PopcornManager", 0x1007, 1, 1);

STMOD_HANDLER previous;

int (* setCompiledSdkVersion)(u32 ver) = NULL;
static void setPsxCompiledFwVersion(u32 version) {
	if (NULL == setCompiledSdkVersion) {
		setCompiledSdkVersion = (void*)sctrlHENFindFunction("sceSystemMemoryManager", "SysMemUserForUser", 0x358CA1BB);
	}

	setCompiledSdkVersion(version);
}

int OnModuleStart(SceModule *mod) {
	if (strcmp(mod->modname, "pops") == 0) {
		PatchPops(mod);
	}

	if (!previous){
		return 0;
	}

	return previous(mod);
}

int module_start(SceSize args, void *argp) {
	logInit("ms0:/log_popcorn.txt");
	logmsg("Popcorn started...\n");

	int res = initGlobals();
	if (res < 0) {
		return res;
	}

	setPsxCompiledFwVersion(FW_661);

	previous = sctrlHENSetStartModuleHandler(OnModuleStart);

	PatchScePopsMgr();

	return 0;
}