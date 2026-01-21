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

#include <systemctrl_se.h>

#include <systemctrl_adrenaline.h>

#include <adrenaline_log.h>

#include "main.h"
#include "../../adrenaline_compat.h"

extern SceAdrenaline *adrenaline;

typedef struct LbaParams {
	int unknown1; // 0
	int cmd; // 4
	int lba_top; // 8
	int lba_size; // 12
	int byte_size_total;  // 16
	int byte_size_centre; // 20
	int byte_size_start; // 24
	int byte_size_last;  // 28
} LbaParams;

SceGameInfo default_gameinfo = {
	.title_id = "HOME0000\0\0\0\0\0\0\0\0",
};

// 0 - Not able to get | 1 - Able to get
int readTitleIdFromDisc() {
	int disc_fd = sceIoOpen("disc0:/UMD_DATA.BIN", PSP_O_RDONLY, 0777);

	if (disc_fd < 0) {
		return 0;
	}

	// Country code
	sceIoRead(disc_fd, rebootex_config.title_id, 4);
	// Delimiter skip
	sceIoLseek(disc_fd, 1, PSP_SEEK_CUR);
	// Title ID
	sceIoRead(disc_fd, rebootex_config.title_id + 4, 5);

	sceIoClose(disc_fd);
	return 1;
}

// 0 - Not able to get | 1 - Able to get
int readTitleIdFromPBP() {
	u32 size = sizeof(rebootex_config.title_id);
	int res = sctrlGetInitPARAM("DISC_ID", NULL, &size, rebootex_config.title_id);

	if (res < 0) {
		return 0;
	}
	return 1;
}

// 0 - Not able to get | 1 - Able to get
int readTitleIdFromISO() {
	int (*isoGetTitleId)(char title_id[10]) = NULL;
	int boot_conf = rebootex_config.bootfileindex;

	switch (boot_conf) {
		case MODE_INFERNO:
			isoGetTitleId = (void*)sctrlHENFindFunction("EPI-InfernoDriver", "inferno_driver", 0xD4FAB33F);
			break;

		case MODE_MARCH33:
			isoGetTitleId = (void*)sctrlHENFindFunction("EPI-March33Driver", "march33_driver", 0xD4FAB33F);
			break;

		case MODE_NP9660:
			isoGetTitleId = (void*)sctrlHENFindFunction("EPI-GalaxyController", "galaxy_driver", 0xD4FAB33F);
			break;

		default:
			break;
	}

	if (isoGetTitleId == NULL) {
		logmsg4("%s: [DEBUG]: Not found `isoGetTitleId`\n", __func__);
		return 0;
	}

	logmsg4("%s: [DEBUG]: Found `isoGetTitleId`\n", __func__);

	return isoGetTitleId(rebootex_config.title_id);
}

void findAndSetTitleId() {
	int apitype = sceKernelInitApitype();
	if (apitype == PSP_INIT_APITYPE_MS2 || apitype == PSP_INIT_APITYPE_EF2 || apitype == PSP_INIT_APITYPE_UMD || apitype == PSP_INIT_APITYPE_UMD2 || apitype >= PSP_INIT_APITYPE_VSH_KERNEL) {
		return;
	}

	if (rebootex_config.title_id[0] == '\0') {
		int is_iso = rebootex_config.bootfileindex >= MODE_OE_LEGACY && rebootex_config.bootfileindex <= MODE_ME;
		if (is_iso) {
			readTitleIdFromISO();
		} else {
			readTitleIdFromPBP();
		}
	}

	SceGameInfo* gameinfo = sceKernelGetGameInfo();
	if (gameinfo != NULL) {
		memcpy(gameinfo->title_id, rebootex_config.title_id, 9);
	}

	if (rebootex_config.title_id[0] == '\0') {
		memcpy(default_gameinfo.title_id, rebootex_config.title_id, 9);
	}

	if (rebootex_config.title_id[0] == '\0' && adrenaline != NULL) {
		memcpy(adrenaline->titleid, rebootex_config.title_id, 9);
	}
}

SceGameInfo* sceKernelGetGameInfoPatched() {
	SceGameInfo* gameinfo = sceKernelGetGameInfo();

	if (gameinfo == NULL) {
		return gameinfo;
	}

	if (rebootex_config.title_id[0] != 0) {
		memcpy(gameinfo->title_id, rebootex_config.title_id, 9);
	}

	return gameinfo;
}

void PatchGameInfoGetter(SceModule* mod) {
	// Kernel module
	if((mod->text_addr & 0x80000000) != 0) {
		u32 func = sctrlHENFindFunctionInMod(mod, "SysMemForKernel", 0xEF29061C);

		if (func != 0) {
			REDIRECT_FUNCTION(func, sceKernelGetGameInfoPatched);
		}
	}
}