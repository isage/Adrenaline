/*
	Adrenaline Galaxy Controller
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

#ifndef _GALAXY_H_
#define _GALAXY_H_

#include <pspsdk.h>
#include <stdio.h>

// Read Argument for Kernel Extend Stack
typedef struct IoReadArg {
	u32 offset;
	u8 *address;
	u32 size;
} IoReadArg;

// CSO Header
typedef struct CISO_header {
	unsigned char magic[4];
	unsigned int header_size;
	uint64_t total_bytes;
	unsigned int block_size;
	unsigned char ver;
	unsigned char align;
	unsigned char rsv_06[2];
} CISO_header;

#define NP9660_DATA_1 (0x00005BB4 - 0x00005BA4 + 0x00000188 + 0x00008900)
#define NP9660_DATA_2 (0x00005BBC - 0x00005BA4 + 0x00000188 + 0x00008900)
#define NP9660_DATA_3 (0x00005BD0 - 0x00005BA4 + 0x00000188 + 0x00008900)
#define NP9660_DATA_4 (0x00005BD8 - 0x00005BA4 + 0x00000188 + 0x00008900)
#define NP9660_DATA_5 (0x00000114 + 0x00008900)
#define NP9660_ISO_FD (0x00000188 + 0x00008900)

// ISO Sector Size
#define ISO_SECTOR_SIZE 0x800

// CSO Buffer Size
#define CISO_IDX_BUFFER_SIZE 0x200
#define CISO_DEC_BUFFER_SIZE 0x2000

// Get Disc Sector Count
int get_total_block(void);

// Read Raw ISO Data
int read_raw_data(unsigned char * addr, unsigned int size, unsigned int offset);

// Read CSO Disc Sector
int read_cso_sector(unsigned char * addr, int sector);

// Custom CSO Sector Reader
int read_cso_data(unsigned char * addr, unsigned int size, unsigned int offset);

// Custom ISO Sector Reader Wrapper
int iso_read(struct IoReadArg * args);

// Open CSO File (subcall from open_iso)
int cso_open(SceUID fd);

// Open ISO File
int open_iso(void);

// Init ISO Emulator
int initEmulator(void);

SceUID sceKernelCreateThreadPatched(const char * name, SceKernelThreadEntry entry, int initPriority, int stackSize, SceUInt attr, SceKernelThreadOptParam * option);
int readDiscSectorNP9660Patched(unsigned int sector, unsigned char * buffer, unsigned int size);
int sceIoClosePatched(int fd);
int sceKernelStartThreadPatched(SceUID thid, SceSize arglen, void * argp);

#endif // _GALAXY_H_

