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

#include <systemctrl.h>
#include <systemctrl_se.h>
#include <macros.h>

#include <adrenaline_log.h>
#include <iso_common.h>

#include <string.h>


// Iso filename
char g_iso_fn[256] = {0};
IoReadArg g_read_arg;
SceUID heapid = -1;
int g_iso_opened = 0;
SceUID g_iso_fd = -1;
int g_total_sectors = -1;

// Compressed block
static u8 *g_ciso_block_buf = NULL;
static u8 *g_ciso_block_buf_alloc_ptr = NULL;

// Decompressed block. size CISO_DEC_BUFFER_SIZE + (1 << g_CISO_hdr.align), align 64
static u8 *g_ciso_dec_buf = NULL;
static u8 *g_ciso_dec_buf_alloc_ptr = NULL;

// ISO sector
char* g_sector_buffer = NULL;

static u32 *g_cso_idx_cache = NULL;
static int g_cso_idx_start_block = -1;
static int g_cso_idx_cache_num = 0;

// reader data
static u32 header_size;
static u32 block_size;
static u32 uncompressed_size;
static u32 block_header;
static u32 align;
static u32 g_ciso_total_block;

// reader functions
static int is_compressed = 0;
static void (*ciso_decompressor)(void* src, u32 src_len, void* dst, u32 dst_len, u32 topbit) = NULL;
static int max_retries = 16;
static int o_flags = 0xF0000 | PSP_O_RDONLY;

static void wait_until_ms0_ready(void) {
	int ret, status = 0;

	if (sceKernelInitKeyConfig() == PSP_INIT_KEYCONFIG_VSH) {
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

static int read_raw_data(u8* addr, u32 size, u32 offset) {
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
			logmsg("%s: %s lseek retry %d error 0x%08X\n", __func__, g_iso_fn, i, (int)ofs);
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

/**
	The core of compressed iso reader.
	Abstracted to be compatible with all formats (CSO/ZSO/JSO/DAX).

	All compressed formats have the same overall structure:
	- A header followed by an array of block offsets (uint32).

	We only need to know the size of the header and some information from it.
	- block size: the size of a block once uncompressed.
	- uncompressed size: total size of the original (uncompressed) ISO file.
	- block header: size of block header if any (zlib header in DAX, JISO block_header, none for CSO/ZSO).
	- align: CISO block alignment (none for others).

	Some other Technical Information:
	- Block offsets can use the top bit to represent additional information for the decompressor (NCarea, compression method, etc).
	- Block size is calculated via the difference with the next block. Works for DAX, allowing us to skip parsing block size array (with correction for last block).
	- Non-Compressed Area can be determined if size of compressed block is equal to size of uncompressed (equal or greater for CSOv2 due to padding).
	- This reader won't work for CSO/ZSO files above 4GB to avoid using 64 bit arithmetic, but can be easily adjustable.
	- This reader can compile and run on PC and other platforms, as long as datatypes are properly defined and read_raw_data() is properly implemented.

	Includes IO Speed improvements:
	- a cache for block offsets, so we reduce block offset IO.
	- reading the entire compressed data (or chunks of it) at the end of provided buffer to reduce block IO.

*/
static int read_compressed_data(u8* addr, u32 size, u32 offset) {
	u32 o_offset = offset;
	u32 g_ciso_total_block = uncompressed_size/block_size;
	u8* com_buf = g_ciso_block_buf;
	u8* dec_buf = g_ciso_dec_buf;
	u8* c_buf = NULL;
	u8* top_addr = addr+size;

	#ifdef DEBUG
	io_calls = 0;
	#endif

	if (offset > uncompressed_size) {
		// return if the offset goes beyond the iso size
		return 0;
	} else if (offset + size > uncompressed_size) {
		// adjust size if it tries to read beyond the game data
		size = uncompressed_size - offset;
	}

	// IO speedup tricks
	u32 starting_block = o_offset / block_size;
	u32 ending_block = ((o_offset+size)/block_size);

	// refresh index table if needed
	if (g_cso_idx_start_block < 0 || starting_block < g_cso_idx_start_block || starting_block-g_cso_idx_start_block+1 >= g_cso_idx_cache_num-1){
		read_raw_data((u8*)g_cso_idx_cache, g_cso_idx_cache_num*sizeof(u32), starting_block * sizeof(u32) + header_size);
		g_cso_idx_start_block = starting_block;
	}

	// Calculate total size of compressed data
	u32 o_start = (g_cso_idx_cache[starting_block-g_cso_idx_start_block]&0x7FFFFFFF)<<align;
	// last block index might be outside the block offset cache, better read it from disk
	u32 o_end;
	if (ending_block-g_cso_idx_start_block < g_cso_idx_cache_num-1){
		o_end = g_cso_idx_cache[ending_block-g_cso_idx_start_block];
	} else {
		read_raw_data((u8*)&o_end, sizeof(u32), ending_block*sizeof(u32)+header_size); // read last two offsets
	}
	o_end = (o_end&0x7FFFFFFF)<<align;
	u32 compressed_size = o_end-o_start;

	logmsg4("%s: [DEBUG]: compressed size: %d, ", __func__, compressed_size);

	// try to read at once as much compressed data as possible
	if (size > block_size*2){ // only if going to read more than two blocks
		if (size < compressed_size) {
			compressed_size = size-block_size; // adjust chunk size if compressed data is still bigger than uncompressed
		}
		c_buf = top_addr - compressed_size; // read into the end of the user buffer
		read_raw_data(c_buf, compressed_size, o_start);
	}

	while (size > 0) {
		// calculate block number and offset within block
		u32 cur_block = offset / block_size;
		u32 pos = offset & (block_size - 1);

		// check if we need to refresh index table
		if (cur_block-g_cso_idx_start_block >= g_cso_idx_cache_num-1){
			read_raw_data((u8*)g_cso_idx_cache, g_cso_idx_cache_num*sizeof(u32), cur_block*sizeof(u32) + header_size);
			g_cso_idx_start_block = cur_block;
		}

		// read compressed block offset and size
		u32 b_offset = g_cso_idx_cache[cur_block-g_cso_idx_start_block];
		u32 b_size = g_cso_idx_cache[cur_block-g_cso_idx_start_block+1];
		u32 topbit = b_offset&0x80000000; // extract top bit for decompressor
		b_offset = (b_offset&0x7FFFFFFF) << align;
		b_size = (b_size&0x7FFFFFFF) << align;
		b_size -= b_offset;

		if (cur_block == g_ciso_total_block-1 && header_size == sizeof(DAXHeader)) {
			// fix for last DAX block (you can't trust the value of b_size since there's no offset for last_block+1)
			b_size = DAX_COMP_BUF;
		}

		// check if we need to (and can) read another chunk of data
		if (c_buf < addr || c_buf+b_size > top_addr){
			if (size > b_size+block_size){ // only if more than two blocks left, otherwise just use normal reading
				compressed_size = o_end-b_offset; // recalculate remaining compressed data
				if (size < compressed_size) {
					compressed_size = size-block_size; // adjust if still bigger than uncompressed
				}
				if (compressed_size >= b_size){
					c_buf = top_addr - compressed_size; // read into the end of the user buffer
					read_raw_data(c_buf, compressed_size, b_offset);
				}
			}
		}

		// read block, skipping header if needed
		if (c_buf >= addr && c_buf+b_size <= top_addr){
			memcpy(com_buf, c_buf+block_header, b_size); // fast read
			c_buf += b_size;
		} else { // slow read
			b_size = read_raw_data(com_buf, b_size, b_offset + block_header);
			if (c_buf) {
				c_buf += b_size;
			}
		}

		// decompress block
		ciso_decompressor(com_buf, b_size, dec_buf, block_size, topbit);

		// read data from block into buffer
		u32 read_bytes = MIN(size, (block_size - pos));
		memcpy(addr, dec_buf + pos, read_bytes);
		size -= read_bytes;
		addr += read_bytes;
		offset += read_bytes;
	}

	u32 res = offset - o_offset;

	#ifdef DEBUG
	logmsg4("read %d bytes at 0x%p took %d IO calls\n", res, o_offset, io_calls);
	#else
	logmsg4("read %d bytes at 0x%p\n", res, o_offset);
	#endif

	return res;
}

// Decompress DAX v0
static void decompress_dax0(void* src, u32 src_len, void* dst, u32 dst_len, u32 topbit){
	// use raw inflate with no NCarea check
	sceKernelDeflateDecompress(dst, dst_len, src, 0);
}

// Decompress DAX v1 or JISO method 1
static void decompress_dax1(void* src, u32 src_len, void* dst, u32 dst_len, u32 topbit){
	// for DAX Version 1 we can skip parsing NC-Areas and just use the block_size trick as in JSO and CSOv2
	if (src_len == dst_len) memcpy(dst, src, dst_len); // check for NC area
	else sceKernelDeflateDecompress(dst, dst_len, src, 0); // use raw inflate
}

// Decompress JISO method 0
static void decompress_jiso(void* src, u32 src_len, void* dst, u32 dst_len, u32 topbit){
	// while JISO allows for DAX-like NCarea, it by default uses compressed size check
	if (src_len == dst_len) memcpy(dst, src, dst_len); // check for NC area
	else sctrlLzoDecompress(dst, (unsigned*)&dst_len, src, src_len); // use lzo
}

// Decompress CISO v1
static void decompress_ciso(void* src, u32 src_len, void* dst, u32 dst_len, u32 topbit){
	if (topbit) memcpy(dst, src, dst_len); // check for NC area
	else sceKernelDeflateDecompress(dst, dst_len, src, 0);
}

// Decompress ZISO
static void decompress_ziso(void* src, u32 src_len, void* dst, u32 dst_len, u32 topbit){
	if (topbit) memcpy(dst, src, dst_len); // check for NC area
	else sctrlLZ4Decompress(dst, src, dst_len);
}

// Decompress CISO v2
static void decompress_cso2(void* src, u32 src_len, void* dst, u32 dst_len, u32 topbit){
	// in CSOv2, top bit represents compression method instead of NCarea
	if (src_len >= dst_len) memcpy(dst, src, dst_len); // check for NC area (JSO-like, but considering padding, thus >=)
	else if (topbit) sctrlLZ4Decompress(dst, src, dst_len);
	else sceKernelDeflateDecompress(dst, dst_len, src, 0);
}

#ifndef __ISO_EXTRA__
static
#endif // __ISO_EXTRA__
int iso_alloc(u32 com_size) {
	#ifdef __USE_USER_ALLOC
	if (is_compressed) {
		// allocate buffer for decompressed block
		g_ciso_dec_buf = user_malloc(com_size+64);
		if (g_ciso_dec_buf == NULL) {
			return -2;
		}
		g_ciso_dec_buf_alloc_ptr = g_ciso_dec_buf;
		if ((u32)g_ciso_dec_buf & 63) {
			// align 64
			g_ciso_dec_buf = (void*)(((u32)g_ciso_dec_buf & (~63)) + 64);
		}
		// allocate buffer for compressed block
		g_ciso_block_buf = user_malloc(com_size+64);
		if (g_ciso_block_buf == NULL) {
			return -3;
		}
		g_ciso_block_buf_alloc_ptr = g_ciso_block_buf;
		if ((u32)g_ciso_block_buf & 63) {
			// align 64
			g_ciso_block_buf = (void*)(((u32)g_ciso_block_buf & (~63)) + 64);
		}
		// allocate buffer for block offset cache
		g_cso_idx_cache = user_malloc((g_cso_idx_cache_num * 4) + 64);
		if (g_cso_idx_cache == NULL) {
			return -4;
		}
	}
	// allocate iso sector buffer
	g_sector_buffer = user_malloc(ISO_SECTOR_SIZE);
	if (g_sector_buffer == NULL) {
		return -6;
	}
	#else
	// lets use our own heap so that kram usage depends on game format (less heap needed for systemcontrol; better memory management)
	if (heapid < 0) {
		heapid = sceKernelCreateHeap(PSP_MEMORY_PARTITION_KERNEL, (2*com_size) + (g_cso_idx_cache_num * 4) + 128, 1, "InfernoHeap");
		if (heapid < 0) {
			return -5;
		}
		if (is_compressed) {
			// allocate buffer for decompressed block
			g_ciso_dec_buf = sceKernelAllocHeapMemory(heapid, com_size+64);
			if (g_ciso_dec_buf == NULL) {
				return -2;
			}
			g_ciso_dec_buf_alloc_ptr = g_ciso_dec_buf;
			if ((u32)g_ciso_dec_buf & 63) {
				// align 64
				g_ciso_dec_buf = (void*)(((u32)g_ciso_dec_buf & (~63)) + 64);
			}
			// allocate buffer for compressed block
			g_ciso_block_buf = sceKernelAllocHeapMemory(heapid, com_size+64);
			if (g_ciso_block_buf == NULL) {
				return -3;
			}
			g_ciso_block_buf_alloc_ptr = g_ciso_block_buf;
			if ((u32)g_ciso_block_buf & 63) {
				// align 64
				g_ciso_block_buf = (void*)(((u32)g_ciso_block_buf & (~63)) + 64);
			}
			// allocate buffer for block offset cache
			g_cso_idx_cache = sceKernelAllocHeapMemory(heapid, (g_cso_idx_cache_num * 4) + 64);
			if (g_cso_idx_cache == NULL) {
				return -4;
			}
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

static void iso_free() {
	#ifdef __USE_USER_ALLOC
	if (is_compressed) {
		user_free(g_ciso_dec_buf_alloc_ptr);
		user_free(g_ciso_block_buf_alloc_ptr);
		user_free(g_cso_idx_cache);
	}
	user_free(g_sector_buffer);
	#else
	if (is_compressed) {
		sceKernelFreeHeapMemory(heapid, g_ciso_dec_buf_alloc_ptr);
		sceKernelFreeHeapMemory(heapid, g_ciso_block_buf_alloc_ptr);
		sceKernelFreeHeapMemory(heapid, g_cso_idx_cache);
	}
	sceKernelFreeHeapMemory(heapid, g_sector_buffer);
	sceKernelDeleteHeap(heapid);
	#endif // __USE_USER_ALLOC
	heapid = -1;
	g_ciso_dec_buf_alloc_ptr = NULL;
	g_ciso_dec_buf = NULL;
	g_ciso_block_buf_alloc_ptr = NULL;
	g_ciso_block_buf = NULL;
	g_cso_idx_cache = NULL;
	g_sector_buffer = NULL;
}

#ifndef __ISO_EXTRA__
static
#endif // __ISO_EXTRA__
int iso_type_check(SceUID fd) {
	CISOHeader g_CISO_hdr;

	g_CISO_hdr.magic = 0;

	sceIoLseek(fd, 0, PSP_SEEK_SET);
	int ret = sceIoRead(fd, &g_CISO_hdr, sizeof(g_CISO_hdr));

	if (ret != sizeof(g_CISO_hdr)) {
		return -1;
	}

	u32 magic = g_CISO_hdr.magic;

	if (magic == CSO_MAGIC || magic == ZSO_MAGIC || magic == DAX_MAGIC || magic == JSO_MAGIC) { // CISO or ZISO or JISO or DAX
		u32 com_size = 0;
		// set reader and decompressor functions according to format
		if (magic == DAX_MAGIC){
			DAXHeader* dax_header = (DAXHeader*)&g_CISO_hdr;
			header_size = sizeof(DAXHeader);
			block_size = DAX_BLOCK_SIZE; // DAX uses static block size (8K)
			uncompressed_size = dax_header->uncompressed_size;
			block_header = 2; // skip over the zlib header (2 bytes)
			align = 0; // no alignment for DAX
			com_size = DAX_COMP_BUF;
			ciso_decompressor = (dax_header->version >= 1)? &decompress_dax1 : &decompress_dax0;
		}
		else if (magic == JSO_MAGIC){
			JISOHeader* jiso_header = (JISOHeader*)&g_CISO_hdr;
			header_size = sizeof(JISOHeader);
			block_size = jiso_header->block_size;
			uncompressed_size = jiso_header->uncompressed_size;
			block_header = 4*jiso_header->block_headers; // if set to 1, each block has a 4 byte header, 0 otherwise
			align = 0; // no alignment for JISO
			com_size = jiso_header->block_size;
			ciso_decompressor = (jiso_header->method)? &decompress_dax1 : &decompress_jiso; //  zlib or lzo, depends on method
		}
		else{ // CSO/ZSO/v2
			header_size = sizeof(CISOHeader);
			block_size = g_CISO_hdr.block_size;
			uncompressed_size = g_CISO_hdr.total_bytes;
			block_header = 0; // CSO/ZSO uses raw blocks
			align = g_CISO_hdr.align;
			com_size = block_size + (1 << g_CISO_hdr.align);
			if (g_CISO_hdr.ver == 2) ciso_decompressor = &decompress_cso2; // CSOv2 uses both zlib and lz4
			else ciso_decompressor = (magic == ZSO_MAGIC)? &decompress_ziso : &decompress_ciso; // CSO/ZSO v1 (zlib or lz4)
		}
		g_total_sectors = uncompressed_size / ISO_SECTOR_SIZE; // total number of DVD sectors (2K) in the original ISO.
		g_ciso_total_block = uncompressed_size / block_size;
		// for files with higher block sizes, we can reduce block cache size
		int ratio = block_size/ISO_SECTOR_SIZE;
		g_cso_idx_cache_num = CISO_IDX_MAX_ENTRIES/ratio;

		is_compressed = 1;
		int alloc_res = iso_alloc(com_size);

		if (alloc_res < 0) {
			return alloc_res;
		}

		return 1;
	} else {
		SceOff off = sceIoLseek(g_iso_fd, 0, PSP_SEEK_CUR);
		SceOff total = sceIoLseek(g_iso_fd, 0, PSP_SEEK_END);
		sceIoLseek(g_iso_fd, off, PSP_SEEK_SET);

		is_compressed = 0;
		int alloc_res = iso_alloc(DAX_BLOCK_SIZE);

		if (alloc_res < 0) {
			return alloc_res;
		}

		g_total_sectors = total / ISO_SECTOR_SIZE;
		return 0;
	}
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

	is_compressed = iso_type_check(g_iso_fd);


	if (is_compressed < 0) {
		return is_compressed;
	}

	g_iso_opened = 1;

	return 0;
}

int iso_read(IoReadArg *args) {
	if (is_compressed) {
		return read_compressed_data(args->address, args->size, args->offset);
	}
	return read_raw_data(args->address, args->size, args->offset);
}

void iso_close() {
	sceIoClose(g_iso_fd);
	g_iso_fd = -1;
	g_iso_opened = 0;

	iso_free();

	g_total_sectors = 0;
	g_cso_idx_start_block = -1;
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

int isoGetGameId(char game_id[10]) {
	// game ID is always at offset 0x8373 within the ISO
	// lba=16, offset=883
	u32 pos = 16 * ISO_SECTOR_SIZE + 883;
	IoReadArg read_arg = {
		.address = (u8*)game_id,
		.size = 10,
		.offset = pos
	};
	int res = iso_read(&read_arg);
	if (res != 10) {
		return 0;
	}

	// remove the dash in the middle: ULUS-01234 -> ULUS01234
	game_id[4] = game_id[5];
	game_id[5] = game_id[6];
	game_id[6] = game_id[7];
	game_id[7] = game_id[8];
	game_id[8] = game_id[9];
	game_id[9] = 0;
	return 1;
}