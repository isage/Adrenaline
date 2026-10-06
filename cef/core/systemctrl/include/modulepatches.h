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

#ifndef __MODULE_PATCHES_H__
#define __MODULE_PATCHES_H__

#include <psploadcore.h>

/**
 * Apply the partition 2 and 11 memory configured by ::sctrlHENSetMemory.
 *
 * @returns 0 on success. -1 if the function to get the partitions is not found.
*/
int ApplyMemory(void);
/**
 * Apply the partition 2 and 11 memory configured by ::sctrlHENSetMemory and
 * resets the rebootex config so a game can request large memory.
*/
void ApplyAndResetMemory(void);
void UnprotectExtraMemory(void);
void CheckControllerInput(void);

////////////////////////////////////////////////////////////////////////////////
// System Module Patchers
////////////////////////////////////////////////////////////////////////////////

void PatchChkreg(void);
void PatchSysmem(void);
void PatchLoadCore(void);
void PatchModuleMgr(void);
void PatchIoFileMgr(void);
void PatchInterruptMgr(void);
void PatchLoadExec(SceModule* mod);
void PatchMediaSync(SceModule* mod);
void PatchController(SceModule* mod);

////////////////////////////////////////////////////////////////////////////////
// Other Module Patchers
////////////////////////////////////////////////////////////////////////////////

void PatchGameByTitleId(void);
void PatchGameByTitleIdOnLoadExec(void);
void PatchGamesByMod(SceModule* mod);
void PatchHideCfwFiles(SceModule* mod);
void PatchPluginModule(SceModule *mod);

#endif