#include "utils.h"

#include <stddef.h>
#include <string.h>
#include <strings.h>

#include <systemctrl.h>
#include <systemctrl_se.h>
#include <pspctrl.h>
#include <common.h>

#include "dirent_track.h"
#include "virtual_pbp.h"
#include "virtual_mp4.h"
#include "patched_io.h"

#include "printk.h"

#define MAGIC_DFD_FOR_DELETE 0x9000
#define MAGIC_DFD_FOR_DELETE_2 0x9001

// extern SEConfig* config;
extern SceUID gamedfd;

static char g_iso_dir[128];
static char g_temp_delete_dir[128];
static int g_delete_eboot_injected = 0;

extern SEConfig config;
extern u32 psp_model;

static const char *g_cfw_dirs[] = {
	"ms0:/seplugins",
	"ms0:/ISO",
	"ms0:/ISO/VIDEO",
};

static int is_iso_dir(const char *path) {
	const char *p;

	if (path == NULL) return 0;

	p = strchr(path, '/');

	if (p == NULL) return 0;

	if (p <= path + 1 || p[-1] != ':') return 0;

	p = strstr(p, ISO_ID);

	if (NULL == p) return 0;

	p = strrchr(path, '@') + 1;
	p += 8;

	while(*p != '\0' && *p == '/') {
		p++;
	}

	if (*p != '\0') return 0;

	return 1;
}

#define is_iso_eboot(path) is_iso_file(path, "/EBOOT.PBP")
#define is_iso_manual(path) is_iso_file(path, "/DOCUMENT.DAT")
#define is_iso_update(path) is_iso_file(path, "/PBOOT.PBP")
#define is_iso_dlc(path) is_iso_file(path, "/PARAM.PBP")
static int is_iso_file(const char* path, const char* file) {
	if (path == NULL) {
		return 0;
	}

	const char *p = strchr(path, '/');

	if (p == NULL) {
		return 0;
	}

	if (p <= path + 1 || p[-1] != ':') {
		return 0;
	}

	p = strstr(p, ISO_ID);

	if (NULL == p) {
		return 0;
	}

	p = strrchr(path, '@') + 9;

	if (p == NULL || 0 != strcmp(p, file)) {
		return 0;
	}

	return 1;
}

static inline int is_game_dir(const char *dirname) {
	char path[256];
	SceIoStat stat;

	const char *p = strchr(dirname, '/');

	if (p == NULL) {
		return 0;
	}

	if (0 != strncasecmp(p, "/PSP/GAME", sizeof("/PSP/GAME")-1)) {
		return 0;
	}

	if (0 == strncasecmp(p, "/PSP/GAME/_DEL_", sizeof("/PSP/GAME/_DEL_")-1)) {
		return 0;
	}

	static const char* game_files[] = {"/EBOOT.PBP", "/PBOOT.PBP", "/PARAM.PBP"};

	for (int i=0; i<NELEMS(game_files); i++){
		STRCPY_S(path, dirname);
		STRCAT_S(path, game_files[i]);

		if(0 == sceIoGetstat(path, &stat)) {
			return 0;
		}
	}

	return 1;
}

static int get_device_name(char *device, int size, const char* path) {
	const char *p;

	if (path == NULL || device == NULL) {
		return -1;
	}

	p = strchr(path, '/');

	if (p == NULL) {
		return -2;
	}

	strncpy(device, path, MIN(size, p-path+1));
	device[MIN(size-1, p-path)] = '\0';

	return 0;
}

static int CorruptIconPatch(char *name) {
	char path[256];
	sprintf(path, "ms0:/PSP/GAME/%s%%/EBOOT.PBP", name);

	// Hide ARK launchers
	if (strcasecmp(name, "SCPS10084") == 0 || strcasecmp(name, "NPUZ01234") == 0){
		strcpy(name, "__SCE"); // hide icon
		return 1;
	}

	SceIoStat stat;
	memset(&stat, 0, sizeof(stat));
	if (sceIoGetstat(path, &stat) >= 0) {
		strcpy(name, "__SCE"); // hide icon
		return 1;
	}

	return 0;
}

static int HideDlc(char *name) {
	static char* dlc_files[] = {"PARAM.PBP", "PBOOT.PBP", "DOCUMENT.DAT"};

	char path[256];
	SceIoStat stat;

	for (int i=0; i<NELEMS(dlc_files); i++) {
		sprintf(path, "ms0:/PSP/GAME/%s/%s", name, dlc_files[i]);
		memset(&stat, 0, sizeof(stat));
		if (sceIoGetstat(path, &stat) >= 0) {
			sprintf(path, "ms0:/PSP/GAME/%s/EBOOT.PBP", name);

			memset(&stat, 0, sizeof(stat));
			if (sceIoGetstat(path, &stat) < 0) {
				strcpy(name, "__SCE"); // hide icon
				return 1;
			}
		}
	}

	return 0;
}

SceUID sceIoDopenPatched(const char *dirname) {
	SceUID res;

	if (is_iso_dir(dirname)) {
		res = MAGIC_DFD_FOR_DELETE;
		g_delete_eboot_injected = 0;
		strncpy(g_iso_dir, dirname, sizeof(g_iso_dir));
		g_iso_dir[sizeof(g_iso_dir)-1] = '\0';

	} else if (is_video_path(dirname)) {
		res = videoIoDopen(dirname);
		goto exit;
	}

	if (strcmp(dirname, g_temp_delete_dir) == 0) {
		res = MAGIC_DFD_FOR_DELETE_2;
		goto exit;
	}

	res = sceIoDopen(dirname);

	if (is_game_dir(dirname)) {
		char path[256] = {0};
		// get_device_name(path, sizeof(path), dirname);
		STRCAT_S(path, "ms0:/ISO");

		const char *p = strstr(dirname, "/PSP/GAME");

		if (NULL != p) {
			p += sizeof("/PSP/GAME") - 1;
			STRCAT_S(path, p);
		}

		u32 k1 = pspSdkSetK1(0);
		int iso_dfd = vpbp_dopen(path);
		pspSdkSetK1(k1);

		if(iso_dfd < 0) {
			goto exit;
		}

		if(res < 0) {
			res = iso_dfd;
		}

		int ret = dirent_add(res, iso_dfd, dirname);

		if (ret < 0) {
			#if defined(DEBUG) && DEBUG >= 2
			printk("%s: dirent_add -> %d\n", __func__, ret);
			#endif
			return -1;
		}
	}

exit:
	#if defined(DEBUG) && DEBUG >= 2
	printk("%s: %s -> 0x%08X\n", __func__, dirname, res);
	#endif
	return res;
}

int sceIoDreadPatched(SceUID fd, SceIoDirent *dir) {
	int res;
	if (is_video_folder(fd)){
		res = videoIoDread(fd, dir);
		goto exit;
	}

	if (fd == MAGIC_DFD_FOR_DELETE || fd == MAGIC_DFD_FOR_DELETE_2) {
		res = 0;

		if (g_delete_eboot_injected == 0) {
			memset(dir, 0, sizeof(*dir));
			u32 k1 = pspSdkSetK1(0);
			res = vpbp_getstat(g_iso_dir, &dir->d_stat);
			pspSdkSetK1(k1);

			if(fd == MAGIC_DFD_FOR_DELETE) {
				strcpy(dir->d_name, "EBOOT.PBP");
			} else {
				strcpy(dir->d_name, "_EBOOT.PBP");
			}

			g_delete_eboot_injected = 1;
			res = 1;
		}

		goto exit;
	}

	res = sceIoDread(fd, dir);

	if (res <= 0) {
		IoDirentEntry* entry = dirent_search(fd);
		if(entry != NULL) {
			u32 k1 = pspSdkSetK1(0);
			res = vpbp_dread(fd, dir);
			pspSdkSetK1(k1);
		}
	} else {
		int k1 = pspSdkSetK1(0);

		if (config.hidecorrupt) {
			CorruptIconPatch(dir->d_name);
		}

		if (config.hidedlcs) {
			HideDlc(dir->d_name);
		}
		pspSdkSetK1(k1);
	}

exit:
	#if defined(DEBUG) && DEBUG >= 2
	printk("%s: 0x%08X %s -> 0x%08X\n", __func__, fd, dir->d_name, res);
	#endif
	return res;
}

int sceIoDclosePatched(SceUID fd) {
	int res;

	if (is_video_folder(fd)) {
		res = videoIoDclose(fd);
		goto exit;
	}

	if (fd == MAGIC_DFD_FOR_DELETE || fd == MAGIC_DFD_FOR_DELETE_2) {
		g_delete_eboot_injected = 0;
		res = 0;
		goto exit;
	}

	IoDirentEntry* entry = dirent_search(fd);

	if (entry != NULL) {
		if (entry->iso_dfd == fd) {
			u32 k1 = pspSdkSetK1(0);
			vpbp_dclose(fd);
			pspSdkSetK1(k1);

			res = 0;
		} else if (entry->dfd == fd) {
			u32 k1 = pspSdkSetK1(0);
			res = vpbp_dclose(fd);
			pspSdkSetK1(k1);
		} else {
			res = sceIoDclose(fd);
		}

		dirent_remove(entry);
	} else {
		res = sceIoDclose(fd);
	}

exit:
	#if defined(DEBUG) && DEBUG >= 2
	printk("%s: 0x%08X -> 0x%08X\n", __func__, fd, res);
	#endif
	return res;
}

SceUID sceIoOpenPatched(const char *file, int flags, SceMode mode) {
	SceUID res;

	if (is_iso_eboot(file)) {
		u32 k1 = pspSdkSetK1(0);
		res = vpbp_open(file, flags, mode);
		pspSdkSetK1(k1);
	} else if (is_video_path(file)) {
		res = videoIoOpen(file, flags, mode);
	} else {
		if (is_iso_manual(file) || is_iso_update(file) || is_iso_dlc(file)){
			vpbp_fixisopath(file);
		}
		res = sceIoOpen(file, flags, mode);
	}

	#if defined(DEBUG) && DEBUG >= 3
	printk("%s: %s -> 0x%08X\n", __func__, file, res);
	#endif

	return res;
}

int sceIoReadPatched(SceUID fd, void *data, SceSize size) {
	int res;

	if (vpbp_is_fd(fd)) {
		u32 k1 = pspSdkSetK1(0);
		res = vpbp_read(fd, data, size);
		pspSdkSetK1(k1);
	} else if (is_video_file(fd)) {
		res = videoIoRead(fd, data, size);
	} else {
		res = sceIoRead(fd, data, size);
	}

	#if defined(DEBUG) && DEBUG >= 3
	printk("%s: 0x%08X -> 0x%08X\n", __func__, fd, res);
	#endif

	return res;
}

int sceIoClosePatched(SceUID fd) {
	int res;

	if (vpbp_is_fd(fd)) {
		u32 k1 = pspSdkSetK1(0);
		res = vpbp_close(fd);
		pspSdkSetK1(k1);
	} else if (is_video_file(fd)) {
		res = videoIoClose(fd);
	} else {
		res = sceIoClose(fd);
	}

	return res;
}

SceOff sceIoLseekPatched(SceUID fd, SceOff offset, int whence) {
	SceOff res = 0;

	if (vpbp_is_fd(fd)) {
		u32 k1 = pspSdkSetK1(0);
		res = vpbp_lseek(fd, offset, whence);
		pspSdkSetK1(k1);
	} else if (is_video_file(fd)) {
		res = videoIoLseek(fd, offset, whence);
	} else {
		res = sceIoLseek(fd, offset, whence);
	}

	#if defined(DEBUG) && DEBUG >= 3
	printk("%s: 0x%08X -> 0x%08X\n", __func__, fd, res);
	#endif

	return res;
}

int sceIoLseek32Patched(SceUID fd, int offset, int whence) {
	int res = 0;

	if (vpbp_is_fd(fd)) {
		u32 k1 = pspSdkSetK1(0);
		res = (int) vpbp_lseek(fd, offset, whence);
		pspSdkSetK1(k1);
	} else if (is_video_file(fd)) {
		res = (int) videoIoLseek(fd, offset, whence);
	} else {
		res = sceIoLseek32(fd, offset, whence);
	}

	#if defined(DEBUG) && DEBUG >= 3
	printk("%s: 0x%08X -> 0x%08X\n", __func__, fd, res);
	#endif

	return res;
}

int sceIoGetstatPatched(const char *file, SceIoStat *stat) {
	int res;

	if (is_iso_eboot(file)) {
		u32 k1 = pspSdkSetK1(0);
		res = vpbp_getstat(file, stat);
		pspSdkSetK1(k1);
	} else if (is_video_path(file)) {
		res = videoIoGetstat(file, stat);
	} else {
		if (is_iso_manual(file) || is_iso_update(file) || is_iso_dlc(file)){
			vpbp_fixisopath(file);
		}
		res = sceIoGetstat(file, stat);
	}

	#if defined(DEBUG) && DEBUG >= 3
	printk("%s: %s -> 0x%08X\n", __func__, file, res);
	#endif

	return res;
}

int sceIoRemovePatched(const char *file) {
	int res;
	if (is_video_path(file)) {
		res = videoIoRemove(file);
		#if defined(DEBUG) && DEBUG >= 2
		printk("%s:<virtual-mp4> %s -> 0x%08X\n", __func__, file, res);
		#endif
		return res;
	}

	if (g_temp_delete_dir[0] != '\0' && strncmp(file, g_temp_delete_dir, strlen(g_temp_delete_dir)) == 0) {
		res = 0;
		#if defined(DEBUG) && DEBUG >= 2
		printk("%s:<virtual> %s -> 0x%08X\n", __func__, file, res);
		#endif
		return res;
	}

	res = sceIoRemove(file);
	#if defined(DEBUG) && DEBUG >= 2
	printk("%s: %s -> 0x%08X\n", __func__, file, res);
	#endif
	return res;
}

int sceIoRmdirPatched(const char *path) {
	int res;

	if (strcmp(path, g_temp_delete_dir) == 0) {
		strcat(g_iso_dir, "/EBOOT.PBP");

		u32 k1 = pspSdkSetK1(0);
		res = vpbp_remove(g_iso_dir);
		pspSdkSetK1(k1);
		#if defined(DEBUG) && DEBUG >= 2
		printk("%s:<virtual> %s -> 0x%08X\n", __func__, path, res);
		#endif
		g_iso_dir[0] = '\0';
		g_temp_delete_dir[0] = '\0';

		return res;
	}

	res = sceIoRmdir(path);
	#if defined(DEBUG) && DEBUG >= 2
	printk("%s: %s 0x%08X\n", __func__, path, res);
	#endif
	return res;
}

int sceIoMkdirPatched(const char *dir, SceMode mode) {
	int k1 = pspSdkSetK1(0);

	if (strcmp(dir, "ms0:/PSP/GAME") == 0) {
		for (int i = 0; i < NELEMS(g_cfw_dirs); i++) {
			sceIoMkdir(g_cfw_dirs[i], mode);
		}
	}

	pspSdkSetK1(k1);
	return sceIoMkdir(dir, mode);
}

int sceIoRenamePatched(const char *oldname, const char *newname) {
	int res;

	if (is_iso_dir(oldname)) {
		res = 0;
		strncpy(g_iso_dir, oldname, sizeof(g_iso_dir));
		g_iso_dir[sizeof(g_iso_dir)-1] = '\0';
		strncpy(g_temp_delete_dir, newname, sizeof(g_temp_delete_dir));
		#if defined(DEBUG) && DEBUG >= 2
		printk("%s:<virtual> %s %s -> 0x%08X\n", __func__, oldname, newname, res);
		#endif
		return 0;
	}

	if(g_temp_delete_dir[0] != '\0' && strncmp(oldname, g_temp_delete_dir, strlen(g_temp_delete_dir)) == 0) {
		res = 0;
		#if defined(DEBUG) && DEBUG >= 2
		printk("%s:<virtual2> %s %s -> 0x%08X\n", __func__, oldname, newname, res);
		#endif
		return res;
	}

	res = sceIoRename(oldname, newname);
	#if defined(DEBUG) && DEBUG >= 2
	printk("%s: %s %s -> 0x%08X\n", __func__, oldname, newname, res);
	#endif
	return res;
}

int sceIoChstatPatched(const char *file, SceIoStat *stat, int bits) {
	int res;
	if(g_temp_delete_dir[0] != '\0' && strncmp(file, g_temp_delete_dir, strlen(g_temp_delete_dir)) == 0) {
		res = 0;
		#if defined(DEBUG) && DEBUG >= 2
		printk("%s:<virtual> %s -> 0x%08X\n", __func__, file, res);
		#endif
		return res;
	}

	res = sceIoChstat(file, stat, bits);
	#if defined(DEBUG) && DEBUG >= 3
	printk("%s: %s -> 0x%08X\n", __func__, file, res);
	#endif
	return res;
}

int homebrewloadexec(char * file, struct SceKernelLoadExecVSHParam * param) {
	#ifdef DEBUG
	printk("Executing %s\n", __func__);
	#endif
	sctrlSESetBootConfFileIndex(BOOT_NORMAL);
	sctrlSESetUmdFile("");

	// fix 1.50 homebrew
	char *perc = strchr(param->argp, '%');
	if (perc) {
		strcpy(perc, perc + 1);
		file = param->argp;
	}

	return sctrlKernelLoadExecVSHMs2(file, param);
}

int umdemuloadexec(char * file, struct SceKernelLoadExecVSHParam * param) {
	#ifdef DEBUG
	printk("Executing %s\n", __func__);
	#endif

	//result
	int result = -1;

	//virtual iso eboot detected
	if (is_iso_eboot(file)) {
		u32 k1 = pspSdkSetK1(0);
		result = vpbp_loadexec(file, param);
		pspSdkSetK1(k1);
		return result;
	}

	sctrlSESetBootConfFileIndex(BOOT_NORMAL);
	sctrlSESetUmdFile("");

	static int apitypes[2][2] = {
		{PSP_INIT_APITYPE_UMDEMU_MS1, PSP_INIT_APITYPE_UMDEMU_MS2},
		{PSP_INIT_APITYPE_UMDEMU_EF1, PSP_INIT_APITYPE_UMDEMU_EF2}
	};

	int apitype = apitypes
		[ (strncmp(file, "ms", 2) == 0)? 0:1 ]
		[ (strstr(param->argp, "/PBOOT.PBP") == NULL)? 0:1 ];

	//forward
	return sctrlKernelLoadExecVSHWithApitype(apitype, file, param);
}

int umdLoadExec(char * file, struct SceKernelLoadExecVSHParam * param) {
	#ifdef DEBUG
	printk("Executing %s\n", __func__);
	#endif

	//result
	int ret = 0;

	sctrlSESetDiscType(PSP_UMD_TYPE_GAME);

	if(psp_model == PSP_GO) {
		char devicename[20];
		int apitype = PSP_INIT_APITYPE_UMDEMU_MS1;

		extern char mounted_iso[64];
		file = mounted_iso;
		ret = get_device_name(devicename, sizeof(devicename), file);

		if (ret == 0 && 0 == strcasecmp(devicename, "ef0:")) {
			apitype = PSP_INIT_APITYPE_UMDEMU_EF1;
		}

		param->key = "umdemu";
		// Set umdmode
		if (config.umdmode == MODE_INFERNO) {
			sctrlSESetBootConfFileIndex(BOOT_INFERNO);
		} else if (config.umdmode == MODE_MARCH33) {
			sctrlSESetBootConfFileIndex(BOOT_MARCH33);
		} else if (config.umdmode == MODE_NP9660) {
			sctrlSESetBootConfFileIndex(BOOT_NP9660);
		}

		sctrlSESetBootConfFileIndex(BOOT_INFERNO);

		ret = sctrlKernelLoadExecVSHWithApitype(apitype, file, param);
	} else {
		sctrlSESetBootConfFileIndex(BOOT_NORMAL);
		sctrlSESetUmdFile("");
		int apitype = (strstr(param->argp, "/PBOOT.PBP")==NULL)? PSP_INIT_APITYPE_DISC:PSP_INIT_APITYPE_DISC_PBOOT;
		ret = sctrlKernelLoadExecVSHWithApitype(apitype, file, param);
	}


	return ret;
}

int umdLoadExecUpdater(char * file, struct SceKernelLoadExecVSHParam * param) {
	#ifdef DEBUG
	printk("Executing %s\n", __func__);
	#endif

	//result
	int ret = 0;
	sctrlSESetBootConfFileIndex(BOOT_UPDATERUMD);
	sctrlSESetDiscType(PSP_UMD_TYPE_GAME);
	ret = sceKernelLoadExecVSHDiscUpdater(file, param);
	return ret;
}

int homebrewLoadExec(int apitype, char *file, struct SceKernelLoadExecVSHParam *param, int unk2) {
	#ifdef DEBUG
	printk("Executing %s(0x%04X, %s)\n", __func__, apitype, file);
	#endif

	sctrlSESetBootConfFileIndex(BOOT_NORMAL);
	sctrlSESetUmdFile("");

	// fix 1.50 homebrew
	char *perc = strchr(param->argp, '%');
	if (perc) {
		strcpy(perc, perc + 1);
		file = param->argp;
	}

	return sctrlKernelLoadExecVSHMs2(file, param);
}

int umdemuLoadExec(int apitype, char *file, struct SceKernelLoadExecVSHParam *param, int unk2) {
	#ifdef DEBUG
	printk("Executing %s(0x%04X, %s)\n", __func__, apitype, file);
	#endif

	//result
	int result = -1;

	//virtual iso eboot detected
	if (is_iso_eboot(file)) {
		u32 k1 = pspSdkSetK1(0);
		result = vpbp_loadexec(file, param);
		pspSdkSetK1(k1);
		return result;
	}

	sctrlSESetBootConfFileIndex(BOOT_NORMAL);
	sctrlSESetUmdFile("");

	static int apitypes[2][2] = {
		{PSP_INIT_APITYPE_UMDEMU_MS1, PSP_INIT_APITYPE_UMDEMU_MS2},
		{PSP_INIT_APITYPE_UMDEMU_EF1, PSP_INIT_APITYPE_UMDEMU_EF2}
	};

	apitype = apitypes
		[ (strncmp(file, "ms", 2) == 0)? 0:1 ]
		[ (strstr(param->argp, "/PBOOT.PBP") == NULL)? 0:1 ];

	#ifdef DEBUG
	printk("Modified %s args: Executing %s(0x%04X, %s)\n", __func__, __func__, apitype, file);
	#endif
	//forward
	return sctrlKernelLoadExecVSHWithApitype(apitype, file, param);
}

int umdLoadExec2(int apitype, char *file, struct SceKernelLoadExecVSHParam *param, int unk2) {
	#ifdef DEBUG
	printk("Executing %s(0x%04X, %s)\n", __func__, apitype, file);
	#endif

	//result
	int ret = 0;

	sctrlSESetDiscType(PSP_UMD_TYPE_GAME);

	if(psp_model == PSP_GO) {
		char devicename[20];
		apitype = PSP_INIT_APITYPE_UMDEMU_MS1;

		extern char mounted_iso[64];
		file = mounted_iso;
		ret = get_device_name(devicename, sizeof(devicename), file);

		if (ret == 0 && 0 == strcasecmp(devicename, "ef0:")) {
			apitype = PSP_INIT_APITYPE_UMDEMU_EF1;
		}

		param->key = "umdemu";
		// Set umdmode
		if (config.umdmode == MODE_INFERNO) {
			sctrlSESetBootConfFileIndex(BOOT_INFERNO);
		} else if (config.umdmode == MODE_MARCH33) {
			sctrlSESetBootConfFileIndex(BOOT_MARCH33);
		} else if (config.umdmode == MODE_NP9660) {
			sctrlSESetBootConfFileIndex(BOOT_NP9660);
		}

		sctrlSESetBootConfFileIndex(BOOT_INFERNO);

		#ifdef DEBUG
		printk("Modified %s args: Executing %s(0x%04X, %s)\n", __func__, __func__, apitype, file);
		#endif

		ret = sctrlKernelLoadExecVSHWithApitype(apitype, file, param);
	} else {
		sctrlSESetBootConfFileIndex(BOOT_NORMAL);
		sctrlSESetUmdFile("");
		apitype = (strstr(param->argp, "/PBOOT.PBP")==NULL)? PSP_INIT_APITYPE_DISC:PSP_INIT_APITYPE_DISC_PBOOT;

		#ifdef DEBUG
		printk("Modified %s args: Executing %s(0x%04X, %s)\n", __func__, __func__, apitype, file);
		#endif
		ret = sctrlKernelLoadExecVSHWithApitype(apitype, file, param);
	}


	return ret;
}

int LoadExecVSHCommonPatched(int apitype, char *file, struct SceKernelLoadExecVSHParam *param, int unk2) {
	#ifdef DEBUG
	printk("Executing %s(0x%04X, %s)\n", __func__, apitype, file);
	#endif

	// ISO
	if (is_iso_eboot(file) || is_iso_manual(file) || is_iso_dlc(file) || is_iso_update(file)) {
		u32 k1 = pspSdkSetK1(0);
		int result = vpbp_loadexec(file, param);
		pspSdkSetK1(k1);
		return result;
	}

	int k1 = pspSdkSetK1(0);

	sctrlSESetUmdFile("");

	// Enable 1.50 homebrews boot
	char *perc = strchr(param->argp, '%');
	if (perc) {
		strcpy(perc, perc + 1);
		file = param->argp;
		param->args = strlen(param->argp) + 1; //Update length
	}

	pspSdkSetK1(k1);

	return sctrlKernelLoadExecVSHWithApitype(apitype, file, param);
}

int LoadExecVSHCommonPatched2(int apitype, char *file, struct SceKernelLoadExecVSHParam *param, int unk2) {
	#ifdef DEBUG
	printk("Executing %s(0x%04X, %s)\n", __func__, apitype, file);
	#endif

	// ISO
	if (is_iso_eboot(file) || is_iso_manual(file) || is_iso_dlc(file) || is_iso_update(file)) {
		u32 k1 = pspSdkSetK1(0);
		int result = vpbp_loadexec(file, param);
		pspSdkSetK1(k1);
		return result;
	}

	int k1 = pspSdkSetK1(0);

	sctrlSESetUmdFile("");

	// Enable 1.50 homebrews boot
	char *perc = strchr(param->argp, '%');
	if (perc) {
		strcpy(perc, perc + 1);
		file = param->argp;
		param->args = strlen(param->argp) + 1; //Update length
	}

	pspSdkSetK1(k1);

	return sctrlKernelLoadExecVSHWithApitype(apitype, file, param);
}

int LoadExecVSHCommonPatched3(int apitype, char *file, struct SceKernelLoadExecVSHParam *param, int unk2) {
	#ifdef DEBUG
	printk("Executing %s(0x%04X, %s)\n", __func__, apitype, file);
	#endif

	// ISO
	if (is_iso_eboot(file) || is_iso_manual(file) || is_iso_dlc(file) || is_iso_update(file)) {
		u32 k1 = pspSdkSetK1(0);
		int result = vpbp_loadexec(file, param);
		pspSdkSetK1(k1);
		return result;
	}

	int k1 = pspSdkSetK1(0);

	sctrlSESetUmdFile("");

	// Enable 1.50 homebrews boot
	char *perc = strchr(param->argp, '%');
	if (perc) {
		strcpy(perc, perc + 1);
		file = param->argp;
		param->args = strlen(param->argp) + 1; //Update length
	}

	pspSdkSetK1(k1);

	return sctrlKernelLoadExecVSHWithApitype(apitype, file, param);
}