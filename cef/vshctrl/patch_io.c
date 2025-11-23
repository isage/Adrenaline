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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pspsdk.h>
#include <psperror.h>
#include <pspiofilemgr.h>

#include <cfwmacros.h>
#include <systemctrl.h>

#include <adrenaline_log.h>

#include "externs.h"
#include "patch_io.h"
#include "virtualpbpmgr.h"

#define DUMMY_CAT_ISO_EXTENSION "     "
#define MAX_FILES 128


static char g_categorypath[256];
static SceUID g_categorydfd = -1;

static SceUID g_gamedfd = -1;
static SceUID g_isodfd = -1;
static SceUID g_overiso = 0;

static int g_vpbpinited = 0;
static int g_isoindex = 0;
static int g_cachechanged = 0;
static VirtualPbp *g_cache = NULL;

////////////////////////////////////////////////////////////////////////////////
// HELPERS
////////////////////////////////////////////////////////////////////////////////

static int CorruptIconPatch(char *name) {
	// Hide ARK bubble launchers
	if (strcasecmp(name, "SCPS10084") == 0 || strcasecmp(name, "NPUZ01234") == 0){
		strcpy(name, "__SCE"); // hide icon
		return 1;
	}

	char path[256];
	sprintf(path, "ms0:/PSP/GAME/%s%%/EBOOT.PBP", name);

	SceIoStat stat;
	memset(&stat, 0, sizeof(stat));
	if (sceIoGetstat(path, &stat) >= 0) {
		strcpy(name, "__SCE"); // hide icon
		return 1;
	}

	return 0;
}

static int HideDlc(char *name) {
	char path[256];
	sprintf(path, "ms0:/PSP/GAME/%s/PARAM.PBP", name);

	SceIoStat stat;
	memset(&stat, 0, sizeof(stat));
	if (sceIoGetstat(path, &stat) >= 0) {
		sprintf(path, "ms0:/PSP/GAME/%s/EBOOT.PBP", name);

		memset(&stat, 0, sizeof(stat));
		if (sceIoGetstat(path, &stat) < 0) {
			strcpy(name, "__SCE"); // hide icon
			return 1;
		}
	}

	return 0;
}

static void ApplyIsoNamePatch(SceIoDirent *dir) {
	if (dir->d_name[0] != '.') {
		memset(dir->d_name, 0, 256);
		sprintf(dir->d_name, "MMMMMISO%d", g_isoindex++);
	}
}

static int ReadCache() {
	SceUID fd;
	int i;

	if (!g_cache) {
		g_cache = (VirtualPbp *)oe_malloc(MAX_FILES * sizeof(VirtualPbp));
	}

	memset(g_cache, 0, sizeof(VirtualPbp) * MAX_FILES);

	for (i = 0; i < 0x10; i++) {
		fd = sceIoOpen("ms0:/PSP/SYSTEM/isocache2.bin", PSP_O_RDONLY, 0);
		if (fd >= 0) {
			break;
		}
	}

	if (i == 0x10) {
		return -1;
	}

	sceIoRead(fd, g_cache, sizeof(VirtualPbp) * MAX_FILES);
	sceIoClose(fd);

	return 0;
}

static int SaveCache() {
	SceUID fd;
	int i;

	if (!g_cache) {
		return -1;
	}

	for (i = 0; i < MAX_FILES; i++) {
		if (g_cache[i].isofile[0] != 0) {
			SceIoStat stat;
			memset(&stat, 0, sizeof(stat));
			if (sceIoGetstat(g_cache[i].isofile, &stat) < 0) {
				g_cachechanged = 1;
				memset(&g_cache[i], 0, sizeof(VirtualPbp));
			}
		}
	}

	if (!g_cachechanged) {
		return 0;
	}

	g_cachechanged = 0;

	sceIoMkdir("ms0:/PSP", 0777);
	sceIoMkdir("ms0:/PSP/SYSTEM", 0777);

	for (i = 0; i < 0x10; i++) {
		fd = sceIoOpen("ms0:/PSP/SYSTEM/isocache2.bin", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
		if (fd >= 0) {
			break;
		}
	}

	if (i == 0x10) {
		return -1;
	}

	sceIoWrite(fd, g_cache, sizeof(VirtualPbp) * MAX_FILES);
	sceIoClose(fd);

	return 0;
}

static int IsCached(char *isofile, ScePspDateTime *mtime, VirtualPbp *res) {
	for (int i = 0; i < MAX_FILES; i++) {
		if (g_cache[i].isofile[0] != 0) {
			if (strcmp(g_cache[i].isofile, isofile) == 0) {
				if (memcmp(mtime, &g_cache[i].mtime, sizeof(ScePspDateTime)) == 0) {
					memcpy(res, &g_cache[i], sizeof(VirtualPbp));
					return 1;
				}
			}
		}
	}

	return 0;
}

static int Cache(VirtualPbp *pbp) {
	for (int i = 0; i < MAX_FILES; i++) {
		if (g_cache[i].isofile[0] == 0) {
			memcpy(&g_cache[i], pbp, sizeof(VirtualPbp));
			g_cachechanged = 1;
			return 1;
		}
	}

	return 0;
}

static int AddIsoDirent(char *path, SceUID fd, SceIoDirent *dir, int readcategories) {
	int res;

NEXT:
	if ((res = sceIoDread(fd, dir)) > 0) {
		static VirtualPbp vpbp;
		static char fullpath[256];
		int res2 = -1;
		int docache;

		if (!FIO_S_ISDIR(dir->d_stat.st_mode)) {
			strcpy(fullpath, path);
			strcat(fullpath, "/");
			strcat(fullpath, dir->d_name);

			if (IsCached(fullpath, &dir->d_stat.sce_st_mtime, &vpbp)) {
				res2 = virtualpbp_fastadd(&vpbp);
				docache = 0;
			} else {
				res2 = virtualpbp_add(fullpath, &dir->d_stat.sce_st_mtime, &vpbp);
				docache = 1;
			}

			if (res2 >= 0) {
				ApplyIsoNamePatch(dir);

				// Fake the entry from file to directory
				dir->d_stat.st_mode = 0x11FF;
				dir->d_stat.st_attr = 0x0010;
				dir->d_stat.st_size = 0;

				// Change the modifcation time to creation time
				memcpy(&dir->d_stat.sce_st_mtime, &dir->d_stat.sce_st_ctime, sizeof(ScePspDateTime));

				if (docache) {
					Cache(&vpbp);
				}
			}
		} else {
			if (readcategories && dir->d_name[0] != '.' && strcmp(dir->d_name, "VIDEO") != 0) {
				strcat(dir->d_name, DUMMY_CAT_ISO_EXTENSION);
			} else {
				goto NEXT;
			}
		}

		return res;
	}

	return -1;
}

int GetIsoIndex(const char *file) {
	char *p = strstr(file, "/MMMMMISO");
	if (!p) {
		return -1;
	}

	char *q = strchr(p + 9, '/');
	if (!q) {
		return strtol(p + 9, NULL, 10);
	}

	char number[5];
	memset(number, 0, 5);
	strncpy(number, p + 9, q - (p + 9));

	return strtol(number, NULL, 10);
}

////////////////////////////////////////////////////////////////////////////////
// PATCHED IMPLEMENTATIONS
////////////////////////////////////////////////////////////////////////////////

SceUID sceIoDopenPatched(const char *dirname) {
	int res, game = 0;
	int k1 = pspSdkSetK1(0);

	int index = GetIsoIndex(dirname);
	if (index >= 0) {
		int res = virtualpbp_open(index);
		pspSdkSetK1(k1);
		return res;
	}

	if (strcmp(dirname, "ms0:/PSP/GAME") == 0) {
		game = 1;
	}

	if (strstr(dirname, DUMMY_CAT_ISO_EXTENSION)) {
		char *p = strrchr(dirname, '/');
		if (p) {
			strcpy(g_categorypath, "ms0:/ISO");
			strcat(g_categorypath, p);
			g_categorypath[8 + strlen(p) - 5] = '\0';

			g_categorydfd = sceIoDopen(g_categorypath);
			pspSdkSetK1(k1);
			return g_categorydfd;
		}
	}

	pspSdkSetK1(k1);
	res = sceIoDopen(dirname);
	pspSdkSetK1(0);

	if (game) {
		g_gamedfd = res;
		g_overiso = 0;
	}

	pspSdkSetK1(k1);
	return res;
}

int sceIoDreadPatched(SceUID fd, SceIoDirent *dir) {
	int res;
	int k1 = pspSdkSetK1(0);

	if (g_vpbpinited) {
		res = virtualpbp_dread(fd, dir);
		if (res >= 0) {
			pspSdkSetK1(k1);
			return res;
		}
	}

	if (fd >= 0) {
		if (fd == g_gamedfd) {
			if (g_isodfd < 0 && !g_overiso) {
				g_isodfd = sceIoDopen("ms0:/ISO");
				if (g_isodfd >= 0) {
					if (!g_vpbpinited) {
						virtualpbp_init();
						g_vpbpinited = 1;
					} else {
						virtualpbp_reset();
					}

					ReadCache();
					g_isoindex = 0;
				} else {
					g_overiso = 1;
				}
			}

			if (g_isodfd >= 0) {
				res = AddIsoDirent("ms0:/ISO", g_isodfd, dir, 1);
				if (res > 0) {
					pspSdkSetK1(k1);
					return res;
				}
			}
		} else if (fd == g_categorydfd) {
			res = AddIsoDirent(g_categorypath, g_categorydfd, dir, 0);
			if (res > 0) {
				pspSdkSetK1(k1);
				return res;
			}
		}
	}

	res = sceIoDread(fd, dir);

	if (res > 0) {
		if (g_cfw_config.hide_corrupt) {
			CorruptIconPatch(dir->d_name);
		}

		if (g_cfw_config.hide_dlcs) {
			HideDlc(dir->d_name);
		}
	}

	pspSdkSetK1(k1);
	return res;
}

int sceIoDclosePatched(SceUID fd) {
	int k1 = pspSdkSetK1(0);
	int res;

	if (g_vpbpinited) {
		res = virtualpbp_close(fd);
		if (res >= 0) {
			pspSdkSetK1(k1);
			return res;
		}
	}

	if (fd == g_categorydfd) {
		g_categorydfd = -1;
	} else if (fd == g_gamedfd) {
		sceIoDclose(g_isodfd);
		g_isodfd = -1;
		g_gamedfd = -1;
		g_overiso = 0;
		SaveCache();
	}

	res = sceIoDclose(fd);
	pspSdkSetK1(k1);
	return res;
}

SceUID sceIoOpenPatched(const char *file, int flags, SceMode mode) {
	int k1 = pspSdkSetK1(0);

	int res = SCE_ENOENT;
	int index = GetIsoIndex(file);
	if (index >= 0) {
		if (strcmp(strrchr(file,'/')+1, "DOCUMENT.DAT") == 0) {
			char path[97];
			sprintf(path, "%s", virtualpbp_getfilename(index));
			((char*)path)[strlen(path)-3] = 'D';
			((char*)path)[strlen(path)-2] = 'A';
			((char*)path)[strlen(path)-1] = 'T';
			res = sceIoOpen(path, flags, mode);
			pspSdkSetK1(k1);
			return res;
		}

		if (strcmp(strrchr(file,'/')+1, "PARAM.PBP") ==0 || strcmp(strrchr(file,'/')+1, "PBOOT.PBP") == 0) {
			char path[255];
			strcpy(path, file);
			virtualpbp_fixisopath(index, path);
			res = sceIoOpen(path, flags, mode);
			pspSdkSetK1(k1);
			return res;
		}

		int res = virtualpbp_open(index);
		pspSdkSetK1(k1);
		return res;
	}

	pspSdkSetK1(k1);
	res = sceIoOpen(file, flags, mode);
	return res;
}

int sceIoClosePatched(SceUID fd) {
	int k1 = pspSdkSetK1(0);
	int res = -1;

	if (g_vpbpinited) {
		res = virtualpbp_close(fd);
	}


	if (res < 0) {
		return sceIoClose(fd);
	}

	pspSdkSetK1(k1);
	return res;
}

int sceIoReadPatched(SceUID fd, void *data, SceSize size) {
	int k1 = pspSdkSetK1(0);
	int res = -1;

	if (g_vpbpinited) {
		res = virtualpbp_read(fd, data, size);
	}


	if (res < 0) {
		return sceIoRead(fd, data, size);
	}

	pspSdkSetK1(k1);
	return res;
}

SceOff sceIoLseekPatched(SceUID fd, SceOff offset, int whence) {
	int k1 = pspSdkSetK1(0);
	int res = -1;

	if (g_vpbpinited) {
		res = virtualpbp_lseek(fd, offset, whence);
	}


	if (res < 0) {
		return sceIoLseek(fd, offset, whence);
	}

	pspSdkSetK1(k1);
	return res;
}

int sceIoLseek32Patched(SceUID fd, int offset, int whence) {
	int k1 = pspSdkSetK1(0);
	int res = -1;

	if (g_vpbpinited) {
		res = virtualpbp_lseek(fd, offset, whence);
	}


	if (res < 0) {
		return sceIoLseek32(fd, offset, whence);
	}

	pspSdkSetK1(k1);
	return res;
}

int sceIoGetstatPatched(const char *file, SceIoStat *stat) {
	int k1 = pspSdkSetK1(0);
	int index = GetIsoIndex(file);
	if (index >= 0) {
		if (strcmp(strrchr(file,'/')+1, "DOCUMENT.DAT") == 0) {
			char path[97];
			sprintf(path, "%s", virtualpbp_getfilename(index));
			((char*)path)[strlen(path)-3] = 'D';
			((char*)path)[strlen(path)-2] = 'A';
			((char*)path)[strlen(path)-1] = 'T';
			pspSdkSetK1(k1);
			return sceIoGetstat(path, stat);
		}

		if (strcmp(strrchr(file,'/')+1, "DOCINFO.EDAT") == 0) {
			pspSdkSetK1(k1);
			return SCE_ENOENT;
		}

		if (strcmp(strrchr(file,'/')+1, "PARAM.PBP") == 0 || strcmp(strrchr(file,'/')+1, "PBOOT.PBP") == 0 ) {
			char path[255];
			strcpy(path, file);
			virtualpbp_fixisopath(index, path);
			int res = sceIoGetstat(path, stat);
			logmsg("%s: file=%s -> 0x%08X\n", __func__, path, res);
			pspSdkSetK1(k1);
			return res;
		}

		int res = virtualpbp_getstat(index, stat);
		pspSdkSetK1(k1);
		return res;
	}

	int game = 0;
	if (strcmp(file, "ms0:/PSP/GAME") == 0) {
		game = 1;
	}


	int res = sceIoGetstat(file, stat);

	if (game && res < 0) {
		sceIoMkdir("ms0:/PSP", 0777);
		sceIoMkdir("ms0:/PSP/GAME", 0777);

		res = sceIoGetstat(file, stat);
	}

	pspSdkSetK1(k1);
	return res;
}

int sceIoChstatPatched(const char *file, SceIoStat *stat, int bits) {
	int k1 = pspSdkSetK1(0);

	int res = SCE_EINVAL;
	int index = GetIsoIndex(file);
	if (index >= 0) {
		res = virtualpbp_chstat(index, stat, bits);
		pspSdkSetK1(k1);
		return res;
	}

	res = sceIoChstat(file, stat, bits);
	pspSdkSetK1(k1);
	return res;
}

int sceIoRemovePatched(const char *file) {
	int k1 = pspSdkSetK1(0);

	int res;
	int index = GetIsoIndex(file);
	if (index >= 0) {
		res = virtualpbp_remove(index);
		pspSdkSetK1(k1);
		return res;
	}

	res = sceIoRemove(file);
	pspSdkSetK1(k1);
	return res;
}

int sceIoRmdirPatched(const char *path) {
	int k1 = pspSdkSetK1(0);

	int res;
	int index = GetIsoIndex(path);
	if (index >= 0) {
		res = virtualpbp_rmdir(index);
		pspSdkSetK1(k1);
		return res;
	}

	res = sceIoRmdir(path);
	pspSdkSetK1(k1);
	return res;
}

int sceIoMkdirPatched(const char *dir, SceMode mode) {
	int k1 = pspSdkSetK1(0);

	if (strcmp(dir, "ms0:/PSP/GAME") == 0) {
		sceIoMkdir("ms0:/ISO", mode);
	}

	int res = sceIoMkdir(dir, mode);
	pspSdkSetK1(k1);
	return res;
}

////////////////////////////////////////////////////////////////////////////////
// MODULE PATCHERS
////////////////////////////////////////////////////////////////////////////////

typedef struct {
	void *import;
	void *patched;
} ImportPatch;

ImportPatch import_patch[] = {
	// Directory functions
	{ &sceIoDopen, sceIoDopenPatched },
	{ &sceIoDread, sceIoDreadPatched },
	{ &sceIoDclose, sceIoDclosePatched },

	// File functions
	{ &sceIoOpen, sceIoOpenPatched },
	{ &sceIoClose, sceIoClosePatched },
	{ &sceIoRead, sceIoReadPatched },
	{ &sceIoLseek, sceIoLseekPatched },
	{ &sceIoLseek32, sceIoLseek32Patched },
	{ &sceIoGetstat, sceIoGetstatPatched },
	{ &sceIoChstat, sceIoChstatPatched },
	{ &sceIoRemove, sceIoRemovePatched },
	{ &sceIoRmdir, sceIoRmdirPatched },
	{ &sceIoMkdir, sceIoMkdirPatched },
};

void IoPatches() {
	for (int i = 0; i < (sizeof(import_patch) / sizeof(ImportPatch)); i++) {
		sctrlHENPatchSyscall(K_EXTRACT_IMPORT(import_patch[i].import), import_patch[i].patched);
	}
}