/*
	Adrenaline
	Copyright (C) 2025, GrayJack
	Copyright (C) 2021, PRO CFW

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

#ifndef __ISO_COMMON_H
#define __ISO_COMMON_H

#define CSO_MAGIC 0x4F534943 // CISO
#define ZSO_MAGIC 0x4F53495A // ZISO
#define DAX_MAGIC 0x00584144 // DAX
#define JSO_MAGIC 0x4F53494A // JISO

#define DAX_BLOCK_SIZE 0x2000
#define DAX_COMP_BUF 0x2400

#define ISO_SECTOR_SIZE 2048

#define CISO_IDX_BUFFER_SIZE 0x200
#define CISO_DEC_BUFFER_SIZE 0x2000
#define CISO_IDX_MAX_ENTRIES 2048 // will be adjusted according to CSO block_size

#define MAX_FILES_NR 8

typedef struct _CISOHeader {
    uint32_t magic;  // 0
    u32 header_size;  // 4
    u64 total_bytes; // 8
    u32 block_size; // 16
    u8 ver; // 20
    u8 align;  // 21
    u8 rsv_06[2];  // 22
} CISOHeader;

typedef struct _DAXHeader{
    uint32_t magic;
    uint32_t uncompressed_size;
    uint32_t version;
    uint32_t nc_areas;
    uint32_t unused[4];
} DAXHeader;

typedef struct _JISOHeader {
    uint32_t magic; // [0x000] 'JISO'
    uint8_t unk_x001; // [0x004] 0x03?
    uint8_t unk_x002; // [0x005] 0x01?
    uint16_t block_size; // [0x006] Block size, usually 2048.
    // TODO: Are block_headers and method 8-bit or 16-bit?
    uint8_t block_headers; // [0x008] Block headers. (1 if present; 0 if not.)
    uint8_t unk_x009; // [0x009]
    uint8_t method; // [0x00A] Method. (See JisoAlgorithm_e.)
    uint8_t unk_x00b; // [0x00B]
    uint32_t uncompressed_size; // [0x00C] Uncompressed data size.
    uint8_t md5sum[16]; // [0x010] MD5 hash of the original image.
    uint32_t header_size; // [0x020] Header size? (0x30)
    uint8_t unknown[12]; // [0x024]
} JISOHeader;

typedef enum {
    JISO_METHOD_LZO    	= 0,
    JISO_METHOD_ZLIB    = 1,
} JisoMethod;

typedef struct IoReadArg {
	u32 offset; // 0
	u8 *address; // 4
	u32 size; // 8
} IoReadArg;

extern char g_iso_fn[256];
extern SceUID g_iso_fd;
extern int g_iso_opened;
extern int g_total_sectors;
extern IoReadArg g_read_arg;
extern char* g_sector_buffer;

int iso_read(IoReadArg *args);
int iso_open(void);
void iso_close(void);

int isoGetGameId(char game_id[10]);

#ifdef __ISO_EXTRA__
int iso_type_check(SceUID fd);
int iso_alloc(u32 com_size);
void iso_free();
int iso_re_open(void);
#endif // __ISO_EXTRA__

#endif // __ISO_COMMON_H