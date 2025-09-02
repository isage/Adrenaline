/*
	Adrenaline
	Copyright (C) 2016-2018, TheFloW

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

#ifndef __MAIN_H__
#define __MAIN_H__

#include <common.h>

#include "executable_patch.h"

extern RebootexConfig rebootex_config;
extern AdrenalineConfig config;

u32 sctrlHENFindImport(const char *szMod, const char *szLib, u32 nid);


int sceKermitSendRequest(SceKermitRequest *request, u32 mode, u32 cmd, u32 args, u32 is_callback, u64 *resp);
int sceKermitRegisterVirtualIntrHandler(int num, int (* handler)());

int sceKermitMemorySetArgument(SceKermitRequest *request, int argc, const void *data, int size, int mode);


int LoadExecForKernel_AA2029EC();

int sceChkregCheckRegion();
int sceChkregGetPsCode(u8 *pscode);

int ReadFile(char *file, void *buf, int size);
int WriteFile(char *file, void *buf, int size);

u32 _findJAL(u32 addr, int reversed, int skip);
#define findFirstJAL(addr) _findJAL(addr, 0, 0)
#define findFirstJALReverse(addr) _findJAL(addr, 1, 0)
#define findJAL(addr, pos) _findJAL(addr, 0, pos)
#define findJALReverse(addr, pos) _findJAL(addr, 1, pos)
#define findFirstJALForFunction(modname, libname, uid) findFirstJAL(FindFunction(modname, libname, uid))
#define findJALForFunction(modname, libname, uid, pos) findJAL(FindFunction(modname, libname, uid), pos)
#define findFirstJALReverseForFunction(modname, libname, uid) findFirstJALReverse(FindFunction(modname, libname, uid))
#define findJALReverseForFunction(modname, libname, uid, pos) findJALReverse(FindFunction(modname, libname, uid), pos)

#endif