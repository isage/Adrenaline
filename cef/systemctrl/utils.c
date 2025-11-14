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

/**
 * SystemControl private utility functions
 */

#include <pspiofilemgr.h>
#include <cfwmacros.h>

int ReadFile(char *file, void *buf, int size) {
	SceUID fd = sceIoOpen(file, PSP_O_RDONLY, 0);
	if (fd < 0)
		return fd;

	int read = sceIoRead(fd, buf, size);

	sceIoClose(fd);
	return read;
}

int WriteFile(char *file, void *buf, int size) {
	SceUID fd = sceIoOpen(file, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	if (fd < 0)
		return fd;

	int written = sceIoWrite(fd, buf, size);

	sceIoClose(fd);
	return written;
}

// Taken from ARK-3
u32 FindFirstBEQ(u32 addr) {
	for (;; addr += 4){
		u32 data = VREAD32(addr);
		if ((data & 0xFC000000) == 0x10000000) {
			return addr;
		}
	}

	return 0;
}

// Taken from ARK CFW
// TODO: Rewrite using `_findJALaddr`
u32 _findJAL(u32 addr, int reversed, int skip) {
	if (addr != 0) {
		int add = 4;
		if (reversed) {
			add = -4;
		}
		for(;;addr += add) {
			if ((VREAD32(addr) >= 0x0C000000) && (VREAD32(addr) < 0x10000000)){
				if (skip == 0) {
					return (((VREAD32(addr) & 0x03FFFFFF) << 2) | 0x80000000);
				} else {
					skip--;
				}
			}
		}
	}

	return 0;
}

u32 _findJALaddr(u32 addr, int reversed, int skip) {
	if (addr != 0) {
		int add = 4;
		if (reversed) {
			add = -4;
		}
		for(;;addr += add) {
			if ((VREAD32(addr) >= 0x0C000000) && (VREAD32(addr) < 0x10000000)){
				if (skip == 0) {
					return addr;
				} else {
					skip--;
				}
			}
		}
	}

	return 0;
}
