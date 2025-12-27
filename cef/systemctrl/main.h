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

#include "utils.h"

extern RebootexConfig rebootex_config;
extern AdrenalineConfig config;
extern STMOD_HANDLER module_handler;

u32 sctrlHENFindImport(const char *szMod, const char *szLib, u32 nid);


int sceKermitSendRequest(SceKermitRequest *request, u32 mode, u32 cmd, u32 args, u32 is_callback, u64 *resp);
int sceKermitRegisterVirtualIntrHandler(int num, int (* handler)());

int sceKermitMemorySetArgument(SceKermitRequest *request, int argc, const void *data, int size, int mode);


int LoadExecForKernel_AA2029EC();

int sceChkregCheckRegion();
int sceChkregGetPsCode(u8 *pscode);

u32 sctrlHENFakeDevkitVersion();
SceUID sceIoOpenDrmPatched(const char *path, int flags, SceMode mode);

#endif