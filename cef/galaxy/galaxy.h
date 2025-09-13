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

#define NP9660_DATA_1 (0x8A98)
#define NP9660_DATA_2 (0x8AA0)
#define NP9660_DATA_3 (0x8AB4)
#define NP9660_DATA_4 (0x8ABC)
#define NP9660_DATA_5 (0x8A14)
#define NP9660_ISO_FD (0x8A88)
#define NP9660_JAL_INITFORKERNEL (0x3C5C)
#define NP9660_JAL_FUNC_01 (0x3C78)
#define NP9660_JAL_FUNC_02 (0x4414)
#define NP9660_JAL_FUNC_03 (0x596C)
#define NP9660_IOCLOSE (0x7D68)
#define NP9660_FNPTR_INIT (0x36A8)
#define NP9660_FNPTR_READDISCSECTOR (0x4FEC)
#define NP9660_FNPTR_SECTORFLUSH (0x505C)

#define NP9660_DATA_1_VITA (0x8b58)
#define NP9660_DATA_2_VITA (0x8b60)
#define NP9660_DATA_3_VITA (0x8b74)
#define NP9660_DATA_4_VITA (0x8b7c)
#define NP9660_DATA_5_VITA (0x8ad4)
#define NP9660_ISO_FD_VITA (0x8b48)
#define NP9660_JAL_INITFORKERNEL_VITA (0x3c40)
#define NP9660_JAL_FUNC_01_VITA (0x3c5c)
#define NP9660_JAL_FUNC_02_VITA (0x43f8)
#define NP9660_JAL_FUNC_03_VITA (0x59d4)
#define NP9660_IOCLOSE_VITA (0x7dd0)
#define NP9660_FNPTR_INIT_VITA (0x368c)
#define NP9660_FNPTR_READDISCSECTOR_VITA (0x5054)

// Open ISO File
int open_iso(void);

// Init ISO Emulator
int initEmulator(void);

SceUID sceKernelCreateThreadPatched(const char * name, SceKernelThreadEntry entry, int initPriority, int stackSize, SceUInt attr, SceKernelThreadOptParam * option);
int readDiscSectorNP9660Patched(unsigned int sector, unsigned char * buffer, unsigned int size);
int sceIoClosePatched(int fd);
int sceKernelStartThreadPatched(SceUID thid, SceSize arglen, void * argp);

#endif // _GALAXY_H_

