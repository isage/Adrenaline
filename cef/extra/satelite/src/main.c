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
#include <string.h>

#include <psppower.h>
#include <pspdisplay.h>
#include <pspthreadman.h>

#include <systemctrl.h>
#include <systemctrl_se.h>
#include <vshctrl.h>
#include <cfwmacros.h>

#define _ADRENALINE_LOG_IMPL_
// #define _ADRENALINE_LOG_USE_PAF_
#include <adrenaline_log.h>

#include <ya2d.h>
#include <tinyfont.h>

#include "menu.h"
#include "font.h"
#include "satelite.h"
#include "options.h"

PSP_MODULE_INFO("EPI-VshCtrlGuSatelite", 0, 2, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_VFPU);



static SEConfigEPI g_cfw_config;

static struct {
	u32 cur_buttons;
	u32 button_on;
	int stop_flag;
	int menu_mode;
	int is_registered;
	int show_info;
} g_vshmenu;

static int main_menu_ctrl(u32 button_on);
static int adv_game_menu_ctrl_p1(u32 button_on);
static int adv_game_menu_ctrl_p2(u32 button_on);

int (*g_menu_ctrl)(u32 button_on) = main_menu_ctrl;

static char g_current_header_text[128] = { 0 };

static u32 g_button_assign_value = 0;

static void SuspendDevice();
static void RecoveryMenu();
static void RestartVSH();
static void switchMainMenu();
static void switchAdvGameMenuP1();
static void switchAdvGameMenuP2();
static void Exit();

static Entry g_main_menu_entries[] = {
	{ "CPU CLOCK XMB", NULL, g_cpuspeeds, NELEMS(g_cpuspeeds), &g_cfw_config.vsh_cpu_speed, 0 },
	{ "CPU CLOCK GAME", NULL, g_cpuspeeds, NELEMS(g_cpuspeeds), &g_cfw_config.app_cpu_speed, 0 },
	{ "UMD ISO MODE", NULL, g_umdmodes, NELEMS(g_umdmodes), &g_cfw_config.umd_mode, 0 },
	{ "", NULL, NULL, 0, NULL, 0 },
	{ "ADVANCED GAME OPTIONS", switchAdvGameMenuP1, NULL, 0, NULL, 1 },
	{ "SUSPEND DEVICE", SuspendDevice, NULL, 0, NULL, 1 },
	{ "RECOVERY MENU", RecoveryMenu, NULL, 0, NULL, 1 },
	{ "RESTART VSH", RestartVSH, NULL, 0, NULL, 1 },
	{ "EXIT", Exit, NULL, 0, NULL, 1 },
};

static Entry g_adv_quick_game_entries_p1[] = {
	{ "CPU CLOCK GAME", NULL, g_cpuspeeds, NELEMS(g_cpuspeeds), &g_cfw_config.app_cpu_speed, 0 },
	{ "UMD ISO MODE", NULL, g_umdmodes, NELEMS(g_umdmodes), &g_cfw_config.umd_mode, 0 },
	{ "HIDE CFW FILES", NULL, g_endisabled, NELEMS(g_endisabled), &g_cfw_config.no_hide_cfw_files, 0 },
	{ "USE SONY PSP OSK", NULL, g_disenabled, NELEMS(g_disenabled), &g_cfw_config.use_sony_psposk, 0 },
	{ "MS CACHE", NULL, g_endisabled, NELEMS(g_endisabled), &g_cfw_config.no_ms_cache, 0 },
	{ "", NULL, NULL, 0, NULL, 0 },
	{ "FORCE HIGH MEM", NULL, g_highmem, NELEMS(g_highmem), &g_cfw_config.force_high_memory, 0 },
	{ "FAKE MAX FREE RAM", NULL, g_fake_free_mem, NELEMS(g_fake_free_mem), &g_cfw_config.fake_max_free_mem, 0 },
	{ "", NULL, NULL, 0, NULL, 0 },
	{ "PART 2", switchAdvGameMenuP2, NULL, 0, NULL, 0 },
	{ "", NULL, NULL, 0, NULL, 0 },
	{ "BACK TO MAIN MENU", switchMainMenu, NULL, 0, NULL, 0 },
};

static Entry g_adv_quick_game_entries_p2[] = {
	{ "INFERNO CACHE POLICY", NULL, g_iso_cache, NELEMS(g_iso_cache), &g_cfw_config.iso_cache, 0 },
	{ "INFERNO CACHE NUMBER", NULL, g_iso_cache_num, NELEMS(g_iso_cache_num), &g_cfw_config.iso_cache_num, 0 },
	{ "INFERNO CACHE SIZE", NULL, g_iso_cache_size, NELEMS(g_iso_cache_size), &g_cfw_config.iso_cache_size, 0 },
	{ "ISO SEEK DELAY", NULL, g_umd_seek_read_delay, NELEMS(g_umd_seek_read_delay), &g_cfw_config.umd_seek, 0 },
	{ "ISO READ DELAY", NULL, g_umd_seek_read_delay, NELEMS(g_umd_seek_read_delay), &g_cfw_config.umd_speed, 0 },
	{ "SEEK/READ DELAY TYPE", NULL, g_umd_seek_read_strat, NELEMS(g_umd_seek_read_strat), &g_cfw_config.umd_sim_strat, 0 },
	{ "USE GRAPHIC ENGINE 2", NULL, g_disenabled, NELEMS(g_disenabled), &g_cfw_config.use_ge2, 0 },
	{ "USE MEDIA ENGINE 2", NULL, g_disenabled, NELEMS(g_disenabled), &g_cfw_config.use_me2, 0 },
	{ "", NULL, NULL, 0, NULL, 0 },
	{ "BACK TO PART 1", switchAdvGameMenuP1, NULL, 0, NULL, 0 },
	{ "", NULL, NULL, 0, NULL, 0 },
	{ "BACK TO MAIN MENU", switchMainMenu, NULL, 0, NULL, 0 },
};

static Entry *g_current_menu = g_main_menu_entries;
static int g_current_menu_size = NELEMS(g_main_menu_entries);

static int g_window_w = 0;
static int g_window_h = 0;

static int g_curr_sel = 0;

static SceUID g_vshmenu_thid = -1;

static TinyFontState g_header_state;
static TinyFontState g_curr_entry_state;

static int calcWindowWidth() {
	int max_w = 50;
	for (int i=0; i < g_current_menu_size; i++) {
		int sw = 9*strlen(g_current_menu[i].name);
		int idx = *g_current_menu[i].value;
		if (g_current_menu[i].options && g_current_menu[i].size_options > *g_current_menu[i].value && g_current_menu[i].options[idx]) {
			// Find the biggest option
			int max = strlen(g_current_menu[i].options[idx]);

			for (int j = 0; j < g_current_menu[i].size_options; j++) {
				int len = strlen(g_current_menu[i].options[j]);
				if (len > max) {
					max = len;
				}
			}

			sw += 9*max;
		}
		if (sw > max_w) {
			max_w = sw;
		}
	}
	int res = max_w + 36;
	if (res > 480) {
		res = 480;
	}
	return res;
}

static int calcWindowHeight(){
	int res = 8 * g_current_menu_size + 35;
	if (res > 272) {
		res = 272;
	}
	return res;
}

static void switchMainMenu() {
	memset(g_current_header_text, 0, 128);
	strcpy(g_current_header_text, MAIN_HEADER);
	g_current_menu = g_main_menu_entries;
	g_current_menu_size = NELEMS(g_main_menu_entries);
	g_curr_sel = 0;
	g_menu_ctrl = main_menu_ctrl;
	g_window_w = calcWindowWidth();
    g_window_h = calcWindowHeight();
	g_vshmenu.menu_mode = 1;
}

static void switchAdvGameMenuP1() {
	memset(g_current_header_text, 0, 128);
	strcpy(g_current_header_text, GAME_OPT_HEADER);
	g_current_menu = g_adv_quick_game_entries_p1;
	g_current_menu_size = NELEMS(g_adv_quick_game_entries_p1);
	g_curr_sel = 0;
	g_menu_ctrl = adv_game_menu_ctrl_p1;
	g_window_w = calcWindowWidth();
    g_window_h = calcWindowHeight();
	g_vshmenu.menu_mode = 1;
}

static void switchAdvGameMenuP2() {
	g_current_menu = g_adv_quick_game_entries_p2;
	g_current_menu_size = NELEMS(g_adv_quick_game_entries_p2);
	g_curr_sel = 0;
	g_menu_ctrl = adv_game_menu_ctrl_p2;
	g_window_w = calcWindowWidth();
    g_window_h = calcWindowHeight();
	g_vshmenu.menu_mode = 1;
}


static void ChangeValue(int interval) {
	if (g_current_menu[g_curr_sel].options) {
		int max = g_current_menu[g_curr_sel].size_options;
		(*g_current_menu[g_curr_sel].value) = (max + (*g_current_menu[g_curr_sel].value) + interval) % max;
	}

	// if (g_current_menu[g_curr_sel].function) {
	// 	g_current_menu[g_curr_sel].function(g_sel);
	// }
}

static void ChangeSelection(int interval) {
	if (g_current_menu_size > 0) {
		g_curr_sel = (g_current_menu_size + g_curr_sel + interval) % g_current_menu_size;

		// Skip empty lines
		if (strcmp(g_current_menu[g_curr_sel].name, "") == 0) {
			g_curr_sel = (g_current_menu_size + g_curr_sel + interval) % g_current_menu_size;
		}
	}
}

static int is_button_enter(u32 button_on) {
	if (g_button_assign_value == 0) {
		return (button_on & PSP_CTRL_CIRCLE) ? 1 : 0;
	} else {
		return (button_on & PSP_CTRL_CROSS) ? 1 : 0;
	}
}

static int is_button_back(u32 button_on) {
	if (g_button_assign_value > 0) {
		return (button_on & PSP_CTRL_CIRCLE) ? 1 : 0;
	} else {
		return (button_on & PSP_CTRL_CROSS) ? 1 : 0;
	}
}

static int menu_ctrl_common(u32 button_on, void (*back_action)()) {
	if ((button_on & PSP_CTRL_SELECT) || (button_on & PSP_CTRL_HOME)) {
		return 1;

	} else if (button_on & PSP_CTRL_DOWN) {
		ChangeSelection(1);

	} else if (button_on & PSP_CTRL_UP) {
		ChangeSelection(-1);

	} else if (button_on & PSP_CTRL_LEFT) {
		ChangeValue(-1);

	} else if (button_on & PSP_CTRL_RIGHT) {
		ChangeValue(1);

	} else if (is_button_enter(button_on)) {
		if (g_current_menu[g_curr_sel].function != NULL) {
			g_current_menu[g_curr_sel].function();

			if (g_current_menu[g_curr_sel].exit) {
				return 1;
			}
		} else {
			ChangeValue(1);
		}

	} else if (is_button_back(button_on)) {
		back_action();

	}

	return 0;
}

static void Exit() {
	g_vshmenu.stop_flag = 1;
}

static int main_menu_ctrl(u32 button_on) {
	return menu_ctrl_common(button_on, Exit);
}

static int adv_game_menu_ctrl_p1(u32 button_on) {
	return menu_ctrl_common(button_on, switchMainMenu);
}

static int adv_game_menu_ctrl_p2(u32 button_on) {
	return menu_ctrl_common(button_on, switchAdvGameMenuP1);
}

static int EatKey(SceCtrlData *pad_data, int count) {
	// buttons check
	g_vshmenu.button_on   = ~g_vshmenu.cur_buttons & pad_data[0].Buttons;
	g_vshmenu.cur_buttons = pad_data[0].Buttons;

	// mask buttons for LOCK VSH control
	for (int i=0; i<count; i++) {
		pad_data[i].Buttons &= ~(
				PSP_CTRL_SELECT|PSP_CTRL_START|
				PSP_CTRL_UP|PSP_CTRL_RIGHT|PSP_CTRL_DOWN|PSP_CTRL_LEFT|
				PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER|
				PSP_CTRL_TRIANGLE|PSP_CTRL_CIRCLE|PSP_CTRL_CROSS|PSP_CTRL_SQUARE|
				PSP_CTRL_HOME|PSP_CTRL_NOTE);

	}

	return 0;
}

void vshmenu_draw(void* frame){

    if (g_vshmenu.stop_flag) {
		return;
	}

    int w = g_window_w, h = g_window_h;
    int x = (480-w)/2;
    int y = (272-h)/2;
    u32 bgcolor = (0xA0<<24) | 0x00993366;
    char* header = g_current_header_text;
	int header_len = strlen(header);
    int subx = CENTER(header_len);
    int hw = 8*header_len+10;

    ya2d_draw_rect(subx-5,  y-15,   hw,   15,   0x8000ff00,  1); // header background
    ya2d_draw_rect(x,     y,      w,    h,    bgcolor,     1); // menu background
    ya2d_draw_rect(x+1,   y-1,    w-2,  1,    bgcolor,     1); // top horizontal outline
    ya2d_draw_rect(x+1,   y+h,    w-2,  1,    bgcolor,     1); // bottom horizontal outline
    ya2d_draw_rect(x-1,   y+1,    1,    h-2,  bgcolor,     1); // left vertical outline
    ya2d_draw_rect(x+w,   y+1,    1,    h-2,  bgcolor,     1); // right vertical outline

    g_header_state.ix = subx;
    tinyFontPrintTextScreenBuf(frame, g_font, g_header_state.ix, y-12, header, WHITE_COLOR, &g_header_state);

    for (int i = 0; i < g_current_menu_size; i++) {
        char text[128] = { 0 };
        strcpy(text, g_current_menu[i].name);

		logmsg("[DEBUG]: %s: drawing %s\n", __func__, g_current_menu[i].name);

        if (g_current_menu[i].options && g_current_menu[i].size_options > *(g_current_menu[i].value)) {
			logmsg("[DEBUG]: %s: current %s option is %s\n", __func__, g_current_menu[i].name, g_current_menu[i].options[*g_current_menu[i].value]);
            char* opt_txt = g_current_menu[i].options[*g_current_menu[i].value];

            int padding = 3 + ((w - 36)/8) - strlen(text) - strlen(opt_txt);
            for (int p = 0; p < padding; p++) {
				if (p == 0 || p == padding-1) {
					strcat(text, " ");
				} else {
					strcat(text, "-");
				}
			}

            strcat(text, opt_txt);
        }

		int text_x = (g_current_menu[i].options) ? x+10 : CENTER(strlen(text));
        g_curr_entry_state.glow = (i==g_curr_sel) ? 1 : 0;
        tinyFontPrintTextScreenBuf(frame, g_font, text_x, y+(10*(i+1)), text, WHITE_COLOR, &g_curr_entry_state);
    }
}

static void button_func(void) {
	// menu control
	switch (g_vshmenu.menu_mode) {
		case 0:
			if ((g_vshmenu.cur_buttons & ALL_CTRL) == 0) {
				g_vshmenu.menu_mode = 1;
			}
			break;
		case 1:
			if (g_menu_ctrl(g_vshmenu.button_on))
				g_vshmenu.menu_mode = 2;
			break;
		case 2:
			if ((g_vshmenu.cur_buttons & ALL_CTRL) == 0)
				g_vshmenu.stop_flag = 1;
			break;
	}
}

static void RecoveryMenu() {
	sctrlSESetBootConfFileIndex(MODE_RECOVERY);
	sctrlKernelExitVSH(NULL);
}

static void SuspendDevice() {
	scePowerRequestSuspend();
}

static void RestartVSH() {
	sctrlKernelExitVSH(NULL);
}

int VshMenu_Thread(SceSize _args, void *_argp) {
	sceKernelChangeThreadPriority(0, 8);
	sctrlSEGetConfig((SEConfig*)&g_cfw_config);
	logmsg("[INFO]: %s: Loaded Config\n", __func__);
	vctrlVSHRegisterVshMenu(EatKey);
	logmsg("[INFO]: %s: Registered VSH Menu\n", __func__);
	int res = vctrlVSHRegisterVshGuMenu(vshmenu_draw);
	logmsg("[INFO]: %s: Registered VSH-GU Menu -> 0x%08X\n", __func__, res);

	vctrlGetRegistryValue("/CONFIG/SYSTEM/XMB", "button_assign", &g_button_assign_value);

	g_vshmenu.is_registered = 1;
	while (!g_vshmenu.stop_flag) {
		logmsg4("[INFO]: %s: Got in the loop\n", __func__);
		if (sceDisplayWaitVblankStart() < 0) {
			logmsg("[ERROR]: %s: Error on `sceDisplayWaitVblankStart`\n", __func__);
			// end of VSH ?
			break;
		}

		button_func();
	}
	g_vshmenu.is_registered = 0;

	logmsg("[INFO]: %s: GOT HERE\n", __func__);

	sctrlSESetConfig((SEConfig*)&g_cfw_config);
	vctrlVSHExitVSHMenu((SEConfig*)&g_cfw_config, NULL, 0);

	return sceKernelExitDeleteThread(0);
}

int module_start() {
	logInit("ms0:/log_satelite.txt");
	logmsg("Satelite started...\n")

	memset(&g_vshmenu, 0, sizeof(g_vshmenu));
    g_vshmenu.cur_buttons = 0xFFFFFFFF;
    g_vshmenu.stop_flag = 0;

	g_header_state.scroll = 1;
    g_header_state.sk = 150;
    g_window_w = calcWindowWidth();
    g_window_h = calcWindowHeight();

	strcpy(g_current_header_text, MAIN_HEADER);

	g_vshmenu_thid = sceKernelCreateThread("VshMenu_Thread", VshMenu_Thread, 0x10, 0x1000, PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_VFPU, NULL);
	if (g_vshmenu_thid >= 0) {
		int res = sceKernelStartThread(g_vshmenu_thid, 0, NULL);

		if (res < 0) {
			logmsg("[ERROR]: %s: Failed to create VSH menu thread\n", __func__);
			return -1;
		}
	}

	return 0;
}

int module_stop() {
	g_vshmenu.stop_flag = 1;

	SceUInt timeout = 100000;
	if (sceKernelWaitThreadEnd(g_vshmenu_thid, &timeout) < 0) {
		sceKernelTerminateDeleteThread(g_vshmenu_thid);
	}

	return 0;
}

void _exit(int a) {
	sceKernelStopUnloadSelfModule(0, NULL, NULL, NULL);
}