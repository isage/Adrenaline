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
#include <pspsdk.h>
#include <psploadcore.h>
#include <pspiofilemgr.h>
#include <systemctrl.h>
#include <psperror.h>
#include <macros.h>
#include <adrenaline_log.h>

#include "main.h"

static char *g_blacklist[] = {
	"iso",
	"seplugins",
	"isocache",
	"irshell",
};

////////////////////////////////////////////////////////////////////////////////
// HELPERS
////////////////////////////////////////////////////////////////////////////////

static inline int is_in_blacklist(const char *dname) {
    // lower string
    char temp[255];
	memset(temp, 0, sizeof(temp));
    strncpy(temp, dname, sizeof(temp));

    lowerString(temp, temp, strlen(temp)+1);

    for (int i=0; i<NELEMS(g_blacklist); ++i) {
        if (strstr(temp, g_blacklist[i])) {
        	return 1;
        }
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// PATCHED IMPLEMENTATIONS
////////////////////////////////////////////////////////////////////////////////

SceUID sceIoOpenDrmPatched(const char *path, int flags, SceMode mode);
SceUID sceIoOpenHidePatched(const char *path, int flags, SceMode mode) {
	int k1 = pspSdkSetK1(0);

	if (is_in_blacklist(strrchr(path,'/')+1)) {
		logmsg2("[INFO]: %s: Game tried to access CFW files: %s\n", __func__, path);
		return SCE_KERR_ILLEGAL_ACCESS;
	}

	int res = SCE_KERR_ILLEGAL_ACCESS;
	if (config.no_nodrm_engine) {
		res = sceIoOpen(path, flags, mode);
	} else {
		// Handle the case where we are be substituting the DrmPatched version
		// of this function
		res = sceIoOpenDrmPatched(path, flags, mode);
	}

	pspSdkSetK1(k1);
    return res;
}

int sceIoDreadHidePatched(SceUID fd, SceIoDirent * dir) {
    int k1 = pspSdkSetK1(0);
    int res = sceIoDread(fd, dir);

    if (res > 0 && is_in_blacklist(dir->d_name)) {
        res = sceIoDread(fd, dir);
    }

    pspSdkSetK1(k1);
    return res;
}

int sceIoRemoveHidePatched(const char *path) {
	int k1 = pspSdkSetK1(0);

	if (is_in_blacklist(strrchr(path,'/')+1)) {
		logmsg2("[INFO]: %s: Game tried to access CFW files: %s\n", __func__, path);
		return SCE_KERR_ILLEGAL_ACCESS;
	}

	int res = sceIoRemove(path);
	pspSdkSetK1(k1);
	return res;
}

int sceIoGetstatHidePatched(const char *path, SceIoStat *stat) {
	int k1 = pspSdkSetK1(0);

	if (is_in_blacklist(strrchr(path,'/')+1)) {
		logmsg2("[INFO]: %s: Game tried to access CFW files: %s\n", __func__, path);
		return SCE_KERR_ILLEGAL_ACCESS;
	}

	int res = sceIoGetstat(path, stat);
	pspSdkSetK1(k1);
	return res;
}

int sceIoChstatHidePatched(const char *path, SceIoStat *stat, int bits) {
	int k1 = pspSdkSetK1(0);

	if (is_in_blacklist(strrchr(path,'/')+1)) {
		logmsg2("[INFO]: %s: Game tried to access CFW files: %s\n", __func__, path);
		return SCE_KERR_ILLEGAL_ACCESS;
	}

	int res = sceIoChstat(path, stat, bits);
	pspSdkSetK1(k1);
	return res;
}

int sceIoRmdirHidePatched(const char *path) {
	int k1 = pspSdkSetK1(0);

	if (is_in_blacklist(strrchr(path,'/')+1)) {
		logmsg2("[INFO]: %s: Game tried to access CFW files: %s\n", __func__, path);
		return SCE_KERR_ILLEGAL_ACCESS;
	}

	int res = sceIoRmdir(path);

	pspSdkSetK1(k1);
	return res;
}


////////////////////////////////////////////////////////////////////////////////
// MODULE PATCHERS
////////////////////////////////////////////////////////////////////////////////

// Should be called **after** `PatchDrmGameModule`
void PatchHideCfwFolders(SceModule* mod) {
	int apitype = sceKernelInitApitype();

	// Do not apply patches:
	// 1. If it is an (unsigned) homebrew running (SCE_APITYPE_MS2 and SCE_APITYPE_EF2)
	// 2. If it is configured to not hide even on games
	if (apitype == SCE_APITYPE_MS2 || apitype == SCE_APITYPE_EF2 || config.no_hide_cfw_files) {
		return;
	}

    sctrlHENHookImportByNID(mod, "IoFileMgrForUser", 0xE3EB004C, &sceIoDreadHidePatched, 0);
    sctrlHENHookImportByNID(mod, "IoFileMgrForUser", 0xB8A740F4, &sceIoChstatHidePatched, 0);
    sctrlHENHookImportByNID(mod, "IoFileMgrForUser", 0xACE946E8, &sceIoGetstatHidePatched, 0);
    sctrlHENHookImportByNID(mod, "IoFileMgrForUser", 0xF27A9C51, &sceIoRemoveHidePatched, 0);
    sctrlHENHookImportByNID(mod, "IoFileMgrForUser", 0x1117C65F, &sceIoRmdirHidePatched, 0);
    sctrlHENHookImportByNID(mod, "IoFileMgrForUser", 0x109F50BC, &sceIoOpenHidePatched, 0);

	sctrlFlushCache();
}