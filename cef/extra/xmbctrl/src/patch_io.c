#include <pspkerneltypes.h>
#include <pspiofilemgr.h>

#include <systemctrl.h>
#include <adrenaline_log.h>
#include <cfwmacros.h>

#include "xmbctrl.h"

////////////////////////////////////////////////////////////////////////////////
// HELPERS
////////////////////////////////////////////////////////////////////////////////

static int is_ef_path(const char *path) {
	if (paf_strncmp(path, "ef0:", 4) == 0 || paf_strncmp(path, "EF0:", 4) == 0) {
		return 1;
	}
	return 0;
}

static int is_ms_path(const char *path) {
	if (paf_strncmp(path, "ms0:", 4) == 0 || paf_strncmp(path, "MS0:", 4) == 0) {
		return 1;
	}
	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// PATCHED IMPLEMENTATIONS
////////////////////////////////////////////////////////////////////////////////

SceUID sceIoDopenPatched(char *path) {
	if (!g_is_ef_drive && is_ef_path(path)) {
		path[0] = 'm';
		path[1] = 's';
	} else if (g_is_ef_drive && is_ms_path(path)) {
		path[0] = 'e';
		path[1] = 'f';
	}

	SceUID fd = sceIoDopen(path);

	logmsg3("[DEBUG]: %s: path=%s -> 0x%08X\n", __func__, path, fd);
	return fd;
}

SceUID sceIoOpenPatched(char *file, int flags, SceMode mode) {
	if (!g_is_ef_drive && is_ef_path(file)) {
		file[0] = 'm';
		file[1] = 's';
	} else if (g_is_ef_drive && is_ms_path(file)) {
		file[0] = 'e';
		file[1] = 'f';
	}

	SceUID fd = sceIoOpen(file, flags, mode);

	logmsg3("[DEBUG]: %s: file=%s -> 0x%08X\n", __func__, file, fd);
	return fd;
}

int sceIoGetstatPatched(char *file, SceIoStat *stat) {
	if (!g_is_ef_drive && is_ef_path(file)) {
		file[0] = 'm';
		file[1] = 's';
	} else if (g_is_ef_drive && is_ms_path(file)) {
		file[0] = 'e';
		file[1] = 'f';
	}

	int res = sceIoGetstat(file, stat);

	logmsg3("[DEBUG]: %s: file=%s -> 0x%08X\n", __func__, file, res);
	return res;
}

int sceIoChstatPatched(char *file, SceIoStat *stat, int bits) {
	if (!g_is_ef_drive && is_ef_path(file)) {
		file[0] = 'm';
		file[1] = 's';
	} else if (g_is_ef_drive && is_ms_path(file)) {
		file[0] = 'e';
		file[1] = 'f';
	}

	int res = sceIoChstat(file, stat, bits);

	logmsg3("[DEBUG]: %s: file=%s -> 0x%08X\n", __func__, file, res);
	return res;
}

int sceIoRemovePatched(char *file) {
	if (!g_is_ef_drive && is_ef_path(file)) {
		file[0] = 'm';
		file[1] = 's';
	} else if (g_is_ef_drive && is_ms_path(file)) {
		file[0] = 'e';
		file[1] = 'f';
	}

	int res = sceIoRemove(file);

	logmsg3("[DEBUG]: %s: file=%s -> 0x%08X\n", __func__, file, res);
	return res;
}

int sceIoRmdirPatched(char *path) {
	if (!g_is_ef_drive && is_ef_path(path)) {
		path[0] = 'm';
		path[1] = 's';
	} else if (g_is_ef_drive && is_ms_path(path)) {
		path[0] = 'e';
		path[1] = 'f';
	}

	int res = sceIoRmdir(path);
	logmsg3("[DEBUG]: %s: path=%s -> 0x%08X\n", __func__, path, res);
	return res;
}

int sceIoMkdirPatched(char *dir, SceMode mode) {
	if (!g_is_ef_drive && is_ef_path(dir)) {
		dir[0] = 'm';
		dir[1] = 's';
	} else if (g_is_ef_drive && is_ms_path(dir)) {
		dir[0] = 'e';
		dir[1] = 'f';
	}

	int res = sceIoMkdir(dir, mode);
	logmsg3("[DEBUG]: %s: dir=%s -> 0x%08X\n", __func__, dir, res);
	return res;
}

int sceIoDevctlPatched(char *dev, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen) {
	if (!g_is_ef_drive && is_ef_path(dev)) {
		dev[0] = 'm';
		dev[1] = 's';
	} else if (g_is_ef_drive && is_ms_path(dev)) {
		dev[0] = 'e';
		dev[1] = 'f';
	}

	int res = sceIoDevctl(dev, cmd, indata, inlen, outdata, outlen);

	logmsg3("[DEBUG]: %s: dev=%s cmd=0x%08X -> 0x%08X\n", __func__, dev, cmd, res);
	return res;
}

////////////////////////////////////////////////////////////////////////////////
// MODULE PATCHERS
////////////////////////////////////////////////////////////////////////////////

void PatchIo(SceModule *mod) {
	sctrlHookImportByNID(mod, "IoFileMgrForUser", 0xB29DDF9C, sceIoDopenPatched);
	sctrlHookImportByNID(mod, "IoFileMgrForUser", 0x109F50BC, sceIoOpenPatched);
	sctrlHookImportByNID(mod, "IoFileMgrForUser", 0xACE946E8, sceIoGetstatPatched);
	sctrlHookImportByNID(mod, "IoFileMgrForUser", 0xB8A740F4, sceIoChstatPatched);
	sctrlHookImportByNID(mod, "IoFileMgrForUser", 0xF27A9C51, sceIoRemovePatched);
	sctrlHookImportByNID(mod, "IoFileMgrForUser", 0x1117C65F, sceIoRmdirPatched);
	sctrlHookImportByNID(mod, "IoFileMgrForUser", 0x06A70004, sceIoMkdirPatched);
	sctrlHookImportByNID(mod, "IoFileMgrForUser", 0x54F5FB11, sceIoDevctlPatched);
}