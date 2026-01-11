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

#include <string.h>

#include <pspumd.h>
#include <pspreg.h>
#include <psperror.h>

#include <systemctrl.h>
#include <vshctrl.h>

#include "isofs_driver/isofs_driver.h"

#include <externs.h>

u8 g_set = 0;

int (* g_vshmenu_ctrl)(SceCtrlData *pad_data, int count);

int g_cpu_list[] = { 0, 20, 75, 100, 133, 222, 266, 300, 333 };
int g_bus_list[] = { 0, 10, 37,  50,  66, 111, 133, 150, 166 };

int vctrlVSHRegisterVshMenu(int (* ctrl)(SceCtrlData *, int)) {
	int k1 = pspSdkSetK1(0);
	g_vshmenu_ctrl = (void *)((u32)ctrl | 0x80000000);
	pspSdkSetK1(k1);
	return 0;
}

int vctrlVSHExitVSHMenu(SEConfig *conf, char *videoiso, int disctype) {
	int k1 = pspSdkSetK1(0);
	int oldspeed = g_cfw_config.vsh_cpu_speed;

	g_vshmenu_ctrl = NULL;
	memcpy(&g_cfw_config, conf, sizeof(SEConfigADR));
	sctrlSEApplyConfig((SEConfig*)&g_cfw_config);

	if (g_set) {
		if (g_cfw_config.vsh_cpu_speed != oldspeed) {
			if (g_cfw_config.vsh_cpu_speed) {
				sctrlHENSetSpeed(g_cpu_list[g_cfw_config.vsh_cpu_speed % N_CPU], g_bus_list[g_cfw_config.vsh_cpu_speed % N_CPU]);
			} else {
				sctrlHENSetSpeed(222, 111);
			}
		}
	}

	pspSdkSetK1(k1);
	return 0;
}

int vctrlGetRegistryValue(const char *dir, const char *name, u32 *val) {
	int ret = 0;
	struct RegParam reg;
	REGHANDLE h;
	int k1 = pspSdkSetK1(0);

	memset(&reg, 0, sizeof(reg));
	reg.regtype = 1;
	reg.namelen = strlen("/system");
	reg.unk2 = 1;
	reg.unk3 = 1;
	strcpy(reg.name, "/system");
	if (sceRegOpenRegistry(&reg, 2, &h) == 0) {
		REGHANDLE hd;
		if (!sceRegOpenCategory(h, dir, 2, &hd)) {
			REGHANDLE hk;
			unsigned int type, size;

			if (!sceRegGetKeyInfo(hd, name, &hk, &type, &size)) {
				if (!sceRegGetKeyValue(hd, hk, val, 4)) {
					ret = 1;
					sceRegFlushCategory(hd);
				}
			}
			sceRegCloseCategory(hd);
		}
		sceRegFlushRegistry(h);
		sceRegCloseRegistry(h);
	}

	pspSdkSetK1(k1);
	return ret;
}

int vctrlSetRegistryValue(const char *dir, const char *name, u32 val) {
	int ret = 0;
	struct RegParam reg;
	REGHANDLE h;
	int k1 = pspSdkSetK1(0);

	memset(&reg, 0, sizeof(reg));
	reg.regtype = 1;
	reg.namelen = strlen("/system");
	reg.unk2 = 1;
	reg.unk3 = 1;
	strcpy(reg.name, "/system");
	if (sceRegOpenRegistry(&reg, 2, &h) == 0) {
		REGHANDLE hd;
		if (!sceRegOpenCategory(h, dir, 2, &hd)) {
			if (!sceRegSetKeyValue(hd, name, &val, 4)) {
				ret = 1;
				sceRegFlushCategory(hd);
			} else {
				sceRegCreateKey(hd, name, REG_TYPE_INT, 4);
				sceRegSetKeyValue(hd, name, &val, 4);
				ret = 1;
				sceRegFlushCategory(hd);
			}
			sceRegCloseCategory(hd);
		}
		sceRegFlushRegistry(h);
		sceRegCloseRegistry(h);
	}

	pspSdkSetK1(k1);
	return ret;
}

int vshDetectDiscType(const char *path) {
	u32 k1 = pspSdkSetK1(0);

	int res = SCE_EINVAL;

	VshCtrlSetUmdFile(path);

	if (isofs_init() < 0) {
		isofs_exit();
		pspSdkSetK1(k1);
		return res;
	}

	res = 0;

	SceIoStat stat;
	if (isofs_getstat("/PSP_GAME/SYSDIR/EBOOT.BIN", &stat) >= 0) {
		res |= PSP_UMD_TYPE_GAME;
	}

	if (isofs_getstat("/UMD_VIDEO/PLAYLIST.UMD", &stat) >= 0) {
		res |= PSP_UMD_TYPE_VIDEO;
	}

	if (isofs_getstat("/UMD_AUDIO/PLAYLIST.UMD", &stat) >= 0) {
		res |= PSP_UMD_TYPE_AUDIO;
	}

	if (res == 0) {
		res = SCE_ENOMEDIUM;
	}

	isofs_exit();
	pspSdkSetK1(k1);
	return res;
}
