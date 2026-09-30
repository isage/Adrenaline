/*
	Adrenaline
	Copyright (C) 2016-2018, TheFloW
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

#include <string.h>

#include <pspkernel.h>
#include <psppower.h>
#include <psperror.h>

#include <cfwmacros.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <systemctrl_adrenaline.h>


static int scePowerRequestColdResetPatched(int a0) {
	sctrlSESetBootConfFileIndex(MODE_UMD);

	if (a0 == 0) {
		return sctrlKernelExitVSH(NULL);
	}

	// Expand `scePowerRequestColdReset` to cold reset the VITA if argument is 1.
	// The only valid argument on PSP is zero, so official software will never
	// use it, but homebrew software can use it.
	if (a0 == 1) {
		return sctrlRebootDevice();
	}

	return SCE_ERR_INMODE;
}

static __attribute__((noinline)) int scePowerGetBatteryLifeTimePatched(void) {
	while(*(volatile u32 *)0xBFC0017C != *(volatile u32 *)0xBFC00180);
	short lifetime = *(volatile short *)0xBFC00184;
	return (lifetime < 0) ? 0 : (int)lifetime;
}

static int power_online(void) {
	return (scePowerGetBatteryLifeTimePatched() == 0) ? 1 : 0;
}

static float (* sceClkcGetCpuFrequency)(void);

static float sceClkcGetCpuFrequencyPatched(void) {
	float res = sceClkcGetCpuFrequency();

	u32 res_hex;
	memcpy(&res_hex, &res, sizeof(u32));

	// 221.566 -> 222
	if (res_hex == 0x435D90C8) {
		res_hex = 0x435E0000;
		memcpy(&res, &res_hex, sizeof(u32));
	}

	return res;
}

static float sceSysregPllGetFrequencyPatched(void) {
	return 333.0f;
}

void PatchPowerService(SceModule* mod) {
	u32 text_addr = mod->text_addr;

	// Redirect to similar functions
	REDIRECT_FUNCTION(K_EXTRACT_IMPORT(&scePowerRequestStandby), K_EXTRACT_IMPORT(&scePowerRequestSuspend));
	REDIRECT_FUNCTION(K_EXTRACT_IMPORT(&scePowerRequestColdReset), scePowerRequestColdResetPatched);

	// Patch to fix charging status
	REDIRECT_FUNCTION(K_EXTRACT_IMPORT(&scePowerGetBatteryLifeTime), scePowerGetBatteryLifeTimePatched);
	REDIRECT_FUNCTION(K_EXTRACT_IMPORT(&scePowerIsPowerOnline), power_online);
	REDIRECT_FUNCTION(K_EXTRACT_IMPORT(&scePowerIsBatteryCharging), power_online);
	MAKE_DUMMY_FUNCTION(K_EXTRACT_IMPORT(&scePowerGetBatteryChargingStatus), 1);

	// Dummy not working functions
	MAKE_DUMMY_FUNCTION(K_EXTRACT_IMPORT(&scePowerGetBatteryTemp), 0);
	MAKE_DUMMY_FUNCTION(K_EXTRACT_IMPORT(&scePowerGetBatteryVolt), 0);

	// Allow all frequencies for scePowerSetCpuClockFrequency
	VWRITE16(text_addr + 0x3182, 0x1000);
	VWRITE16(text_addr + 0x319A, 0x1000);

	// Allow all frequencies for scePowerSetClockFrequency
	VWRITE16(text_addr + 0x339A, 0x1000);

	// Patch
	sceClkcGetCpuFrequency = (void *)K_EXTRACT_IMPORT(text_addr + 0x4810);
	MAKE_JUMP(text_addr + 0x4810, sceClkcGetCpuFrequencyPatched);

	SceModule *mod_low_io = sceKernelFindModuleByName("sceLowIO_Driver");

	MAKE_CALL(mod_low_io->text_addr + 0x2B60, sceSysregPllGetFrequencyPatched);
	MAKE_CALL(mod_low_io->text_addr + 0x2BC4, sceSysregPllGetFrequencyPatched);

	sctrlFlushCache();
}
