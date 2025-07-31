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

#include <pspkernel.h>
#include <pspiofilemgr.h>
#include <pspthreadman_kernel.h>
#include <psputilsforkernel.h>

#include <stdio.h>
#include <string.h>

#include <systemctrl.h>
#include <systemctrl_se.h>
#include <macros.h>

#define _ADRENALINE_LOG_IMPL_
#include <adrenaline_log.h>

#include "galaxy.h"

PSP_MODULE_INFO("EPIGalaxyController", 0x1006, 1, 0);

// SceNpUmdMount Thread ID
SceUID g_SceNpUmdMount_thid = -1;

// ISO File Descriptor
SceUID g_iso_fd = -1;

// ISO Block Count
int g_total_blocks = -1;

// ISO Open Flag
int g_iso_opened = 0;

// CSO Mode Flag
int g_is_ciso = 0;

// CSO Block Buffer
void * g_ciso_block_buf = NULL;

// CSO Decompress Buffer
void * g_ciso_dec_buf = NULL;

// CSO Index Cache
unsigned int g_CISO_idx_cache[CISO_IDX_BUFFER_SIZE / 4] __attribute__((aligned(64)));

// CSO Decompress Buffer Offset
unsigned int g_ciso_dec_buf_offset = (unsigned int)-1;

// CSO Decompress Buffer Size
int g_ciso_dec_buf_size = 0;

// CSO Current Index
int g_CISO_cur_idx = 0;

// CSO Header
static CISO_header g_CISO_hdr __attribute__((aligned(64)));

// CSO Block Count
unsigned int ciso_total_block = 0;

// np9660.prx Text Address
unsigned int g_sceNp9660_driver_text_addr = 0;

// ISO File Path
char * g_iso_fn = NULL;

// Dummy UMD Data for Global Pointer
unsigned char g_umddata[16] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

// NP9660 Initialization Call
void (* initNP9660)(unsigned int zero) = NULL;

// NP9660 Disc Sector Reader
int (* readDiscSectorNP9660)(unsigned int sector, unsigned char * buffer, unsigned int size) = NULL;

// NP9660 Sector Flush
void (* readSectorFlushNP9660)(void) = NULL;

// Set Global Pointer
int sceKernelSetQTGP3(void *data);

u32 findRefInGlobals(char* libname, u32 addr, u32 ptr) {
	while (strcmp(libname, (char*)addr)) {
		addr++;
	}

	if (addr%4) {
		addr &= -0x4; // align to 4 bytes
	}

	while (VREAD32(addr += 4) != ptr);

	return addr;
}

// Get Disc Sector Count
int get_total_block(void) {
	if(g_is_ciso) {
		// Return CSO Header Value
		return ciso_total_block;
	}

	SceOff offset = sceIoLseek(g_iso_fd, 0, PSP_SEEK_END);

	if(offset < 0) {
		return (int)offset;
	}

	// Return Sector Count
	return offset / ISO_SECTOR_SIZE;
}

// Read Raw ISO Data
int read_raw_data(unsigned char * addr, unsigned int size, unsigned int offset) {
	// Retry Counter
	int i = 0;

seek:
	// Seek to Position
	for (i = 0; i < 16; i++) {
		SceOff ofs = sceIoLseek(g_iso_fd, offset, PSP_SEEK_SET);

		if(ofs >= 0) {
			break;
		}

		open_iso();
	}

	if (i == 16) {
		return 0x80010013;
	}

	int read = sceIoRead(g_iso_fd, addr, size);

	if (read < 0) {
		// Reopen ISO File
		open_iso();

		// Retry Seeking
		goto seek;
	}

	return read;
}

// Read CSO Disc Sector
int read_cso_sector(unsigned char * addr, int sector) {
	// Negative Sector
	int n_sector = sector - g_CISO_cur_idx;

	if (g_CISO_cur_idx == -1 || n_sector < 0 || n_sector >= NELEMS(g_CISO_idx_cache)) {
		int read = read_raw_data((unsigned char *)g_CISO_idx_cache, sizeof(g_CISO_idx_cache), (sector << 2) + sizeof(CISO_header));

		if(read < 0) {
			return -4;
		}

		// Set Current Sector Index
		g_CISO_cur_idx = sector;

		n_sector = 0;
	}

	unsigned int offset = (g_CISO_idx_cache[n_sector] & 0x7FFFFFFF) << g_CISO_hdr.align;

	if (g_CISO_idx_cache[n_sector] & 0x80000000) {
		return read_raw_data(addr, ISO_SECTOR_SIZE, offset);
	}

	sector++;

	n_sector = sector - g_CISO_cur_idx;

	if (g_CISO_cur_idx == -1 || n_sector < 0 || n_sector >= NELEMS(g_CISO_idx_cache)) {
		int read = read_raw_data((unsigned char *)g_CISO_idx_cache, sizeof(g_CISO_idx_cache), (sector << 2) + sizeof(CISO_header));

		if(read < 0) {
			return -5;
		}

		g_CISO_cur_idx = sector;

		n_sector = 0;
	}

	unsigned int next_offset = (g_CISO_idx_cache[n_sector] & 0x7FFFFFFF) << g_CISO_hdr.align;
	unsigned int size = next_offset - offset;

	if(g_CISO_hdr.align) {
		size += 1 << g_CISO_hdr.align;
	}

	// Set Minimum Size
	if (size <= ISO_SECTOR_SIZE) {
		size = ISO_SECTOR_SIZE;
	}

	// Packed Payload required
	if (g_ciso_dec_buf_offset == (unsigned int)-1 || offset < g_ciso_dec_buf_offset || offset + size >= g_ciso_dec_buf_offset + g_ciso_dec_buf_size) {
		int read = read_raw_data(g_ciso_dec_buf, size, offset);

		if (read < 0) {
			g_ciso_dec_buf_offset = 0xFFF00000;
			return -6;
		}

		g_ciso_dec_buf_offset = offset;
		g_ciso_dec_buf_size = read;
	}

	// Unpack Data
	int read = sceKernelDeflateDecompress(addr, ISO_SECTOR_SIZE, g_ciso_dec_buf + offset - g_ciso_dec_buf_offset, 0);

	return read < 0 ? read : ISO_SECTOR_SIZE;
}

// Custom CSO Sector Reader
int read_cso_data(unsigned char * addr, unsigned int size, unsigned int offset) {
	// Backup Start Sector Offset
	unsigned int o_offset = offset;

	while (size > 0) {
		unsigned int cur_block = offset / ISO_SECTOR_SIZE;
		unsigned int pos = offset & (ISO_SECTOR_SIZE - 1);

		if(cur_block >= g_total_blocks) {
			break;
		}

		int read = read_cso_sector(g_ciso_block_buf, cur_block);

		if (read != ISO_SECTOR_SIZE) {
			return -7;
		}

		read = MIN(size, (ISO_SECTOR_SIZE - pos));
		memcpy(addr, g_ciso_block_buf + pos, read);

		size -= read;
		addr += read;
		offset += read;
	}

	return offset - o_offset;
}

// Custom ISO Sector Reader Wrapper
int iso_read(IoReadArg * args) {
	if (g_is_ciso != 0) {
		return read_cso_data(args->address, args->size, args->offset);
	} else {
		return read_raw_data(args->address, args->size, args->offset);
	}
}

int cso_open(SceUID fd) {
	g_CISO_hdr.magic[0] = 0;
	g_ciso_dec_buf_offset = (unsigned int)-1;
	g_ciso_dec_buf_size = 0;

	// Rewind file
	sceIoLseek(fd, 0, PSP_SEEK_SET);

	if(sceIoRead(fd, &g_CISO_hdr, sizeof(g_CISO_hdr)) != sizeof(g_CISO_hdr)) {
		return -1;
	}

	if (VREAD32((unsigned int)g_CISO_hdr.magic) != 0x4F534943) {
		return 0x8002012F;
	}

	g_CISO_cur_idx = -1;

	ciso_total_block = g_CISO_hdr.total_bytes / g_CISO_hdr.block_size;

	// Decompression Buffer not yet created
	if (g_ciso_dec_buf == NULL) {
		g_ciso_dec_buf = oe_malloc(CISO_DEC_BUFFER_SIZE + (1 << g_CISO_hdr.align) + 64);

		if (g_ciso_dec_buf == NULL) {
			return -2;
		}

		// Alignment required
		if (((unsigned int)g_ciso_dec_buf & 63) != 0) {
			g_ciso_dec_buf = (void *)(((unsigned int)g_ciso_dec_buf & (~63)) + 64);
		}
	}

	// Sector Buffer not yet created
	if (g_ciso_block_buf == NULL) {
		g_ciso_block_buf = oe_malloc(ISO_SECTOR_SIZE);
		if(g_ciso_block_buf == NULL) {
			return -3;
		}
	}

	return 0;
}

// Open ISO File
int open_iso(void) {
	sceIoClose(g_iso_fd);

	g_iso_opened = 0;

	while(1) {
		g_iso_fd = sceIoOpen(g_iso_fn, 0xF0000 | PSP_O_RDONLY, 0);

		if(g_iso_fd >= 0) {
			break;
		}

		sceKernelDelayThread(10000);
	}

	// Update NP9660 File Descriptor Variable
	VWRITE32(g_sceNp9660_driver_text_addr + NP9660_ISO_FD, g_iso_fd);

	g_is_ciso = 0;

	if (cso_open(g_iso_fd) >= 0) {
		g_is_ciso = 1;
	}

	g_total_blocks = get_total_block();
	g_iso_opened = 1;

	return 0;
}

int readDiscSectorNP9660Patched(unsigned int sector, unsigned char * buffer, unsigned int size) {
	// This will fail but it sets some needed variables
	readDiscSectorNP9660(sector, buffer, size);

	// Prepare Arguments for our custom ISO Sector Reader
	IoReadArg args;
	args.offset = sector;
	args.address = buffer;
	args.size = size;

	int res = sceKernelExtendKernelStack(0x2000, (void *)iso_read, &args);

	// Flush & Update NP9660 Sector Information
	readSectorFlushNP9660();

	return res;
}

int sceIoClosePatched(int fd) {
	int res = sceIoClose(fd);

	if(fd == g_iso_fd) {
		g_iso_fd = -1;

		// Remove File Descriptor
		VWRITE32(g_sceNp9660_driver_text_addr + NP9660_ISO_FD, -1);

		sctrlFlushCache();
	}

	return res;
}

// Init ISO Emulator
int initEmulator(void) {
	static int firstrun = 1;

	initNP9660(0);
	open_iso();

	// Suspend interrupts
	int interrupts = sceKernelCpuSuspendIntr();

	// sceUmdManGetUmdDiscInfo Patch
	VWRITE32(g_sceNp9660_driver_text_addr + NP9660_DATA_1, 0xE0000800);
	VWRITE32(g_sceNp9660_driver_text_addr + NP9660_DATA_2, 9);
	VWRITE32(g_sceNp9660_driver_text_addr + NP9660_DATA_3, g_total_blocks);
	VWRITE32(g_sceNp9660_driver_text_addr + NP9660_DATA_4, g_total_blocks);
	VWRITE32(g_sceNp9660_driver_text_addr + NP9660_DATA_5, 0);

	// Resume interrupts
	sceKernelCpuResumeIntr(interrupts);

	// First run delay
	if (firstrun) {
		sceKernelDelayThread(800000);
		firstrun = 0;
	}

	sctrlFlushCache();

	// Set global ptr for UMD Data
	sceKernelSetQTGP3(g_umddata);

	return 0;
}

int sceKernelStartThreadPatched(SceUID thid, SceSize arglen, void * argp) {
	// SceNpUmdMount Thread
	if(g_SceNpUmdMount_thid == thid) {
		SceModule2 * pMod = sceKernelFindModuleByName("sceNp9660_driver");
		g_sceNp9660_driver_text_addr = pMod->text_addr;

		// Make InitForKernel Call return 0x80000000 always
		MAKE_INSTRUCTION(g_sceNp9660_driver_text_addr + 0x3C5C, 0x3C028000); // jal InitForKernel_23458221 to lui $v0, 0x00008000

		// Hook Function #1
		MAKE_CALL(g_sceNp9660_driver_text_addr + 0x3C78, initEmulator);

		// Hook Function #2
		MAKE_CALL(g_sceNp9660_driver_text_addr + 0x4414, readDiscSectorNP9660Patched); // jal sub_3948 to jal sub_00000054
		MAKE_CALL(g_sceNp9660_driver_text_addr + 0x596C, readDiscSectorNP9660Patched); // jal sub_3948 to jal sub_00000054

		// Hook sceIoClose Stub
		MAKE_JUMP(g_sceNp9660_driver_text_addr + 0x7D68, sceIoClosePatched); // hook sceIoClose import

		// Backup Function Pointer
		initNP9660 = (void *)(pMod->text_addr + 0x36A8);
		readDiscSectorNP9660 = (void *)(pMod->text_addr + 0x4FEC);
		readSectorFlushNP9660 = (void *)(pMod->text_addr + 0x505C);

		sctrlFlushCache();
	}

	return sceKernelStartThread(thid, arglen, argp);
}

SceUID sceKernelCreateThreadPatched(const char * name, SceKernelThreadEntry entry, int initPriority, int stackSize, SceUInt attr, SceKernelThreadOptParam * option) {
	SceUID thid = sceKernelCreateThread(name, entry, initPriority, stackSize, attr,	option);

	if(strncmp(name, "SceNpUmdMount", 13) == 0) {
		// Save Thread ID
		g_SceNpUmdMount_thid = thid;
	}

	return thid;
}

u32 findReferenceInGlobalScope(u32 ref){
	u32 addr = ref;
	while (strcmp("ThreadManForKernel", (char*)addr)) {
		addr++;
	}
	addr+=18;

	if (addr%4) {
		addr &= -0x4;
	}

	while (VREAD32(addr += 4) != ref);

	return addr;
}

// Entry Point
int module_start(SceSize args, void* argp) {
	logInit("ms0:/log_galaxy.txt");
	logmsg("Galaxy driver started...\n")

	// Get ISO path
	g_iso_fn = sctrlSEGetUmdFile();
	logmsg3("%s: UmdFile: %s\n", __func__, g_iso_fn);

	// Leave NP9660 alone, we got no ISO
	if(g_iso_fn[0] == 0) {
		return 1;
	}

	SceModule2 * pMod = (SceModule2 *)sceKernelFindModuleByName("sceThreadManager");

	if(pMod != NULL) {
		u32 ref;
		// Hook sceKernelCreateThread
		ref = K_EXTRACT_CALL(&sceKernelCreateThread);
		VWRITE32(findRefInGlobals("ThreadManForKernel", ref, ref), (u32)sceKernelCreateThreadPatched);

		// Hook sceKernelStartThread
		ref = K_EXTRACT_CALL(&sceKernelStartThread);
		VWRITE32(findRefInGlobals("ThreadManForKernel", ref, ref), (u32)sceKernelStartThreadPatched);
	}

	sctrlFlushCache();

	// ISO File Descriptor
	int fd = -1;

	// Wait for MS
	while (1) {
		fd = sceIoOpen(g_iso_fn, PSP_O_RDONLY, 0);

		if (fd >= 0) {
			break;
		}

		// Delay and retry
		sceKernelDelayThread(10000);
	}

	sceIoClose(fd);

	return 0;
}

