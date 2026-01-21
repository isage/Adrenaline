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
int ApplyMemory();
/**
 * Apply the partition 2 and 11 memory configured by ::sctrlHENSetMemory and
 * resets the rebootex config so a game can request large memory.
*/
void ApplyAndResetMemory();
void UnprotectExtraMemory();
void CheckControllerInput();

////////////////////////////////////////////////////////////////////////////////
// System Module Patchers
////////////////////////////////////////////////////////////////////////////////

void PatchChkreg();
void PatchMemlmd();
void PatchSysmem();
void PatchUtility();
void PatchLoadCore();
void PatchModuleMgr();
void PatchIoFileMgr();
void PatchInterruptMgr();
void PatchMesgLed(SceModule* mod);
void PatchLoadExec(SceModule* mod);
void PatchMediaSync(SceModule* mod);
void PatchUmdDriver(SceModule* mod);
void PatchController(SceModule* mod);
void PatchImposeDriver(SceModule* mod);
void PatchUSBCamDriver(SceModule* mod);
void PatchPowerService(SceModule* mod);
void PatchMeCodecWrapper(SceModule* mod);

////////////////////////////////////////////////////////////////////////////////
// Other Module Patchers
////////////////////////////////////////////////////////////////////////////////

void PatchVolatileMemBug();
void PatchGameByTitleId();
void PatchGameByTitleIdOnLoadExec();
void PatchGamesByMod(SceModule* mod);
void PatchHideCfwFiles(SceModule* mod);

void PatchVlfLib(SceModule* mod);
void PatchCwCheatPlugin(SceModule* mod);

#endif