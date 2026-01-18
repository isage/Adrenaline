/*
	Adrenaline ISO Common
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

#include <psptypes.h>
#include <psputilsforkernel.h>
#include <psperror.h>
#include <pspsysmem_kernel.h>

#include <systemctrl.h>
#include <systemctrl_se.h>
#include <cfwmacros.h>

#ifdef __USE_USER_ALLOC
#include <vshctrl.h>
#endif

#include <adrenaline_log.h>
#include "iso_common.h"

#include <string.h>


// Iso filename
char g_iso_fn[256] = {0};
IoReadArg g_read_arg;
SceUID heapid = -1;
int g_iso_opened = 0;
SceUID g_iso_fd = -1;
int g_total_sectors = -1;

// ISO sector
char* g_sector_buffer = NULL;

// reader functions
static int is_compressed = 0;
static int max_retries = 16;
static int o_flags = 0xF0000 | PSP_O_RDONLY;

static int read_raw_data(void* arg, u8* addr, u32 size, u32 offset);

// ciso data
CisoFile g_ciso_file = {
    .read_data = (void*)&read_raw_data,
	#ifdef __USE_USER_ALLOC
	.memalign = &user_memalign,
    .free = &user_free,
	#else
    .memalign = &oe_memalign,
    .free = &oe_free,
	#endif
};

// for libcisoread
int zlib_inflate(void* dst, int dst_len, void* src){
    return sceKernelDeflateDecompress(dst, dst_len, src, NULL);
}

static void wait_until_ms0_ready(void) {
	int ret, status = 0;

	if (sceKernelApplicationType() == PSP_INIT_KEYCONFIG_VSH) {
		o_flags = PSP_O_RDONLY;
		max_retries = 10;
		return; // no wait on VSH
	}

	const char *drvname = (
		(g_iso_fn[0] == 'm' || g_iso_fn[0] == 'M') &&
		(g_iso_fn[1] == 's' || g_iso_fn[1] == 'S')
	)?  "mscmhc0:" : "mscmhcemu0:";

	while (1) {
		ret = sceIoDevctl(drvname, 0x02025801, 0, 0, &status, sizeof(status));

		if (ret < 0){
			sceKernelDelayThread(20000);
			continue;
		}

		if (status == 4) {
			break;
		}

		sceKernelDelayThread(20000);
	}
}

#ifdef DEBUG
static int io_calls = 0;
#endif

static int read_raw_data(void* arg, u8* addr, u32 size, u32 offset) {
	#ifdef DEBUG
	io_calls++;
	#endif

	SceOff ofs;
	int i = 0;

	for (i = 0; i < max_retries; ++i) {
		ofs = sceIoLseek(g_iso_fd, offset, PSP_SEEK_SET);

		if (ofs >= 0) {
			i = 0;
			break;
		} else {
			logmsg("%s: %s(%d) lseek retry %d error 0x%08X\n", __func__, g_iso_fn, g_iso_fd, i, (int)ofs);
			iso_open();
		}

		#ifdef __READ_RAW_DELAY_THREAD
		sceKernelDelayThread(100000);
		#endif // __READ_RAW_DELAY_THREAD
	}

	int ret = 0;
	if (i == max_retries) {
		ret = SCE_ENODEV;
		goto exit;
	}

	for (i = 0; i < max_retries; ++i) {
		ret = sceIoRead(g_iso_fd, addr, size);

		if (ret >= 0) {
			i = 0;
			break;
		} else {
			logmsg("%s: %s read retry %d error 0x%08X\n", __func__, g_iso_fn, i, ret);
			iso_open();
			sceIoLseek(g_iso_fd, offset, PSP_SEEK_SET);
		}

		#ifdef __READ_RAW_DELAY_THREAD
		sceKernelDelayThread(100000);
		#endif // __READ_RAW_DELAY_THREAD
	}

	if (i == max_retries) {
		ret = SCE_ENODEV;
		goto exit;
	}

exit:
	return ret;
}

#ifndef __ISO_EXTRA__
static
#endif // __ISO_EXTRA__
int iso_alloc() {
	#ifdef __USE_USER_ALLOC
	// allocate iso sector buffer
	g_sector_buffer = user_malloc(ISO_SECTOR_SIZE);
	if (g_sector_buffer == NULL) {
		return -6;
	}
	#else
	// lets use our own heap so that kram usage depends on game format (less heap needed for systemcontrol; better memory management)
	if (heapid < 0) {
		heapid = sceKernelCreateHeap(PSP_MEMORY_PARTITION_KERNEL, ISO_SECTOR_SIZE+64, 1, "InfernoHeap");
		if (heapid < 0) {
			return -5;
		}

		// allocate iso sector buffer
		g_sector_buffer = sceKernelAllocHeapMemory(heapid, ISO_SECTOR_SIZE);
		if (g_sector_buffer == NULL) {
			return -6;
		}
	}
	#endif // __USE_USER_ALLOC

	return 0;
}

#ifndef __ISO_EXTRA__
static
#endif // __ISO_EXTRA__
void iso_free() {
	#ifdef __USE_USER_ALLOC
	if (is_compressed) {
		ciso_close(&g_ciso_file);
	}
	user_free(g_sector_buffer);
	#else
	if (is_compressed) {
		ciso_close(&g_ciso_file);
	}
	sceKernelFreeHeapMemory(heapid, g_sector_buffer);
	sceKernelDeleteHeap(heapid);
	#endif // __USE_USER_ALLOC
	heapid = -1;
	g_sector_buffer = NULL;
}

int iso_open(void) {
	if (g_iso_fn[0] == 0) {
		return SCE_EINVAL;
	}

	wait_until_ms0_ready();
	sceIoClose(g_iso_fd);
	g_iso_opened = 0;

	int retries = 0;
	do {
		g_iso_fd = sceIoOpen(g_iso_fn, o_flags, 0777);

		if (g_iso_fd < 0) {
			if (++retries >= max_retries) {
				return g_iso_fd;
			}

			sceKernelDelayThread(20000);
		}
	} while (g_iso_fd < 0);

	if (g_iso_fd < 0) {
		return g_iso_fd;
	}

	g_ciso_file.reader_arg = (void*)g_iso_fd;

    is_compressed = ciso_open(&g_ciso_file);

	if (is_compressed < 0) return is_compressed;
    // total number of DVD sectors (2K) in the original ISO.
    else if (is_compressed > 0){
        g_total_sectors = g_ciso_file.uncompressed_size / ISO_SECTOR_SIZE;
    }
    else {
        SceOff off = sceIoLseek(g_iso_fd, 0, PSP_SEEK_CUR);
        SceOff total = sceIoLseek(g_iso_fd, 0, PSP_SEEK_END);
        sceIoLseek(g_iso_fd, off, PSP_SEEK_SET);
        g_total_sectors = total / ISO_SECTOR_SIZE;
    }

	int alloc_res = iso_alloc();
	if (alloc_res < 0) {
		return alloc_res;
	}

	g_iso_opened = 1;

	return 0;
}

int iso_read(IoReadArg *args) {
	if (is_compressed) {
		return ciso_read(&g_ciso_file, args->address, args->size, args->offset);
	}
	return read_raw_data(NULL, args->address, args->size, args->offset);
}

void iso_close() {
	sceIoClose(g_iso_fd);
	g_iso_fd = -1;
	g_iso_opened = 0;

	iso_free();

	g_total_sectors = 0;
	is_compressed = 0;
	memset(g_iso_fn, 0, 255);
}

#ifdef __ISO_EXTRA__
int iso_re_open(void) {
	int retries = max_retries;
	int fd = -1;

	sceIoClose(g_iso_fd);

	while (retries -- > 0) {
		fd = sceIoOpen(g_iso_fn, o_flags, 0777);

		if (fd >= 0) {
			break;
		}

		sceKernelDelayThread(100000);
	}

	if (fd >= 0) {
		g_iso_fd = fd;
	}

	return fd;
}
#endif // __ISO_EXTRA__

int isoGetTitleId(char title_id[10]) {
	// game ID is always at offset 0x8373 within the ISO
	// lba=16, offset=883
	u32 pos = 16 * ISO_SECTOR_SIZE + 883;
	IoReadArg read_arg = {
		.address = (u8*)title_id,
		.size = 10,
		.offset = pos
	};
	int res = iso_read(&read_arg);
	if (res != 10) {
		return 0;
	}

	// remove the dash in the middle: ULUS-01234 -> ULUS01234
	title_id[4] = title_id[5];
	title_id[5] = title_id[6];
	title_id[6] = title_id[7];
	title_id[7] = title_id[8];
	title_id[8] = title_id[9];
	title_id[9] = 0;
	return 1;
}