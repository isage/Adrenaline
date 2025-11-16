/*
	Adrenaline Galaxy Controller
	Copyright (C) 2025, GrayJack
	Copyright (C) 2021, PRO CFW
	Copyright (C) 2008, M33 Team Developers (Dark_Alex, adrahil, Mathieulh)

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
#include <systemctrl_se.h>

#define _ADRENALINE_LOG_IMPL_
#include <adrenaline_log.h>

#include "galaxy.h"
#include "../bits/iso_common.h"

PSP_MODULE_INFO("EPI-GalaxyController", 0x1006, 1, 0);

int module_start(SceSize args, void* argp) {
	logInit("ms0:/log_galaxy.txt");
	logmsg("Galaxy driver started...\n")

	// Get ISO path
	memset(g_iso_fn, 0, sizeof(g_iso_fn));
	strncpy(g_iso_fn, sctrlSEGetUmdFile(), sizeof(g_iso_fn)-1);
	logmsg3("[INFO] UMD File: %s\n", g_iso_fn);

	// Leave NP9660 alone, we got no ISO
	if(g_iso_fn[0] == 0) {
		return 1;
	}

	PatchThreadManager();

	// ISO File Descriptor
	int fd = -1;

	// Wait for MS
	while (1) {
		fd = sceIoOpen(g_iso_fn, PSP_O_RDONLY, 0);

		if (fd >= 0) {
			break;
		}

		// Delay and retry
		sceKernelDelayThread(10000);
	}

	sceIoClose(fd);

	return 0;
}

