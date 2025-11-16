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
#include <string.h>

#include <pspiofilemgr.h>
#include <psputilsforkernel.h>
#include <pspthreadman_kernel.h>

#include <cfwmacros.h>
#include <systemctrl.h>

#include "galaxy.h"
#include "../bits/iso_common.h"

// SceNpUmdMount Thread ID
static SceUID g_SceNpUmdMount_thid = -1;

// np9660.prx Text Address
static u32 g_sceNp9660_driver_text_addr = 0;

// Dummy UMD Data for Global Pointer
static unsigned char g_umddata[16] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

// NP9660 Initialization Call
void (* initNP9660)(u32 zero) = NULL;

// NP9660 Disc Sector Reader
int (* readDiscSectorNP9660)(u32 sector, void * buffer, u32 size) = NULL;

// NP9660 Sector Flush
void (* readSectorFlushNP9660)(void) = NULL;

// Values for physical UMD seek/read speed simulation
static u8 g_umd_seek = 0;
static u8 g_umd_speed = 0;
static u32 g_cur_offset = 0;
static u32 g_last_read_offset = 0;

int sceKernelSetQTGP3(void *data);

////////////////////////////////////////////////////////////////////////////////
// HELPERS
////////////////////////////////////////////////////////////////////////////////

static u32 findRefInGlobals(char* libname, u32 addr, u32 ptr) {
	while (strcmp(libname, (char*)addr)) {
		addr++;
	}

	if (addr%4) {
		addr &= -0x4; // align to 4 bytes
	}

	while (VREAD32(addr += 4) != ptr);

	return addr;
}

// Open ISO File
static int open_iso(void) {
	int res = iso_open();

	if (res < 0) {
		return res;
	}

	// Update NP9660 File Descriptor Variable
	VWRITE32(g_sceNp9660_driver_text_addr + NP9660_ISO_FD, g_iso_fd);

	return 0;
}


////////////////////////////////////////////////////////////////////////////////
// CFW API (That depends on this file globals/statics)
////////////////////////////////////////////////////////////////////////////////

void isoSetUmdDelay(int seek, int speed) {
	g_umd_seek = seek;
	g_umd_speed = speed;
}

int isoReadUmdFile(u32 sector, void *buffer, u32 size) {
	// This will fail but it sets some needed variables
	readDiscSectorNP9660(sector, buffer, size);

	// Prepare Arguments for our custom ISO Sector Reader
	IoReadArg args;
	args.offset = sector;
	args.address = buffer;
	args.size = size;

	int res = sceKernelExtendKernelStack(0x2000, (void *)iso_read, &args);


	if (g_umd_seek){
		// simulate seek time
		u32 diff = 0;
		g_last_read_offset = sector+size;
		if (g_cur_offset > g_last_read_offset) {
			diff = g_cur_offset-g_last_read_offset;
		} else {
			diff = g_last_read_offset-g_cur_offset;
		}
		g_cur_offset = g_last_read_offset;
		u32 seek_time = (diff*g_umd_seek)/1024;
		sceKernelDelayThread(seek_time);
	}
	if (g_umd_speed){
		// simulate read time
		sceKernelDelayThread(size*g_umd_speed);
	}

	// Flush & Update NP9660 Sector Information
	readSectorFlushNP9660();

	return res;
}

////////////////////////////////////////////////////////////////////////////////
// PATCHED IMPLEMENTATIONS
////////////////////////////////////////////////////////////////////////////////

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
	VWRITE32(g_sceNp9660_driver_text_addr + NP9660_DATA_3, g_total_sectors);
	VWRITE32(g_sceNp9660_driver_text_addr + NP9660_DATA_4, g_total_sectors);
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
		PatchNp9660Driver();
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

////////////////////////////////////////////////////////////////////////////////
// MODULE PATCHERS
////////////////////////////////////////////////////////////////////////////////

void PatchNp9660Driver() {
	SceModule * mod = sceKernelFindModuleByName("sceNp9660_driver");
	g_sceNp9660_driver_text_addr = mod->text_addr;

	// Make InitForKernel Call return 0x80000000 always
	MAKE_INSTRUCTION(g_sceNp9660_driver_text_addr + NP9660_JAL_INITFORKERNEL, 0x3C028000); // jal InitForKernel_23458221 to lui $v0, 0x00008000

	// Hook Function #1
	MAKE_CALL(g_sceNp9660_driver_text_addr + NP9660_JAL_FUNC_01, initEmulator);

	// Hook Function #2
	MAKE_CALL(g_sceNp9660_driver_text_addr + NP9660_JAL_FUNC_02, readDiscSectorNP9660Patched); // jal sub_3948 to jal sub_00000054
	MAKE_CALL(g_sceNp9660_driver_text_addr + NP9660_JAL_FUNC_03, readDiscSectorNP9660Patched); // jal sub_3948 to jal sub_00000054

	// Hook sceIoClose Stub
	MAKE_JUMP(g_sceNp9660_driver_text_addr + NP9660_IOCLOSE, sceIoClosePatched); // hook sceIoClose import

	// Backup Function Pointer
	initNP9660 = (void *)(mod->text_addr + NP9660_FNPTR_INIT);
	readDiscSectorNP9660 = (void *)(mod->text_addr + NP9660_FNPTR_READDISCSECTOR);
	readSectorFlushNP9660 = (void *)(mod->text_addr + NP9660_FNPTR_SECTORFLUSH);

	sctrlFlushCache();
}

void PatchThreadManager() {
	SceModule * mod = (SceModule *)sceKernelFindModuleByName("sceThreadManager");

	if(mod != NULL) {
		u32 ref;
		// Hook sceKernelCreateThread
		ref = K_EXTRACT_CALL(&sceKernelCreateThread);
		VWRITE32(findRefInGlobals("ThreadManForKernel", ref, ref), (u32)sceKernelCreateThreadPatched);

		// Hook sceKernelStartThread
		ref = K_EXTRACT_CALL(&sceKernelStartThread);
		VWRITE32(findRefInGlobals("ThreadManForKernel", ref, ref), (u32)sceKernelStartThreadPatched);
	}

	sctrlFlushCache();
}