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
 * Module patches for modules not related to the system
 *
 * Mostly third-party modules
 */

#include <string.h>

#include <pspinit.h>

#include <adrenaline_log.h>

#include "main.h"
#include "adrenaline.h"
#include "../../adrenaline_compat.h"

#include "plugin.h"

////////////////////////////////////////////////////////////////////////////////
// PATCHED IMPLEMENTATIONS
////////////////////////////////////////////////////////////////////////////////
int sceKernelSuspendThreadPatched(SceUID thid) {
	SceKernelThreadInfo info;
	info.size = sizeof(SceKernelThreadInfo);
	if (sceKernelReferThreadStatus(thid, &info) == 0) {
		if (strcmp(info.name, "popsmain") == 0) {
			SendAdrenalineCmd(ADRENALINE_VITA_CMD_PAUSE_POPS, 0);
		}
	}

	return sceKernelSuspendThread(thid);
}

int sceKernelResumeThreadPatched(SceUID thid) {
	SceKernelThreadInfo info;
	info.size = sizeof(SceKernelThreadInfo);
	if (sceKernelReferThreadStatus(thid, &info) == 0) {
		if (strcmp(info.name, "popsmain") == 0) {
			SendAdrenalineCmd(ADRENALINE_VITA_CMD_RESUME_POPS, 0);
		}
	}

	return sceKernelResumeThread(thid);
}

int (* _sceKernelVolatileMemTryLock)(int unk, void **ptr, int *size) = NULL;
int sceKernelVolatileMemTryLockPatched(int unk, void **ptr, int *size) {
	int res = 0;

	for (int i = 0; i < 0x10; i++) {
		res = _sceKernelVolatileMemTryLock(unk, ptr, size);
		if (res >= 0) {
			break;
		}

		sceKernelDelayThread(100);
	}

	return res;
}

////////////////////////////////////////////////////////////////////////////////
// MODULE PATCHERS
////////////////////////////////////////////////////////////////////////////////

void PatchVolatileMemBug() {
	if (sceKernelBootFrom() == PSP_BOOT_DISC) {
		_sceKernelVolatileMemTryLock = (void *)sctrlHENFindFunction("sceSystemMemoryManager", "sceSuspendForUser", 0xA14F40B2);
		sctrlHENPatchSyscall(_sceKernelVolatileMemTryLock, sceKernelVolatileMemTryLockPatched);
		sctrlFlushCache();
	}
}

void PatchCwCheatPlugin(SceModule* mod) {
	if (sceKernelApplicationType() == PSP_INIT_KEYCONFIG_POPS) {
		sctrlHookImportByNID(mod, "ThreadManForKernel", 0x9944F31F, sceKernelSuspendThreadPatched);
		sctrlHookImportByNID(mod, "ThreadManForKernel", 0x75156E8F, sceKernelResumeThreadPatched);
		sctrlFlushCache();
	}
}

void PatchVlfLib(SceModule* mod) {
	static u32 nids[] = { 0x2A245FE6, 0x7B08EAAB, 0x22050FC0, 0x158BE61A, 0xD495179F };

	for (int i = 0; i < (sizeof(nids) / sizeof(u32)); i++) {
		sctrlHookImportByNID(mod, "VlfGui", nids[i], NULL);
	}

	sctrlFlushCache();
}