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
#include <cfwmacros.h>
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

    for (int i = 0; i < NELEMS(g_blacklist); ++i) {
		char *found = strstr(temp, g_blacklist[i]);
        if (found != NULL) {
			// "ISO" can show up inside other words
			// So if index is of the "iso", and "iso" was found after the start of the lowered string (`temp`)
			// and ".iso" or "/iso", we should consider another word that contains "iso" inside it.
			//
			// Example: God of War - Ghost of Sparta: it tries to find and open `SPA_050_PRISON.BIN`
			if (i == 0 && (int)found > (int)temp && (found-1) != '.' && (found-1) != '/') {
				return 0;
			}
        	return 1;
        }
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// PATCHED IMPLEMENTATIONS
////////////////////////////////////////////////////////////////////////////////

SceUID sceIoDopenHidePatched(const char *dirname) {
	SceUID res = SCE_KERR_ILLEGAL_ACCESS;

	if (is_in_blacklist(dirname)) {
		logmsg2("[INFO]: %s: Game tried to access CFW files: %s\n", __func__, dirname);
		goto exit;
	}

	res = sceIoDopen(dirname);

exit:
	logmsg4("[DEBUG]: %s: dirname=%s -> 0x%08lX\n", __func__, dirname, res);
    return res;
}

int sceIoDreadHidePatched(SceUID fd, SceIoDirent * dir) {
    int k1 = pspSdkSetK1(0);
    int res = sceIoDread(fd, dir);

    if (res > 0 && is_in_blacklist(dir->d_name)) {
		logmsg2("[INFO]: %s: Game tried to access CFW files: %s\n", __func__, dir->d_name);
        res = sceIoDread(fd, dir);
    }

    pspSdkSetK1(k1);
    return res;
}

static SceUID (* _sceIoOpen)(const char *path, int flags, SceMode mode) = sceIoOpen;
SceUID sceIoOpenHidePatched(const char *path, int flags, SceMode mode) {
	SceUID res = SCE_KERR_ILLEGAL_ACCESS;

	if (is_in_blacklist(path)) {
		logmsg2("[INFO]: %s: Game tried to access CFW files: %s\n", __func__, path);
		goto exit;
	}

	res = _sceIoOpen(path, flags, mode);

exit:
	logmsg4("[DEBUG]: %s: path=%s, flags=0x%08X, mode=0x%08X -> 0x%08lX\n", __func__, path, flags, mode, res);
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

void PatchHideCfwFiles(SceModule* mod) {
	int apptype = sceKernelApplicationType();
	int apitype = sceKernelInitApitype();

	// Do not apply patches:
	// 1. Sanity check: Not VSH or UPDATE
	// 2. If it is an (unsigned) homebrew running (SCE_APITYPE_MS2 and SCE_APITYPE_EF2)
	// 3. If it is configured to not hide even on games
	if (apptype == SCE_APPTYPE_VSH || apptype == SCE_APPTYPE_UPDATER || apitype == SCE_APITYPE_MS2 || apitype == SCE_APITYPE_EF2 || config.no_hide_cfw_files) {
		return;
	}

	// The hide CFW files overwrite the hook to `sceIoOpen` made by the DRM
	// patch if both are active. So we use the `sceIoOpenDrmPatched` if that
	// option is enabled
	if (!config.no_nodrm_engine) {
		_sceIoOpen = sceIoOpenDrmPatched;
	}

    sctrlHENHookImportByNID(mod, "IoFileMgrForUser", 0xE3EB004C, sceIoDreadHidePatched, 0);
    sctrlHENHookImportByNID(mod, "IoFileMgrForUser", 0xB8A740F4, sceIoChstatHidePatched, 0);
    sctrlHENHookImportByNID(mod, "IoFileMgrForUser", 0xACE946E8, sceIoGetstatHidePatched, 0);
    sctrlHENHookImportByNID(mod, "IoFileMgrForUser", 0xF27A9C51, sceIoRemoveHidePatched, 0);
    sctrlHENHookImportByNID(mod, "IoFileMgrForUser", 0x1117C65F, sceIoRmdirHidePatched, 0);
    sctrlHENHookImportByNID(mod, "IoFileMgrForUser", 0x109F50BC, sceIoOpenHidePatched, 0);
    sctrlHENHookImportByNID(mod, "IoFileMgrForUser", 0xB29DDF9C, sceIoDopenHidePatched, 0);

	sctrlFlushCache();
}