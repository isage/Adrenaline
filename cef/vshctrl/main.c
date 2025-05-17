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

#include <stddef.h>
#include <string.h>
#include <strings.h>

#include <pspctrl.h>
#include <pspsysmem_kernel.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include "printk.h"


PSP_MODULE_INFO("VshControl", 0x1007, 1, 1);

#define BOOT_BIN "disc0:/PSP_GAME/SYSDIR/BOOT.BIN"
#define EBOOT_BIN "disc0:/PSP_GAME/SYSDIR/EBOOT.BIN"
#define EBOOT_OLD "disc0:/PSP_GAME/SYSDIR/EBOOT.OLD"

#define DUMMY_CAT_ISO_EXTENSION "     "

#define MAX_FILES 128

SceCtrlData *last_control_data = NULL;
char mounted_iso[64];
int has_umd_iso = 0;
u32 psp_model = 0;

AdrenalineConfig config;

void *sceKernelGetGameInfo();

void ClearCaches();

///////////////////////////////////////////////////////

u32 firsttick;
u8 set;

SceUID modid = -1;

int initPatch();

int module_start(SceSize args, void *argp) {
	printkInit("ms0:/log_vshctrl.txt");
	printk("VshCtrl started\n");

	psp_model = sceKernelGetModel();
	sctrlSEGetConfig(&config);

	if (config.vshcpuspeed != 0) {
		firsttick = sceKernelGetSystemTimeLow();
	}

	initPatch();

	// always reset to NORMAL mode in VSH
	// to avoid ISO mode is used in homebrews in next reboot
	char* umdfile = sctrlSEGetUmdFile();
	has_umd_iso = (umdfile[0] != 0 && sctrlSEGetBootConfFileIndex() == BOOT_VSHUMD);
	if (has_umd_iso) strcpy(mounted_iso, umdfile);
	sctrlSESetUmdFile("");
	sctrlSESetBootConfFileIndex(BOOT_NORMAL);

	printkSync();

	return 0;
}