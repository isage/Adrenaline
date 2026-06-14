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

#ifndef _SCTRL_UTILS_H_
#define _SCTRL_UTILS_H_

#include <psptypes.h>

#define SCE_MODULE_USER_MODULE  (0x100)

int ReadFile(char *file, void *buf, int size);
int WriteFile(char *file, void *buf, int size);

u32 _findJALaddr(u32 addr, int reversed, int skip);
#define findFirstJALaddr(addr) _findJALaddr(addr, 0, 0)
#define findFirstJALaddrReverse(addr) _findJALaddr(addr, 1, 0)
#define findJALaddr(addr, pos) _findJALaddr(addr, 0, pos)
#define findJALaddrReverse(addr, pos) _findJALaddr(addr, 1, pos)
#define findFirstJALaddrForFunction(modname, libname, uid) findFirstJALaddr(FindFunction(modname, libname, uid))
#define findJALaddrForFunction(modname, libname, uid, pos) findJALaddr(FindFunction(modname, libname, uid), pos)
#define findFirstJALaddrReverseForFunction(modname, libname, uid) findFirstJALaddrReverse(FindFunction(modname, libname, uid))
#define findJALaddrReverseForFunction(modname, libname, uid, pos) findJALaddrReverse(FindFunction(modname, libname, uid), pos)

#endif