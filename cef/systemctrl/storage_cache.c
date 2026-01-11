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

#include <common.h>
#include <adrenaline_log.h>

//#define CHECK_MODE
#undef CHECK_MODE

// Not much of a difference when past 4Kb
#define MSCACHE_SIZE (4*1024)

#define INVALID_POS (-1)

// Cache Structure
typedef struct MsCache {
	char * buf;
	int bufSize;
	// Invalid = -1
	SceOff pos;
	int age;
} MsCache;

static PspIoDrv* hooked_drv = NULL;
static SceUID cache_mem = -1;

static MsCache g_cache;
int g_cacheSize = 0;

#ifdef DEBUG
// Cache Statistic
unsigned int cacheReadTimes = 0;
unsigned int cacheHit = 0;
unsigned int cacheMissed = 0;
unsigned int cacheUncacheable = 0;
#endif

// Original Function Pointer
int (* storageRead)(PspIoDrvFileArg * arg, char * data, int len) = NULL;
int (* storageWrite)(PspIoDrvFileArg * arg, const char * data, int len) = NULL;
SceOff (* storageLseek)(PspIoDrvFileArg * arg, SceOff ofs, int whence) = NULL;
int(* storageOpen)(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode) = NULL;

// Check Range Overlapping
static inline int isCacheWithinRange(SceOff pos, SceOff start, int len) {
	if (pos >= start && pos < start + len) {
		return 1;
	}

	// Out of Range
	return 0;
}

static struct MsCache * getHitCache(SceOff pos, int len) {
	if (g_cache.pos != INVALID_POS) {
		if (isCacheWithinRange(pos, g_cache.pos, g_cache.bufSize) && isCacheWithinRange(pos+len-1, g_cache.pos, g_cache.bufSize)) {
			return &g_cache;
		}
	}

	return NULL;
}

static void disableCache(MsCache * cache) {
	cache->pos = INVALID_POS;
	cache->bufSize = 0;
}

// Disable Cache within Range
static void disableCacheWithinRange(SceOff pos, int len) {
	if (g_cache.pos != INVALID_POS) {
		if (!(pos + len <= g_cache.pos || pos >= g_cache.pos + g_cache.bufSize)) {
			disableCache(&g_cache);
		}
	}
}

// sceIoRead Hook
static int storageReadCache(PspIoDrvFileArg * arg, char * data, int len) {
	int result = 0;
	int read_len = 0;

	// Get position in the file
	SceOff pos = storageLseek(arg, 0, PSP_SEEK_CUR);

	MsCache * cache = getHitCache(pos, len);

	// Found cached portion of the file
	if (cache != NULL) {
		#ifndef CHECK_MODE
		// Calculate buffered file content size
		read_len = MIN(len, cache->pos + cache->bufSize - pos);

		// Copy Buffered Data
		memcpy(data, cache->buf + pos - cache->pos, read_len);
		result = read_len;

		// Move position in the file
		storageLseek(arg, pos + result, PSP_SEEK_SET);
		#else
		// Check validate code
		result = (*storageRead)(arg, data, len);

		if (0 != memcmp(data, cache->buf + pos - cache->pos, len)) {
			#ifdef DEBUG
			printk("%s: 0x%08X <%d> cache mismatched!!!\r\n", __func__, (uint)pos, (int)len);
			#endif
			VWRITE32(0, 0);
		}
		#endif

		#ifdef DEBUG
		cacheHit += len;
		#endif


	// No Cache available
	} else {
		cache = &g_cache;

		// Requested Length fits into Cache
		if (len <= g_cacheSize) {
			disableCache(cache);

			result = storageRead(arg, cache->buf, g_cacheSize);

			if (result > 0) {
				// Save cache buffer size
				read_len = result;
				// Only give the caller the data he wants
				result = MIN(len, result);

				// Copy data into cache
				memcpy(data, cache->buf, result);

				cache->pos = pos;
				cache->bufSize = read_len;

				storageLseek(arg, pos + result, PSP_SEEK_SET);
			}

			#ifdef DEBUG
			cacheMissed += len;
			#endif

		// Too big to cache...
		} else {
			result = storageRead(arg, data, len);

			#ifdef DEBUG
			cacheUncacheable += len;
			#endif
		}
	}

	#ifdef DEBUG
	cacheReadTimes += len;
	#endif

	return result;
}

// sceIoWrite Hook
static int storageWriteCache(PspIoDrvFileArg * arg, const char * data, int len) {
	SceOff pos = storageLseek(arg, 0, PSP_SEEK_CUR);

	disableCacheWithinRange(pos, len);

	return storageWrite(arg, data, len);
}

// sceIoOpen Hook
static int storageOpenCache(PspIoDrvFileArg * arg, char * file, int flags, SceMode mode) {
	// New File = New Cache
	disableCache(&g_cache);

	return storageOpen(arg, file, flags, mode);
}

static int (*storageIoIoctl)(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen) = NULL;
static int storageIoIoctlCache(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen) {
	disableCache(&g_cache);
	return storageIoIoctl(arg, cmd, indata, inlen, outdata, outlen);
}

static int (*storageIoRemove)(PspIoDrvFileArg *arg, const char *name) = NULL;
static int storageIoRemoveCache(PspIoDrvFileArg *arg, const char *name) {
	disableCache(&g_cache);
	return storageIoRemove(arg, name);
}

static int (*storageIoMkdir)(PspIoDrvFileArg *arg, const char *name, SceMode mode) = NULL;
static int storageIoMkdirCache(PspIoDrvFileArg *arg, const char *name, SceMode mode) {
	disableCache(&g_cache);
	return storageIoMkdir(arg, name, mode);
}

static int (*storageIoRmdir)(PspIoDrvFileArg *arg, const char *name) = NULL;
static int storageIoRmdirCache(PspIoDrvFileArg *arg, const char *name) {
	disableCache(&g_cache);
	return storageIoRmdir(arg, name);
}

static int (*storageIoDopen)(PspIoDrvFileArg *arg, const char *dirname) = NULL;
static int storageIoDopenCache(PspIoDrvFileArg *arg, const char *dirname) {
	disableCache(&g_cache);
	return storageIoDopen(arg, dirname);
}

static int (*storageIoDclose)(PspIoDrvFileArg *arg) = NULL;
static int storageIoDcloseCache(PspIoDrvFileArg *arg) {
	disableCache(&g_cache);
	return storageIoDclose(arg);
}

static int (*storageIoDread)(PspIoDrvFileArg *arg, SceIoDirent *dir) = NULL;
static int storageIoDreadCache(PspIoDrvFileArg *arg, SceIoDirent *dir) {
	disableCache(&g_cache);
	return storageIoDread(arg, dir);
}

static int (*storageIoGetstat)(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat) = NULL;
static int storageIoGetstatCache(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat) {
	disableCache(&g_cache);
	return storageIoGetstat(arg, file, stat);
}

static int (*storageIoChstat)(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat, int bits) = NULL;
static int storageIoChstatCache(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat, int bits) {
	disableCache(&g_cache);
	return storageIoChstat(arg, file, stat, bits);
}

static int (*storageIoRename)(PspIoDrvFileArg *arg, const char *oldname, const char *newname) = NULL;
static int storageIoRenameCache(PspIoDrvFileArg *arg, const char *oldname, const char *newname) {
	disableCache(&g_cache);
	return storageIoRename(arg, oldname, newname);
}

static int (*storageIoChdir)(PspIoDrvFileArg *arg, const char *dir) = NULL;
static int storageIoChdirCache(PspIoDrvFileArg *arg, const char *dir) {
	disableCache(&g_cache);
	return storageIoChdir(arg, dir);
}

static int (*storageIoMount)(PspIoDrvFileArg *arg) = NULL;
static int storageIoMountCache(PspIoDrvFileArg *arg) {
	disableCache(&g_cache);
	return storageIoMount(arg);
}

static int (*storageIoUmount)(PspIoDrvFileArg *arg) = NULL;
static int storageIoUmountCache(PspIoDrvFileArg *arg) {
	disableCache(&g_cache);
	return storageIoUmount(arg);
}

static int (*storageIoDevctl)(PspIoDrvFileArg *arg, const char *devname, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen) = NULL;
static int storageIoDevctlCache(PspIoDrvFileArg *arg, const char *devname, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen) {
	disableCache(&g_cache);
	return storageIoDevctl(arg, devname, cmd, indata, inlen, outdata, outlen);
}

static int (*storageIoUnk21)(PspIoDrvFileArg *arg) = NULL;
static int storageIoUnk21Cache(PspIoDrvFileArg *arg) {
	disableCache(&g_cache);
	return storageIoUnk21(arg);
}

// Initialize "ms" Driver Cache
int storageCacheInit(const char* driver) {
	if (driver == NULL){
		if (hooked_drv){
			// Unhook Driver Functions
			hooked_drv->funcs->IoRead = storageRead;
			hooked_drv->funcs->IoWrite = storageWrite;
			hooked_drv->funcs->IoOpen= storageOpen;
			hooked_drv->funcs->IoIoctl = storageIoIoctl;
			hooked_drv->funcs->IoRemove = storageIoRemove;
			hooked_drv->funcs->IoMkdir = storageIoMkdir;
			hooked_drv->funcs->IoRmdir = storageIoRmdir;
			hooked_drv->funcs->IoDopen = storageIoDopen;
			hooked_drv->funcs->IoDclose = storageIoDclose;
			hooked_drv->funcs->IoDread = storageIoDread;
			hooked_drv->funcs->IoGetstat = storageIoGetstat;
			hooked_drv->funcs->IoChstat = storageIoChstat;
			hooked_drv->funcs->IoRename = storageIoRename;
			hooked_drv->funcs->IoChdir = storageIoChdir;
			hooked_drv->funcs->IoMount = storageIoMount;
			hooked_drv->funcs->IoUmount = storageIoUmount;
			hooked_drv->funcs->IoDevctl = storageIoDevctl;
			hooked_drv->funcs->IoUnk21 = storageIoUnk21;
		}
		sceKernelFreePartitionMemory(cache_mem);
		cache_mem = -1;
		g_cache.buf = NULL;
		g_cache.bufSize = 0;
		return 0;
	}

	// cache already on
	if (g_cacheSize > 0) {
		return 0;
	}

	int app_type = sceKernelApplicationType();

	// Not needed on POPS
	if (app_type == PSP_INIT_KEYCONFIG_POPS) {
		return 0;
	}

	PspIoDrv * pdrv = sctrlHENFindDriver(driver);

	// Driver unavailable
	if (pdrv == NULL) {
		return SCE_KERR_DRIVER_DELETED;
	}

	// Allocate Memory
	SceUID memid = sceKernelAllocPartitionMemory(1, "storageCache", PSP_SMEM_High, MSCACHE_SIZE + 64, NULL);
	cache_mem = memid;

	if (memid < 0) {
		return SCE_ENOMEM;
	}

	// Get Memory Pointer
	g_cache.buf = sceKernelGetBlockHeadAddr(memid);

	// Couldn't fetch Pointer
	if (g_cache.buf == NULL) {
		return SCE_KERR_ILLEGAL_MEMBLOCK;
	}

	// Align Buffer to 64
	g_cache.buf = PTR_ALIGN_64(g_cache.buf);

	// Set Cache Size
	g_cacheSize = MSCACHE_SIZE;

	disableCache(&g_cache);

	// Fetch Driver Functions
	hooked_drv = pdrv;
	storageRead = pdrv->funcs->IoRead;
	storageWrite = pdrv->funcs->IoWrite;
	storageLseek = pdrv->funcs->IoLseek;
	storageOpen = pdrv->funcs->IoOpen;
	storageIoIoctl = pdrv->funcs->IoIoctl;
	storageIoRemove = pdrv->funcs->IoRemove;
	storageIoMkdir = pdrv->funcs->IoMkdir;
	storageIoRmdir = pdrv->funcs->IoRmdir;
	storageIoDopen = pdrv->funcs->IoDopen;
	storageIoDclose = pdrv->funcs->IoDclose;
	storageIoDread = pdrv->funcs->IoDread;
	storageIoGetstat = pdrv->funcs->IoGetstat;
	storageIoChstat = pdrv->funcs->IoChstat;
	storageIoRename = pdrv->funcs->IoRename;
	storageIoChdir = pdrv->funcs->IoChdir;
	storageIoMount = pdrv->funcs->IoMount;
	storageIoUmount = pdrv->funcs->IoUmount;
	storageIoDevctl = pdrv->funcs->IoDevctl;
	storageIoUnk21 = pdrv->funcs->IoUnk21;

	// Hook Driver Functions
	if (storageRead) pdrv->funcs->IoRead = storageReadCache;
	if (storageWrite) pdrv->funcs->IoWrite = storageWriteCache;
	if (storageOpen) pdrv->funcs->IoOpen= storageOpenCache;
	if (storageIoIoctl) pdrv->funcs->IoIoctl = storageIoIoctlCache;
	if (storageIoRemove) pdrv->funcs->IoRemove = storageIoRemoveCache;
	if (storageIoMkdir) pdrv->funcs->IoMkdir = storageIoMkdirCache;
	if (storageIoRmdir) pdrv->funcs->IoRmdir = storageIoRmdirCache;
	if (storageIoDopen) pdrv->funcs->IoDopen = storageIoDopenCache;
	if (storageIoDclose) pdrv->funcs->IoDclose = storageIoDcloseCache;
	if (storageIoDread) pdrv->funcs->IoDread = storageIoDreadCache;
	if (storageIoGetstat) pdrv->funcs->IoGetstat = storageIoGetstatCache;
	if (storageIoChstat) pdrv->funcs->IoChstat = storageIoChstatCache;
	if (storageIoRename) pdrv->funcs->IoRename = storageIoRenameCache;
	if (storageIoChdir) pdrv->funcs->IoChdir = storageIoChdirCache;
	if (storageIoMount) pdrv->funcs->IoMount = storageIoMountCache;
	if (storageIoUmount) pdrv->funcs->IoUmount = storageIoUmountCache;
	if (storageIoDevctl) pdrv->funcs->IoDevctl = storageIoDevctlCache;
	if (storageIoUnk21) pdrv->funcs->IoUnk21 = storageIoUnk21Cache;

	// Return Success
	return 0;
}

void storageCacheStat(int reset) {
	#ifdef DEBUG
	// Output Buffer
	char buf[256];

	// Statistic available
	if (cacheReadTimes != 0) {
		sprintf(buf, "Mstor cache size: %dKB\n", g_cacheSize / 1024);
		sceIoWrite(1, buf, strlen(buf));
		sprintf(buf, "hit percent: %02d%%/%02d%%/%02d%%, [%d/%d/%d/%d]\n",
				(int)(100 * cacheHit / cacheReadTimes),
				(int)(100 * cacheMissed / cacheReadTimes),
				(int)(100 * cacheUncacheable / cacheReadTimes),
				(int)cacheHit, (int)cacheMissed, (int)cacheUncacheable, (int)cacheReadTimes);
		sceIoWrite(1, buf, strlen(buf));
		sprintf(buf, "caches stat:\n");
		sceIoWrite(1, buf, strlen(buf));
		sprintf(buf, "Cache Pos: 0x%08X bufSize: %d Buf: 0x%08X\n", (uint)g_cache.pos, g_cache.bufSize, (uint)g_cache.buf);
		sceIoWrite(1, buf, strlen(buf));


	// No Statistic available
	} else {
		sprintf(buf, "no storage cache call yet\n");
		sceIoWrite(1, buf, strlen(buf));
	}

	if (reset) {
		cacheReadTimes = cacheHit = cacheMissed = cacheUncacheable = 0;
	}
	#endif
}

void storageCacheDisable(void) {
	disableCache(&g_cache);
}

