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

#ifndef __SCTRL_EXTERNS_H_
#define __SCTRL_EXTERNS_H_

#include <pspkermit.h>

#include <systemctrl.h>
#include <systemctrl_se.h>
#include <rebootexconfig.h>
#include <systemctrl_adrenaline.h>

#include "utils.h"

extern RebootexConfigEPI g_rebootex_config;
extern SEConfigEPI g_cfw_config;
extern STMOD_HANDLER g_module_handler;
extern SceAdrenaline *g_adrenaline;
extern u8 g_is_plugin_loading;


void lowerString(char* orig, char* ret, int strSize);

int LoadExecForKernel_AA2029EC();

int sceChkregCheckRegion();
int sceChkregGetPsCode(u8 *pscode);

u32 sctrlHENFakeDevkitVersion();
SceUID sceIoOpenDrmPatched(const char *path, int flags, SceMode mode);

#endif /* __SCTRL_EXTERNS_H_ */