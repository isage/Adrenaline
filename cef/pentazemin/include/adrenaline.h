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

#ifndef __ADRENALINE_H__
#define __ADRENALINE_H__

#include <psptypes.h>
#include <psploadcore.h>

int SendAdrenalineCmd(int cmd, u32 args);

void initAdrenalineInfo();

void PatchSasCore(SceModule* mod);
void PatchLowIODriver2(SceModule* mod);
void PatchPowerService2(SceModule* mod);

int initAdrenaline();

#endif