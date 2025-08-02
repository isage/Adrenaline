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

#define NP9660_DATA_1 (0x00005BB4 - 0x00005BA4 + 0x00000188 + 0x00008900)
#define NP9660_DATA_2 (0x00005BBC - 0x00005BA4 + 0x00000188 + 0x00008900)
#define NP9660_DATA_3 (0x00005BD0 - 0x00005BA4 + 0x00000188 + 0x00008900)
#define NP9660_DATA_4 (0x00005BD8 - 0x00005BA4 + 0x00000188 + 0x00008900)
#define NP9660_DATA_5 (0x00000114 + 0x00008900)
#define NP9660_ISO_FD (0x00000188 + 0x00008900)

// Open ISO File
int open_iso(void);

// Init ISO Emulator
int initEmulator(void);

SceUID sceKernelCreateThreadPatched(const char * name, SceKernelThreadEntry entry, int initPriority, int stackSize, SceUInt attr, SceKernelThreadOptParam * option);
int readDiscSectorNP9660Patched(unsigned int sector, unsigned char * buffer, unsigned int size);
int sceIoClosePatched(int fd);
int sceKernelStartThreadPatched(SceUID thid, SceSize arglen, void * argp);

#endif // _GALAXY_H_

