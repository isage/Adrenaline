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

#include <stdio.h>
#include <string.h>

#include <pspsdk.h>
#include <pspkernel.h>
#include <pspinit.h>
#include <pspthreadman_kernel.h>
#include <pspsysmem_kernel.h>
#include <pspiofilemgr.h>
#include <pspiofilemgr_kernel.h>

#include <cfwmacros.h>
#include <psperror.h>
#include <systemctrl_adrenaline.h>

#include <adrenaline_log.h>

#include "externs.h"
#include "adrenaline.h"


static PspIoDrv *g_ms_drv;
static PspIoDrv *g_flashfat_drv;
static PspIoDrv g_ef_drv;
static PspIoDrv g_fatef_drv;

static PspIoDrvFuncs g_ms_funcs;
static PspIoDrvFuncs g_ef_funcs;
static PspIoDrvFuncs g_flash_funcs;

static int (* _sceIoAddDrv)(PspIoDrv *drv);
static int (* _sceIoDelDrv)(const char *drv_name);
static int (* _sceIoUnassign)(const char *dev);
static int (* _sceIoAssign)(const char *dev1, const char *dev2, const char *dev3, int mode, void* unk1, long unk2);

static char mytolower(const char in) {
	if(in >=65 && in<=90)
		return in + 32;
	return in;
}

static char *stristr(const char *str1, const char *str2) {
	#define MAXLEN 256

	static char temp1[MAXLEN+1], temp2[MAXLEN+1];
	temp1[MAXLEN] = 0;
	temp2[MAXLEN] = 0;

	strncpy(temp1, str1, MAXLEN);
	strncpy(temp2, str2, MAXLEN);

	for (int i = 0; i < MAXLEN && (temp1[i] != 0); i++) {
		temp1[i] = mytolower((int)temp1[i]);
	}

	for (int i = 0; i < MAXLEN && (temp2[i] != 0); i++) {
		temp2[i] = mytolower((int)temp2[i]);
	}

	const char *pos = strstr(temp1, temp2);
	if (pos) {
		return (char *)(pos - temp1 + str1);
	}

	return 0;
}

static int NoDevicePatch(u32 cmd) {
	if ((cmd & 0x00FFFF00) == 0x00208000) {
		// Returning error allows pops decryption?
		return ((cmd & 0xFF) == 0x82) ? -1 : 0;
	}

	return 1;
}

static int _msIoOpen(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	char *file = (char *)args[1];
	int flags = args[2];
	SceMode mode = (SceMode)args[3];

	int res = g_ms_funcs.IoOpen(arg, file, flags, mode);

	// Fixes Tekken and Soul Calibur
	if (stristr(file, "PARAM.SFO")) {
		((u32 *)arg)[18] = -1;
	}

	return res;
}

static int _msIoIoctl(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	u32 cmd = (u32)args[1];
	void *indata = (void *)args[2];
	int inlen = args[3];
	void *outdata = (void *)args[4];
	int outlen = args[5];

	if (((u32 *)arg)[18] != -1) {
		int res = NoDevicePatch(cmd);
		if (res <= 0) {
			return res;
		}
	}

	return g_ms_funcs.IoIoctl(arg, cmd, indata, inlen, outdata, outlen);
}

static int IoDevctlReinsertMs() {
	SceModule *mod = sceKernelFindModuleByName("sceKermitMsfs_driver");
	if (!mod) {
		return -1;
	}

	int (* MsfsSysEventHandler)(int ev_id, char *ev_name, void *param, int *result) = (void *)mod->text_addr + 0x150;

	// Perform a MS reinsertion
	static int ev_ids[] = { 0x102, 0x400, 0x10000, 0x100000 };

	for (int i = 0; i < (sizeof(ev_ids) / sizeof(int)); i++) {
		MsfsSysEventHandler(ev_ids[i], NULL, NULL, NULL);
	}

	return 0;
}

static int _msIoDevctl(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *devname = (const char *)args[1];
	u32 cmd = (u32)args[2];
	void *indata = (void *)args[3];
	int inlen = args[4];
	void *outdata = (void *)args[5];
	int outlen = args[6];

	if (cmd == 0x02415830) {
		char *oldname = (char *)VREAD32((u32)indata);
		char *newname = (char *)VREAD32((u32)indata + 4);
		return g_ms_funcs.IoRename(arg, oldname, newname);
	} else if (cmd == 0x0240D81E) {
		return IoDevctlReinsertMs();
	} else if (cmd == 0x02425856) {
		// Character code set
		return 0;
	}

	int res = g_ms_funcs.IoDevctl(arg, devname, cmd, indata, inlen, outdata, outlen);

	if (cmd == 0x02425818) {
		// Fix integer overflow in Outrun
		if (sceKernelBootFrom() == PSP_BOOT_DISC) {
			u32 data = *(u32 *)indata;
			if (data) {
				SceDevInf *info = (SceDevInf *)data;

				u64 size = (u64)info->freeClusters * (u64)info->sectorSize * (u64)info->sectorCount;
				if (size >= 0x7FFFFFFF) {
					u32 sectorSize = info->sectorSize;
					u32 memStickSectorSize = 32 * 1024;
					u32 sectorCount = memStickSectorSize / sectorSize;
					u32 freeSize = 1900 * 1024 * 1024; // pretend to have 1.8GB, enough for any game
					u32 clusterSize = sectorSize * sectorCount;
					if (clusterSize) {
						u32 maxClusters = (u32)(freeSize / clusterSize);
						if (info->freeClusters > maxClusters) {
							info->maxClusters = maxClusters;
							info->freeClusters = maxClusters;
							info->maxSectors = maxClusters;
							info->sectorCount = sectorCount;
							logmsg4("[INFO]: %s: Faked free size to 1.8GB\n", __func__);
						}
					}
				}
			}
		}
	}

	return res;
}

static int msIoOpen(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode) {
	u32 args[4];
	args[0] = (u32)arg;
	args[1] = (u32)file;
	args[2] = (u32)flags;
	args[3] = (u32)mode;

	return sceKernelExtendKernelStack(0x4000, (void *)_msIoOpen, args);
}

static int msIoIoctl(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen) {
	u32 args[6];
	args[0] = (u32)arg;
	args[1] = (u32)cmd;
	args[2] = (u32)indata;
	args[3] = (u32)inlen;
	args[4] = (u32)outdata;
	args[5] = (u32)outlen;

	return sceKernelExtendKernelStack(0x4000, (void *)_msIoIoctl, args);
}

static int msIoDevctl(PspIoDrvFileArg *arg, const char *devname, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen) {
	u32 args[7];
	args[0] = (u32)arg;
	args[1] = (u32)devname;
	args[2] = (u32)cmd;
	args[3] = (u32)indata;
	args[4] = (u32)inlen;
	args[5] = (u32)outdata;
	args[6] = (u32)outlen;

	return sceKernelExtendKernelStack(0x4000, (void *)_msIoDevctl, args);
}

__attribute__((noinline)) static int BuildMsPathForEf(PspIoDrvFileArg *arg, const char *name, char *ms_path) {

	// Already __ef0__
	if (strncmp(name, "__ef0__", 7) == 0 || strncmp(name, "/__ef0__", 8) == 0) {
		sprintf(ms_path, "%s", name);
	} else {
		sprintf(ms_path, "/__ef%d__%s", (int)arg->fs_num, name);
	}

	int fs_num = arg->fs_num;
	arg->fs_num = 0;
	return fs_num;
}

static char ms_path[256];
static int _efIoOpen(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	char *file = (char *)args[1];
	int flags = args[2];
	SceMode mode = (SceMode)args[3];

	memset(ms_path, 0, sizeof(ms_path));
	BuildMsPathForEf(arg, file, ms_path);
	int res = msIoOpen(arg, ms_path, flags, mode);
	// int res = g_ms_funcs.IoOpen(arg, ms_path, flags, mode);

	return res;
}

static int _efIoRename(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *oldname = (const char *)args[1];
	const char *newname = (const char *)args[2];

	char oldpath[256];
	BuildMsPathForEf(arg, oldname, oldpath);
	char newpath[256];
	BuildMsPathForEf(arg, newname, newpath);

	int res = g_ms_funcs.IoRename(arg, oldpath, newpath);
	return res;
}

static int _efIoRemove(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *file = (const char *)args[1];

	memset(ms_path, 0, sizeof(ms_path));
	BuildMsPathForEf(arg, file, ms_path);
	int res = g_ms_funcs.IoRemove(arg, ms_path);

	return res;
}

static int _efIoMkdir(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *file = (const char *)args[1];
	SceMode mode = (SceMode)args[2];

	memset(ms_path, 0, sizeof(ms_path));
	BuildMsPathForEf(arg, file, ms_path);
	int res = g_ms_funcs.IoMkdir(arg, ms_path, mode);

	return res;
}

static int _efIoRmDir(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *file = (const char *)args[1];

	memset(ms_path, 0, sizeof(ms_path));
	BuildMsPathForEf(arg, file, ms_path);
	int res = g_ms_funcs.IoRmdir(arg, ms_path);

	return res;
}

static int _efIoDopen(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *file = (const char *)args[1];

	memset(ms_path, 0, sizeof(ms_path));
	BuildMsPathForEf(arg, file, ms_path);
	int res = g_ms_funcs.IoDopen(arg, ms_path);

	return res;
}

static int _efIoGetstat(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *file = (const char *)args[1];
	SceIoStat *stat = (SceIoStat *)args[2];

	memset(ms_path, 0, sizeof(ms_path));
	BuildMsPathForEf(arg, file, ms_path);
	int res = g_ms_funcs.IoGetstat(arg, ms_path, stat);

	return res;
}

static int _efIoChstat(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *file = (const char *)args[1];
	SceIoStat *stat = (SceIoStat *)args[2];
	int bits = (int)args[3];

	memset(ms_path, 0, sizeof(ms_path));
	BuildMsPathForEf(arg, file, ms_path);
	int res = g_ms_funcs.IoChstat(arg, ms_path, stat, bits);

	return res;
}

static int _efIoChdir(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *dir = (const char *)args[1];

	memset(ms_path, 0, sizeof(ms_path));
	BuildMsPathForEf(arg, dir, ms_path);
	int res = g_ms_funcs.IoChdir(arg, ms_path);

	return res;
}

static int _efIoIoctl(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	u32 cmd = (u32)args[1];
	void *indata = (void *)args[2];
	int inlen = args[3];
	void *outdata = (void *)args[4];
	int outlen = args[5];

	int res = msIoIoctl(arg, cmd, indata, inlen, outdata, outlen);

	return res;
}

static int _efIoDevctl(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	u32 cmd = (u32)args[2];
	const char *devname = (cmd == 0x02425818) ? "ef0:" : "ms0:";
	void *indata = (void *)args[3];
	int inlen = args[4];
	void *outdata = (void *)args[5];
	int outlen = args[6];

	if (cmd == 0x02025801) {
		devname = "mscmhc0:";
	}

	if (cmd == 0x02425818) {
		sctrlSendAdrenalineCmd(ADRENALINE_VITA_CMD_EF_DEVINFO, 0);
	}

	int res = msIoDevctl(arg, devname, cmd, indata, inlen, outdata, outlen);

	return res;
}

static int efIoOpen(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode) {
	u32 args[4];
	args[0] = (u32)arg;
	args[1] = (u32)file;
	args[2] = (u32)flags;
	args[3] = (u32)mode;

	return sceKernelExtendKernelStack(0x4000, (void *)_efIoOpen, args);
}

static int efIoRename(PspIoDrvFileArg *arg, const char *oldname, const char *newname) {
	u32 args[3];
	args[0] = (u32)arg;
	args[1] = (u32)oldname;
	args[2] = (u32)newname;

	return sceKernelExtendKernelStack(0x4000, (void *)_efIoRename, args);
}

static int efIoRemove(PspIoDrvFileArg * arg, char * file) {
	u32 args[2];
	args[0] = (u32)arg;
	args[1] = (u32)file;

	return sceKernelExtendKernelStack(0x4000, (void *)_efIoRemove, args);
}

static int efIoMkdir(PspIoDrvFileArg * arg, char * file, SceMode mode) {
	u32 args[3];
	args[0] = (u32)arg;
	args[1] = (u32)file;
	args[2] = (u32)mode;

	return sceKernelExtendKernelStack(0x4000, (void *)_efIoMkdir, args);
}

static int efIoRmDir(PspIoDrvFileArg * arg, char * file) {
	u32 args[2];
	args[0] = (u32)arg;
	args[1] = (u32)file;

	return sceKernelExtendKernelStack(0x4000, (void *)_efIoRmDir, args);
}

static int efIoDopen(PspIoDrvFileArg * arg, char * file) {
	u32 args[2];
	args[0] = (u32)arg;
	args[1] = (u32)file;

	return sceKernelExtendKernelStack(0x4000, (void *)_efIoDopen, args);
}

static int efIoGetstat(PspIoDrvFileArg * arg, char * file, SceIoStat *stat) {
	u32 args[3];
	args[0] = (u32)arg;
	args[1] = (u32)file;
	args[2] = (u32)stat;

	return sceKernelExtendKernelStack(0x4000, (void *)_efIoGetstat, args);
}

static int efIoChstat(PspIoDrvFileArg * arg, char * file, SceIoStat* stat, int bits) {
	u32 args[4];
	args[0] = (u32)arg;
	args[1] = (u32)file;
	args[2] = (u32)stat;
	args[3] = (u32)bits;

	return sceKernelExtendKernelStack(0x4000, (void *)_efIoChstat, args);
}

static int efIoChdir(PspIoDrvFileArg * arg, const char *dir) {
	u32 args[2];
	args[0] = (u32)arg;
	args[1] = (u32)dir;

	return sceKernelExtendKernelStack(0x4000, (void *)_efIoChdir, args);
}

static int efIoDevctl(PspIoDrvFileArg *arg, const char *devname, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen) {
	u32 args[7];
	args[0] = (u32)arg;
	args[1] = (u32)devname;
	args[2] = (u32)cmd;
	args[3] = (u32)indata;
	args[4] = (u32)inlen;
	args[5] = (u32)outdata;
	args[6] = (u32)outlen;

	return sceKernelExtendKernelStack(0x4000, (void *)_efIoDevctl, args);
}

static int efIoIoctl(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen) {
	u32 args[6];
	args[0] = (u32)arg;
	args[1] = (u32)cmd;
	args[2] = (u32)indata;
	args[3] = (u32)inlen;
	args[4] = (u32)outdata;
	args[5] = (u32)outlen;

	return sceKernelExtendKernelStack(0x4000, (void *)_efIoIoctl, args);
}

__attribute__((noinline)) static int BuildMsPathChangeFsNum(PspIoDrvFileArg *arg, const char *name, char *ms_path) {
	sprintf(ms_path, "/__ADRENALINE__/flash%d%s", (int)arg->fs_num, name);

	int fs_num = arg->fs_num;
	arg->fs_num = 0;
	return fs_num;
}

////////////////////////////////////////////////////////////////////////////////
// FLASH0 FONT READ CACHE
////////////////////////////////////////////////////////////////////////////////

// `sceFont` reads glyphs straight from the `/font/*.pgf` files with a seek plus
// a few tiny reads per character, so redrawing a screen of text costs hundreds
// of I/O calls (e.g. Dissidia 012 menus froze ~280ms per cursor move). Each call
// is expensive on vPSP: ~0.75ms through the Memory Stick redirection, and still
// ~0.35ms through the in-memory flash0 image.
//
// Read-only font files are served from a small LRU cache of fixed-size blocks,
// and seeks on them only move a position kept here.

#define FONT_CACHE_PREFIX "/font/"
#define FONT_CACHE_BLOCK_SIZE (4 * 1024)
#define FONT_CACHE_MAX_FILES 8

typedef struct {
	PspIoDrvFileArg *arg;
	PspIoDrvFuncs *funcs;
	u32 size;
	u32 pos;
} FontCacheFile;

typedef struct {
	PspIoDrvFileArg *arg;
	u32 base;
	u32 len;
	u32 stamp;
} FontCacheBlock;

static FontCacheFile g_font_files[FONT_CACHE_MAX_FILES];
static FontCacheBlock *g_font_blocks = NULL;
static u8 *g_font_cache_mem = NULL;
static u32 g_font_cache_nblocks = 0;
static u32 g_font_cache_clock = 0;
static SceUID g_font_cache_sema = -1;
static int g_font_cache_failed = 0;

// Lazily allocated on the first font open, so nothing is reserved when no font is used
static int FontCacheInit() {
	if (g_font_cache_nblocks) {
		return 1;
	}

	if (g_font_cache_failed || g_font_cache_sema < 0) {
		return 0;
	}

	// Keep kernel memory free for plugins: use the extra RAM partition, unless the
	// high memory layout has given it to the user partition
	int partition_id = 11;
	PspSysMemPartition *user_partition = sctrlGetMemoryPartition(PSP_MEMORY_PARTITION_USER);
	if (user_partition && user_partition->size > (24 * 1024 * 1024)) {
		partition_id = PSP_MEMORY_PARTITION_KERNEL;
	}

	static const u32 counts[] = { 96, 64, 32, 16 };
	for (int i = 0; i < (sizeof(counts) / sizeof(u32)); i++) {
		u32 n = counts[i];
		u32 size = n * FONT_CACHE_BLOCK_SIZE + n * sizeof(FontCacheBlock);
		SceUID uid = sceKernelAllocPartitionMemory(partition_id, "EPI_FontCache", PSP_SMEM_High, size, NULL);
		if (uid >= 0) {
			g_font_cache_mem = (u8 *)sceKernelGetBlockHeadAddr(uid);
			g_font_blocks = (FontCacheBlock *)(g_font_cache_mem + n * FONT_CACHE_BLOCK_SIZE);
			for (int j = 0; j < n; j++) {
				g_font_blocks[j].arg = NULL;
			}
			g_font_cache_nblocks = n;
			logmsg3("[INFO]: %s: %ld blocks of %d bytes in partition %d\n", __func__, n, FONT_CACHE_BLOCK_SIZE, partition_id);
			return 1;
		}
	}

	logmsg("[WARN]: %s: Could not allocate font cache\n", __func__);
	g_font_cache_failed = 1;
	return 0;
}

static FontCacheFile *FontCacheFind(PspIoDrvFileArg *arg) {
	if (!g_font_cache_nblocks) {
		return NULL;
	}

	for (int i = 0; i < FONT_CACHE_MAX_FILES; i++) {
		if (g_font_files[i].arg == arg) {
			return &g_font_files[i];
		}
	}

	return NULL;
}

static void FontCacheOpen(PspIoDrvFileArg *arg, PspIoDrvFuncs *funcs, const char *file, int flags) {
	if ((flags & PSP_O_WRONLY) || strncmp(file, FONT_CACHE_PREFIX, sizeof(FONT_CACHE_PREFIX) - 1) != 0) {
		return;
	}

	sceKernelWaitSema(g_font_cache_sema, 1, NULL);

	FontCacheFile *f = NULL;
	if (FontCacheInit()) {
		for (int i = 0; i < FONT_CACHE_MAX_FILES; i++) {
			if (g_font_files[i].arg == NULL) {
				f = &g_font_files[i];
				break;
			}
		}
	}

	if (f) {
		arg->fs_num = 0;
		SceOff end = funcs->IoLseek(arg, 0, PSP_SEEK_END);
		funcs->IoLseek(arg, 0, PSP_SEEK_SET);

		if (end > 0) {
			f->arg = arg;
			f->funcs = funcs;
			f->size = (u32)end;
			f->pos = 0;
		}
	}

	sceKernelSignalSema(g_font_cache_sema, 1);
}

static void FontCacheClose(FontCacheFile *f) {
	sceKernelWaitSema(g_font_cache_sema, 1, NULL);

	for (int i = 0; i < g_font_cache_nblocks; i++) {
		if (g_font_blocks[i].arg == f->arg) {
			g_font_blocks[i].arg = NULL;
		}
	}
	f->arg = NULL;

	sceKernelSignalSema(g_font_cache_sema, 1);
}

// Returns the block holding `base` for the file, reading it on a miss
static FontCacheBlock *FontCacheGetBlock(FontCacheFile *f, u32 base, int *err) {
	FontCacheBlock *victim = NULL;

	for (int i = 0; i < g_font_cache_nblocks; i++) {
		FontCacheBlock *b = &g_font_blocks[i];
		if (b->arg == f->arg && b->base == base) {
			b->stamp = ++g_font_cache_clock;
			return b;
		}

		// Prefer an empty block, otherwise the least recently used one
		if (!victim || (victim->arg && (!b->arg || b->stamp < victim->stamp))) {
			victim = b;
		}
	}

	u8 *dst = g_font_cache_mem + (u32)(victim - g_font_blocks) * FONT_CACHE_BLOCK_SIZE;

	f->arg->fs_num = 0;
	f->funcs->IoLseek(f->arg, base, PSP_SEEK_SET);
	int res = f->funcs->IoRead(f->arg, (char *)dst, FONT_CACHE_BLOCK_SIZE);
	if (res <= 0) {
		victim->arg = NULL;
		*err = res;
		return NULL;
	}

	victim->arg = f->arg;
	victim->base = base;
	victim->len = (u32)res;
	victim->stamp = ++g_font_cache_clock;

	return victim;
}

static int FontCacheRead(FontCacheFile *f, char *data, int len) {
	if (len <= 0) {
		return 0;
	}

	sceKernelWaitSema(g_font_cache_sema, 1, NULL);

	int err = 0;
	u32 done = 0;
	u32 want = (f->pos < f->size) ? (f->size - f->pos) : 0;
	if ((u32)len < want) {
		want = (u32)len;
	}

	if (want >= FONT_CACHE_BLOCK_SIZE) {
		// Large reads gain nothing from the cache: read them directly
		f->arg->fs_num = 0;
		f->funcs->IoLseek(f->arg, f->pos, PSP_SEEK_SET);
		int res = f->funcs->IoRead(f->arg, data, want);
		if (res > 0) {
			done = res;
			f->pos += res;
		} else {
			err = res;
		}
	} else {
		while (done < want) {
			u32 base = f->pos & ~(FONT_CACHE_BLOCK_SIZE - 1);
			FontCacheBlock *b = FontCacheGetBlock(f, base, &err);
			if (!b) {
				break;
			}

			u32 offset = f->pos - base;
			if (offset >= b->len) {
				break;
			}

			u32 n = b->len - offset;
			if (n > want - done) {
				n = want - done;
			}

			memcpy(data + done, g_font_cache_mem + (u32)(b - g_font_blocks) * FONT_CACHE_BLOCK_SIZE + offset, n);
			done += n;
			f->pos += n;
		}
	}

	sceKernelSignalSema(g_font_cache_sema, 1);

	return (done == 0 && err < 0) ? err : (int)done;
}

static SceOff FontCacheLseek(FontCacheFile *f, s32 ofs, int whence) {
	SceOff pos;

	switch (whence) {
		case PSP_SEEK_CUR:
			pos = (SceOff)f->pos + ofs;
			break;
		case PSP_SEEK_END:
			pos = (SceOff)f->size + ofs;
			break;
		default:
			pos = ofs;
			break;
	}

	if (pos < 0) {
		return SCE_EINVAL;
	}

	f->pos = (u32)pos;
	return pos;
}

static int _flashIoOpen(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	char *file = (char *)args[1];
	int flags = args[2];
	SceMode mode = (SceMode)args[3];

	if (g_config.osk_type == OSK_TYPE_VITA) {
		if (strcmp(file, "/vsh/module/osk_plugin.prx") == 0) {
			file = "/vsh/module/kermit_osk_plugin.prx";
		}
	}

	if (strcmp(file, "/kd/mpeg_vsh.prx") == 0) {
		file = "/kd/mpeg.prx";
	}

	char ms_path[128];
	int fs_num = BuildMsPathChangeFsNum(arg, file, ms_path);

	// Fonts are read with a seek plus tiny reads per glyph, and every access on a
	// redirected file is a round trip to the VITA side. Open them from the
	// in-memory flash0 image first, and only fall back to the redirection.
	if (fs_num == 0 && strncmp(file, "/font/", 6) == 0) {
		arg->fs_num = fs_num;
		int res = g_flash_funcs.IoOpen(arg, file, flags, mode);
		if (res >= 0) {
			FontCacheOpen(arg, &g_flash_funcs, file, flags);
			return res;
		}
		arg->fs_num = 0;
	}

	int res = g_ms_funcs.IoOpen(arg, ms_path, flags, mode);
	if (res >= 0) {
		((u32 *)arg)[18] = 1;

		if (fs_num == 0) {
			FontCacheOpen(arg, &g_ms_funcs, file, flags);
		}
	}

	if (fs_num == 1 || res >= 0) {
		return res;
	}

	arg->fs_num = fs_num;

	return g_flash_funcs.IoOpen(arg, file, flags, mode);
}

static int _flashIoClose(PspIoDrvFileArg *arg) {
	FontCacheFile *f = FontCacheFind(arg);
	if (f) {
		FontCacheClose(f);
	}

	if (((u32 *)arg)[18]) {
		arg->fs_num = 0;
		return g_ms_funcs.IoClose(arg);
	}

	return g_flash_funcs.IoClose(arg);
}

static int _flashIoRead(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	char *data = (char *)args[1];
	int len = args[2];

	FontCacheFile *f = FontCacheFind(arg);
	if (f) {
		return FontCacheRead(f, data, len);
	}

	if (((u32 *)arg)[18]) {
		arg->fs_num = 0;
		return g_ms_funcs.IoRead(arg, data, len);
	}

	return g_flash_funcs.IoRead(arg, data, len);
}

static int _flashIoWrite(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *data = (const char *)args[1];
	int len = args[2];

	if (!data && len == 0) {
		return 0;
	}

	if (((u32 *)arg)[18]) {
		arg->fs_num = 0;
		return g_ms_funcs.IoWrite(arg, data, len);
	}

	return g_flash_funcs.IoWrite(arg, data, len);
}

static SceOff _flashIoLseek(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	u32 ofs = (u32)args[1];
	int whence = args[2];

	FontCacheFile *f = FontCacheFind(arg);
	if (f) {
		return FontCacheLseek(f, (s32)ofs, whence);
	}

	if (((u32 *)arg)[18]) {
		arg->fs_num = 0;
		return g_ms_funcs.IoLseek(arg, ofs, whence);
	}

	return g_flash_funcs.IoLseek(arg, ofs, whence);
}

static int _flashIoRemove(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *name = (const char *)args[1];

	char ms_path[128];
	int fs_num = BuildMsPathChangeFsNum(arg, name, ms_path);
	int res = g_ms_funcs.IoRemove(arg, ms_path);

	if (fs_num == 1 || res >= 0) {
		return res;
	}

	arg->fs_num = fs_num;
	return g_flash_funcs.IoRemove(arg, name);
}

static int _flashIoMkdir(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *name = (const char *)args[1];
	SceMode mode = (SceMode)args[2];

	char ms_path[128];
	int fs_num = BuildMsPathChangeFsNum(arg, name, ms_path);
	int res = g_ms_funcs.IoMkdir(arg, ms_path, mode);

	if (fs_num == 1 || res >= 0) {
		return res;
	}

	arg->fs_num = fs_num;
	return g_flash_funcs.IoMkdir(arg, name, mode);
}

static int _flashIoRmdir(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *name = (const char *)args[1];

	char ms_path[128];
	int fs_num = BuildMsPathChangeFsNum(arg, name, ms_path);
	int res = g_ms_funcs.IoRmdir(arg, ms_path);

	if (fs_num == 1 || res >= 0) {
		return res;
	}

	arg->fs_num = fs_num;
	return g_flash_funcs.IoRmdir(arg, name);
}

static int _flashIoDopen(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *dirname = (const char *)args[1];

	char ms_path[128];
	int fs_num = BuildMsPathChangeFsNum(arg, dirname, ms_path);
	int res = g_ms_funcs.IoDopen(arg, ms_path);
	if (res >= 0) {
		((u32 *)arg)[18] = 1;
	}

	if (fs_num == 1 || res >= 0) {
		return res;
	}

	arg->fs_num = fs_num;
	return g_flash_funcs.IoDopen(arg, dirname);
}

static int _flashIoDclose(PspIoDrvFileArg *arg) {
	if (((u32 *)arg)[18]) {
		arg->fs_num = 0;
		return g_ms_funcs.IoDclose(arg);
	}

	return g_flash_funcs.IoDclose(arg);
}

static int _flashIoDread(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	SceIoDirent *dir = (SceIoDirent *)args[1];

	if (((u32 *)arg)[18]) {
		arg->fs_num = 0;
		return g_ms_funcs.IoDread(arg, dir);
	}

	return g_flash_funcs.IoDread(arg, dir);
}

static int _flashIoGetstat(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *file = (const char *)args[1];
	SceIoStat *stat = (SceIoStat *)args[2];

	char ms_path[128];
	int fs_num = BuildMsPathChangeFsNum(arg, file, ms_path);
	int res = g_ms_funcs.IoGetstat(arg, ms_path, stat);

	if (fs_num == 1 || res >= 0) {
		return res;
	}

	arg->fs_num = fs_num;
	return g_flash_funcs.IoGetstat(arg, file, stat);
}

static int _flashIoChstat(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *file = (const char *)args[1];
	SceIoStat *stat = (SceIoStat *)args[2];
	int bits = (int)args[3];

	char ms_path[128];
	int fs_num = BuildMsPathChangeFsNum(arg, file, ms_path);
	int res = g_ms_funcs.IoChstat(arg, ms_path, stat, bits);

	if (fs_num == 1 || res >= 0) {
		return res;
	}

	arg->fs_num = fs_num;
	return g_flash_funcs.IoChstat(arg, file, stat, bits);
}

static int _flashIoChdir(u32 *args) {
	PspIoDrvFileArg *arg = (PspIoDrvFileArg *)args[0];
	const char *dir = (const char *)args[1];

	char ms_path[128];
	int fs_num = BuildMsPathChangeFsNum(arg, dir, ms_path);
	int res = g_ms_funcs.IoChdir(arg, ms_path);

	if (fs_num == 1 || res >= 0) {
		return res;
	}

	arg->fs_num = fs_num;
	return g_flash_funcs.IoChdir(arg, dir);
}

static int flashIoOpen(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode) {
	u32 args[4];
	args[0] = (u32)arg;
	args[1] = (u32)file;
	args[2] = (u32)flags;
	args[3] = (u32)mode;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoOpen, args);
}

static int flashIoClose(PspIoDrvFileArg *arg) {
	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoClose, arg);
}

static int flashIoRead(PspIoDrvFileArg *arg, char *data, int len) {
	u32 args[3];
	args[0] = (u32)arg;
	args[1] = (u32)data;
	args[2] = (u32)len;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoRead, args);
}

static int flashIoWrite(PspIoDrvFileArg *arg, const char *data, int len) {
	u32 args[3];
	args[0] = (u32)arg;
	args[1] = (u32)data;
	args[2] = (u32)len;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoWrite, args);
}

static SceOff flashIoLseek(PspIoDrvFileArg *arg, SceOff ofs, int whence) {
	u32 args[3];
	args[0] = (u32)arg;
	args[1] = (u32)ofs;
	args[2] = (u32)whence;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoLseek, args);
}

static int flashIoIoctl(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen) {
	int res = NoDevicePatch(cmd);
	if (res <= 0) {
		return res;
	}

	return 0;
}

static int flashIoRemove(PspIoDrvFileArg *arg, const char *name) {
	u32 args[2];
	args[0] = (u32)arg;
	args[1] = (u32)name;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoRemove, args);
}

static int flashIoMkdir(PspIoDrvFileArg *arg, const char *name, SceMode mode) {
	u32 args[3];
	args[0] = (u32)arg;
	args[1] = (u32)name;
	args[2] = (u32)mode;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoMkdir, args);
}

static int flashIoRmdir(PspIoDrvFileArg *arg, const char *name) {
	u32 args[2];
	args[0] = (u32)arg;
	args[1] = (u32)name;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoRmdir, args);
}

static int flashIoDopen(PspIoDrvFileArg *arg, const char *dirname) {
	u32 args[2];
	args[0] = (u32)arg;
	args[1] = (u32)dirname;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoDopen, args);
}

static int flashIoDclose(PspIoDrvFileArg *arg) {
	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoDclose, arg);
}

static int flashIoDread(PspIoDrvFileArg *arg, SceIoDirent *dir) {
	u32 args[2];
	args[0] = (u32)arg;
	args[1] = (u32)dir;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoDread, args);
}

static int flashIoGetstat(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat) {
	u32 args[3];
	args[0] = (u32)arg;
	args[1] = (u32)file;
	args[2] = (u32)stat;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoGetstat, args);
}

static int flashIoChstat(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat, int bits) {
	u32 args[4];
	args[0] = (u32)arg;
	args[1] = (u32)file;
	args[2] = (u32)stat;
	args[3] = (u32)bits;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoChstat, args);
}

static int flashIoChdir(PspIoDrvFileArg *arg, const char *dir) {
	u32 args[2];
	args[0] = (u32)arg;
	args[1] = (u32)dir;

	return sceKernelExtendKernelStack(0x4000, (void *)_flashIoChdir, args);
}

static int flashIoDevctl(PspIoDrvFileArg *arg, const char *devname, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen) {
	return 0;
}

static int sceIoAddDrvPatched(PspIoDrv *drv) {
	if (strcmp(drv->name, "ms") == 0) {
		g_ms_drv = drv;
		return 0;
	} else if (strcmp(drv->name, "flashfat") == 0) {
		g_flashfat_drv = drv;
		return 0;
	} else if (strcmp(drv->name, "fatms") == 0) {
		memcpy(&g_ms_funcs, drv->funcs, sizeof(PspIoDrvFuncs));

		drv->funcs->IoOpen = msIoOpen;
		drv->funcs->IoIoctl = msIoIoctl;
		drv->funcs->IoDevctl = msIoDevctl;

		// Add ms driver
		g_ms_drv->funcs = drv->funcs;

		// Add ef driver
		memcpy(&g_ef_funcs, drv->funcs, sizeof(PspIoDrvFuncs));
		g_ef_funcs.IoOpen = (void*)efIoOpen;
		g_ef_funcs.IoRemove = (void*)efIoRemove;
		g_ef_funcs.IoMkdir = (void*)efIoMkdir;
		g_ef_funcs.IoRmdir = (void*)efIoRmDir;
		g_ef_funcs.IoDopen = (void*)efIoDopen;
		g_ef_funcs.IoGetstat = (void*)efIoGetstat;
		g_ef_funcs.IoChstat = (void*)efIoChstat;
		g_ef_funcs.IoRename = (void*)efIoRename;
		g_ef_funcs.IoChdir = (void*)efIoChdir;
		g_ef_funcs.IoDevctl = (void*)efIoDevctl;
		g_ef_funcs.IoIoctl = (void*)efIoIoctl;

		memcpy(&g_ef_drv, g_ms_drv, sizeof(PspIoDrv));
		g_ef_drv.name = "ef";
		g_ef_drv.name2 = "EF";
		g_ef_drv.funcs = &g_ef_funcs;

		memcpy(&g_fatef_drv, g_ms_drv, sizeof(PspIoDrv));
		g_fatef_drv.name = "fatef";
		g_fatef_drv.name2 = "FATEF";
		g_fatef_drv.funcs = &g_ef_funcs;

		_sceIoAddDrv(g_ms_drv);
		_sceIoAddDrv(&g_ef_drv);
		_sceIoAddDrv(&g_fatef_drv);
	} else if (strcmp(drv->name, "flash") == 0) {
		memcpy(&g_flash_funcs, drv->funcs, sizeof(PspIoDrvFuncs));

		drv->funcs->IoOpen = flashIoOpen;
		drv->funcs->IoClose = flashIoClose;
		drv->funcs->IoRead = flashIoRead;
		drv->funcs->IoWrite = flashIoWrite;
		drv->funcs->IoLseek = flashIoLseek;
		drv->funcs->IoIoctl = flashIoIoctl;
		drv->funcs->IoRemove = flashIoRemove;
		drv->funcs->IoMkdir = flashIoMkdir;
		drv->funcs->IoRmdir = flashIoRmdir;
		drv->funcs->IoDopen = flashIoDopen;
		drv->funcs->IoDclose = flashIoDclose;
		drv->funcs->IoDread = flashIoDread;
		drv->funcs->IoGetstat = flashIoGetstat;
		drv->funcs->IoChstat = flashIoChstat;
		drv->funcs->IoChdir = flashIoChdir;
		drv->funcs->IoDevctl = flashIoDevctl;

		// Add flashfat driver
		g_flashfat_drv->funcs = drv->funcs;
		_sceIoAddDrv(g_flashfat_drv);
	}

	return _sceIoAddDrv(drv);
}

static int sceIoDelDrvPatched(const char *drv_name) {
	if (strcmp(drv_name, "ms") == 0 || strcmp(drv_name, "flashfat") == 0) {
		return 0;
	} else if (strcmp(drv_name, "fatms") == 0) {
		_sceIoDelDrv("ms");
	} else if (strcmp(drv_name, "flash") == 0) {
		_sceIoDelDrv("flashfat");
	}

	return _sceIoDelDrv(drv_name);
}

static int sceIoUnassignPatched(const char *dev) {
	int k1 = pspSdkSetK1(0);

	if (strncmp(dev, "ms", 2) == 0 || strncmp(dev, "ef", 2) == 0 || strncmp(dev, "flash", 5) == 0) {
		pspSdkSetK1(k1);
		return 0;
	}

	pspSdkSetK1(k1);
	return _sceIoUnassign(dev);
}

static int sceIoAssignPatched(const char *dev1, const char *dev2, const char *dev3, int mode, void* unk1, long unk2) {
	int k1 = pspSdkSetK1(0);

	if (strncmp(dev1, "ms", 2) == 0  || strncmp(dev1, "ef", 2) == 0 || strncmp(dev1, "flash", 5) == 0) {
		pspSdkSetK1(k1);
		return 0;
	}

	pspSdkSetK1(k1);
	return _sceIoAssign(dev1, dev2, dev3, mode, unk1, unk2);
}

void PatchIoFileMgr(void) {
	g_font_cache_sema = sceKernelCreateSema("EPI_FontCacheSema", 0, 1, 1, NULL);

	HIJACK_FUNCTION(K_EXTRACT_IMPORT(&sceIoAddDrv), sceIoAddDrvPatched, _sceIoAddDrv);
	HIJACK_FUNCTION(K_EXTRACT_IMPORT(&sceIoDelDrv), sceIoDelDrvPatched, _sceIoDelDrv);

	// This fixes popsman flash2 assign
	HIJACK_FUNCTION(K_EXTRACT_IMPORT(&sceIoUnassign), sceIoUnassignPatched, _sceIoUnassign);
	HIJACK_FUNCTION(K_EXTRACT_IMPORT(&sceIoAssign), sceIoAssignPatched, _sceIoAssign);
}

// void PatchRedirectMsToEf() {
// 	// Redirect `ms0:` to `ef0:`
// 	int apitype = g_adrenaline->fake_api_type;
// 	if (apitype == PSP_INIT_APITYPE_EF2
// 		|| apitype == PSP_INIT_APITYPE_UMD_EMU_EF1
// 		|| apitype == PSP_INIT_APITYPE_UMD_EMU_EF2
// 		|| apitype == PSP_INIT_APITYPE_EF5)
// 	{
// 		memcpy(g_ms_drv->funcs, &g_ef_funcs, sizeof(PspIoDrvFuncs));
// 	}
// }