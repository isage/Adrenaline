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
#include <psppower.h>
#include <pspdisplay.h>
#include <pspthreadman.h>

#include <systemctrl.h>
#include <systemctrl_se.h>
#include <vshctrl.h>

#include "menu.h"
#include "satelite.h"

PSP_MODULE_INFO("EPI-VshCtrlSatelite", 0, 1, 0);

static char *g_cpuspeeds[] = { "Default", "20/10", "75/37", "100/50", "133/66", "222/111", "266/133", "300/150", "333/166" };
static char *g_umdmodes[] = { "Inferno", "M33 Driver", "Sony NP9660" };

static SEConfigADR g_cfw_config;

void SuspendDevice();
void RecoveryMenu();
void RestartVSH();

static Entry g_entries[] = {
	{ "CPU CLOCK XMB", NULL, g_cpuspeeds, sizeof(g_cpuspeeds), &g_cfw_config.vsh_cpu_speed, 0 },
	{ "CPU CLOCK GAME", NULL, g_cpuspeeds, sizeof(g_cpuspeeds), &g_cfw_config.app_cpu_speed, 0 },
	{ "UMD ISO MODE", NULL, g_umdmodes, sizeof(g_umdmodes), &g_cfw_config.umd_mode, 0 },
	{ "SUSPEND DEVICE", SuspendDevice, NULL, 0, NULL, 1 },
	{ "RECOVERY MENU", RecoveryMenu, NULL, 0, NULL, 1 },
	{ "RESTART VSH", RestartVSH, NULL, 0, NULL, 1 },
	{ "EXIT", NULL, NULL, 0, NULL, 1 },
};

static int g_exit_mode = -1;

u32 g_button_on = 0;
static u32 g_cur_buttons = -1;

static SceUID g_vshmenu_thid;

static int ctrl_handler(SceCtrlData *pad_data, int count) {
	g_button_on = pad_data->Buttons & ~g_cur_buttons;
	g_cur_buttons = pad_data->Buttons;

	for (int i = 0; i < count; i++) {
		pad_data[i].Buttons &= ~ALL_CTRL;
	}

	return 0;
}

void RecoveryMenu() {
	sctrlSESetBootConfFileIndex(BOOT_RECOVERY);
	sctrlKernelExitVSH(NULL);
}

void SuspendDevice() {
	scePowerRequestSuspend();
}

void RestartVSH() {
	sctrlKernelExitVSH(NULL);
}

int VshMenu_Thread(SceSize _args, void *_argp) {
	sceKernelChangeThreadPriority(0, 8);
	sctrlSEGetConfig(&g_cfw_config);
	vctrlVSHRegisterVshMenu(ctrl_handler);

	int exit_sel = 0, menu_mode = 0;

	MenuReset(g_entries, sizeof(g_entries));

	while (g_exit_mode < 0) {
		sceDisplayWaitVblankStart();

		if (menu_mode > 0) {
			MenuDisplay();
		}

		switch(menu_mode) {
			case 0:
				if (!(g_cur_buttons & ALL_CTRL)) {
					menu_mode++;
				}
				break;

			case 1:
				exit_sel = MenuCtrl();
				if (exit_sel >= 0) {
					menu_mode++;
				}
				break;

			case 2:
				if (!(g_cur_buttons & ALL_CTRL)) {
					g_exit_mode = exit_sel;
				}
				break;
		}
	}

	sctrlSESetConfig(&g_cfw_config);

	MenuExitFunction(g_exit_mode);

	vctrlVSHExitVSHMenu(&g_cfw_config);

	return sceKernelExitDeleteThread(0);
}

int module_start() {
	g_vshmenu_thid = sceKernelCreateThread("VshMenu_Thread", VshMenu_Thread, 0x10, 0x1000, 0, NULL);
	if (g_vshmenu_thid >= 0) {
		sceKernelStartThread(g_vshmenu_thid, 0, NULL);
	}

	return 0;
}

int module_stop() {
	g_exit_mode = 0;

	SceUInt timeout = 100000;
	if (sceKernelWaitThreadEnd(g_vshmenu_thid, &timeout) < 0) {
		sceKernelTerminateDeleteThread(g_vshmenu_thid);
	}

	return 0;
}