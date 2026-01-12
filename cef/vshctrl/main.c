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

#include <string.h>

#include <systemctrl.h>
#include <systemctrl_se.h>

#define _ADRENALINE_LOG_IMPL_
#include <adrenaline_log.h>

#include "patch_vsh.h"

PSP_MODULE_INFO("VshControl", 0x1007, 1, 2);

STMOD_HANDLER previous;

SEConfigADR g_cfw_config;
u32 g_firsttick;

int OnModuleStart(SceModule *mod) {
	char *modname = mod->modname;

	if (strcmp(modname, "vsh_module") == 0) {
		PatchVshMain(mod);
	} else if (strcmp(modname, "sysconf_plugin_module") == 0) {
		PatchSysconfPlugin(mod);
	} else if (strcmp(modname, "game_plugin_module") == 0) {
		PatchGamePlugin(mod);
	} else if (strcmp(modname, "update_plugin_module") == 0) {
		PatchUpdatePlugin(mod);
	}

	if (!previous)
		return 0;

	return previous(mod);
}

int module_start(SceSize args, void *argp) {
	logInit("ms0:/log_vshctrl.txt");
	logmsg("VshCtrl started\n");

	PatchLoadExec();

	sctrlSEGetConfig(&g_cfw_config);

	if (g_cfw_config.vsh_cpu_speed != 0) {
		g_firsttick = sceKernelGetSystemTimeLow();
	}

	previous = sctrlHENSetStartModuleHandler(OnModuleStart);

	return 0;
}