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

#include <pspctrl.h>
#include <pspkerneltypes.h>

#include <systemctrl_se.h>
#include <systemctrl_adrenaline.h>

#include "main.h"
#include "menu.h"
#include "options.h"
#include "plugins.h"

#include "../../adrenaline_version.h"

#define CTRL_DELAY   100000
#define CTRL_DEADZONE_DELAY 500000

typedef struct {
	Entry *entries;
	int n_entries;
	char *title;
} Page;

static int g_sel = 0;
static int g_sel_page = 0;
u8 g_theme = 0;

static u32 g_last_btn = 0;
static u32 g_last_tick = 0;
static u32 g_deadzone_tick = 0;

static Theme g_themes[] = {
	// Default (done)
	{
		.accent_color = 0x4,
		.text_color = 0x7,
		.main_bg = 0x1,
		.select_text_color = 0x7,
		.select_text_bg =0x15,
		.panel_text = 0x15,
		.panel_bg = 0x7,
		.panel_select_text = 0xF,
		.panel_select_bg = 0x1,
		.dialog_text = 0x15,
		.dialog_bg = 0x7,
		.default_value_color = 0xA,
		.changed_value_color = 0xC
	},
	// Green (done)
	{
		.accent_color = 0xF,
		.text_color = 0xA,
		.main_bg = 0x7A,
		.select_text_color = 0x7A,
		.select_text_bg =0xA,
		.panel_text = 0x7,
		.panel_bg = 0x9D,
		.panel_select_text = 0x2F,
		.panel_select_bg = 0x79,
		.dialog_text = 0x7A,
		.dialog_bg = 0xA,
		.default_value_color = 0xA,
		.changed_value_color = 0xC

	},
	// Red (done)
	{
		.accent_color = 0xF,
		.text_color = 0x28,
		.main_bg = 0x88,
		.select_text_color = 0x88,
		.select_text_bg =0x28,
		.panel_text = 0x7,
		.panel_bg = 0x9D,
		.panel_select_text = 0x27,
		.panel_select_bg = 0x71,
		.dialog_text = 0x88,
		.dialog_bg = 0x28,
		.default_value_color = 0xA,
		.changed_value_color = 0xC

	},
	// Gray (done)
	{
		.accent_color = 0x1E,
		.text_color = 0x15,
		.main_bg = 0x1B,
		.select_text_color = 0x1E,
		.select_text_bg =0x15,

		.panel_text = 0x1B,
		.panel_bg = 0x13,

		.panel_select_text = 0x1C,
		.panel_select_bg = 0x12,

		.dialog_text = 0x1B,
		.dialog_bg = 0x15,
		.default_value_color = 0xA,
		.changed_value_color = 0xC
	},
	// Pink (done)
	{
		.accent_color = 0xF,
		.text_color = 0x3E,
		.main_bg = 0x9E,
		.select_text_color = 0x9E,
		.select_text_bg =0x3E,
		.panel_text = 0x7,
		.panel_bg = 0x9D,
		.panel_select_text = 0x3E,
		.panel_select_bg = 0x9E,
		.dialog_text = 0x9E,
		.dialog_bg = 0x3E,
		.default_value_color = 0xA,
		.changed_value_color = 0xC
	},
	// Purple (done)
	{
		.accent_color = 0xF,
		.text_color = 0x24,
		.main_bg = 0x84,
		.select_text_color = 0x84,
		.select_text_bg =0x24,
		.panel_text = 0x7,
		.panel_bg = 0x9D,
		.panel_select_text = 0x3E,
		.panel_select_bg = 0x85,
		.dialog_text = 0x84,
		.dialog_bg = 0x24,
		.default_value_color = 0xA,
		.changed_value_color = 0xC
	},
	// Cyan
	{
		.accent_color = 0xF,
		.text_color = 0x0B,
		.main_bg = 0x7C,
		.select_text_color = 0x7C,
		.select_text_bg =0x0B,
		.panel_text = 0x7,
		.panel_bg = 0x9D,
		.panel_select_text = 0x0B,
		.panel_select_bg = 0x7D,
		.dialog_text = 0x7C,
		.dialog_bg = 0x0B,
		.default_value_color = 0xA,
		.changed_value_color = 0xC
	},
	// Orange
	{
		.accent_color = 0xF,
		.text_color = 0x2A,
		.main_bg = 0x8A,
		.select_text_color = 0x8A,
		.select_text_bg =0x2A,
		.panel_text = 0x7,
		.panel_bg = 0x9D,
		.panel_select_text = 0x2A,
		.panel_select_bg = 0x85,
		.dialog_text = 0x8A,
		.dialog_bg = 0x2A,
		.default_value_color = 0xA,
		.changed_value_color = 0xC
	},
	// Yellow
	{
		.accent_color = 0xF,
		.text_color = 0x2C,
		.main_bg = 0x8C,
		.select_text_color = 0x8C,
		.select_text_bg =0x2C,
		.panel_text = 0x7,
		.panel_bg = 0x9D,
		.panel_select_text = 0x2C,
		.panel_select_bg = 0x8B,
		.dialog_text = 0x8C,
		.dialog_bg = 0x2C,
		.default_value_color = 0xA,
		.changed_value_color = 0xC
	},
	// B/W
	{
		.accent_color = 0x4,
		.text_color = 0xF,
		.main_bg = 0x0,
		.select_text_color = 0x0,
		.select_text_bg =0xF,
		.panel_text = 0xF,
		.panel_bg = 0x0,
		.panel_select_text = 0x4,
		.panel_select_bg = 0x0,
		.dialog_text = 0x0,
		.dialog_bg = 0xF,
		.default_value_color = 0xA,
		.changed_value_color = 0xC
	},

	// B/R
	{
		.accent_color = 0xF,
		.text_color = 0x4,
		.main_bg = 0x0,
		.select_text_color = 0x0,
		.select_text_bg =0x4,
		.panel_text = 0x4,
		.panel_bg = 0x0,
		.panel_select_text = 0xF,
		.panel_select_bg = 0x0,
		.dialog_text = 0x0,
		.dialog_bg = 0x4,
		.default_value_color = 0xA,
		.changed_value_color = 0xC
	},

	// B/G
	{
		.accent_color = 0xF,
		.text_color = 0x2,
		.main_bg = 0x0,
		.select_text_color = 0x0,
		.select_text_bg =0x2,
		.panel_text = 0x2,
		.panel_bg = 0x0,
		.panel_select_text = 0xF,
		.panel_select_bg = 0x0,
		.dialog_text = 0x0,
		.dialog_bg = 0x2,
		.default_value_color = 0xA,
		.changed_value_color = 0xC
	},
	// B/B
	{
		.accent_color = 0xF,
		.text_color = 0x1,
		.main_bg = 0x0,
		.select_text_color = 0xF,
		.select_text_bg =0x1,
		.panel_text = 0x1,
		.panel_bg = 0x0,
		.panel_select_text = 0xF,
		.panel_select_bg = 0x0,
		.dialog_text = 0xF,
		.dialog_bg = 0x1,
		.default_value_color = 0xA,
		.changed_value_color = 0xC
	},
};

static char *g_theme_names[] = {
	"Default", // "Blue",
	"Green",
	"Red",
	"Gray",
	"Pink",
	"Purple",
	"Cyan",
	"Orange",
	"Yellow",
	"B/White",
	"B/Red",
	"B/Green",
	"B/Blue",
};

static Entry g_general_entries[] = {
	{ "CPU/BUS clock speed in XMB", NULL, g_cpuspeeds, sizeof(g_cpuspeeds), &g_cfw_config.vsh_cpu_speed },
	{ "CPU/BUS clock speed in PSP game/app", NULL, g_cpuspeeds, sizeof(g_cpuspeeds), &g_cfw_config.app_cpu_speed },
	{ "UMDemu ISO driver", NULL, g_umdmodes, sizeof(g_umdmodes), &g_cfw_config.umd_mode },
	{ "Skip Sony logo on coldboot", NULL, g_disenabled, sizeof(g_disenabled), &g_cfw_config.skip_logo },
	{ "Skip Sony logo on gameboot", NULL, g_disenabled, sizeof(g_disenabled), &g_cfw_config.skip_game_boot_logo },
	{ "Hide corrupt icons in game menu", NULL, g_disenabled, sizeof(g_disenabled), &g_cfw_config.hide_corrupt },
	{ "Hide DLCs in game menu", NULL, g_disenabled, sizeof(g_disenabled), &g_cfw_config.hide_dlcs },
	{ "Hide PIC0/PIC1 in game menu", NULL, g_hide_pics, sizeof(g_hide_pics), &g_cfw_config.hide_pic0pic1 },
	{ "Hide MAC address", NULL, g_disenabled, sizeof(g_disenabled), &g_cfw_config.hide_mac_addr },
	{ "Hide CFW files in PSP games", NULL, g_endisabled, sizeof(g_endisabled), &g_cfw_config.no_hide_cfw_files },
	{ "Autorun /PSP/GAME/BOOT/EBOOT.PBP", NULL, g_disenabled, sizeof(g_disenabled), &g_cfw_config.startup_program },
	{ "Fake VSH Region", NULL, g_regions, sizeof(g_regions), &g_cfw_config.fake_region },
	{ "Extended screen colors mode", NULL, g_extendedcolors, sizeof(g_extendedcolors), &g_cfw_config.extended_colors },
	{ "Recovery color", Setrecovery_color, g_theme_names, sizeof(g_theme_names), &g_cfw_config.recovery_color },
	{ "Use Sony PSP OSK", NULL, g_disenabled, sizeof(g_disenabled), &g_cfw_config.use_sony_psposk },
	{ "Memory Stick Cache", NULL, g_endisabled, sizeof(g_endisabled), &g_cfw_config.no_ms_cache },
	{ "NoDRM engine", NULL, g_endisabled, sizeof(g_endisabled), &g_cfw_config.no_nodrm_engine },
	{ "XMB Control", NULL, g_endisabled, sizeof(g_endisabled), &g_cfw_config.no_xmbctrl },
	{ "", NULL, NULL, 0, NULL },
	{ "Toggle USB", ToggleUSB, NULL, 0, NULL },
	{ "", NULL, NULL, 0, NULL },
	{ "Exit", Exit, NULL, 0, NULL },
};

static Entry g_advanced_entries[] = {
	{ "Force high memory layout", NULL, g_highmem, sizeof(g_highmem), &g_cfw_config.force_high_memory },
	{ "Fake max free memory", NULL, g_fake_free_mem, sizeof(g_fake_free_mem), &g_cfw_config.fake_max_free_mem },
	{ "Use Graphic Engine 2", NULL, g_disenabled, sizeof(g_disenabled), &g_cfw_config.use_ge2 },
	{ "Use Media Engine 2", NULL, g_disenabled, sizeof(g_disenabled), &g_cfw_config.use_me2 },
	{ "Execute BOOT.BIN in UMDemu ISO", NULL, g_disenabled, sizeof(g_disenabled), &g_cfw_config.execute_boot_bin },
	{ "Inferno ISO cache policy", NULL, g_iso_cache, sizeof(g_iso_cache), &g_cfw_config.iso_cache },
	{ "Inferno ISO cache number", NULL, g_iso_cache_num, sizeof(g_iso_cache_num), &g_cfw_config.iso_cache_num },
	{ "Inferno ISO cache size", NULL, g_iso_cache_size, sizeof(g_iso_cache_size), &g_cfw_config.iso_cache_size },
	{ "UMDemu  ISO seek time delay factor", NULL, g_umd_seek_read_delay, sizeof(g_umd_seek_read_delay), &g_cfw_config.umd_seek },
	{ "UMDemu  ISO read speed delay factor", NULL, g_umd_seek_read_delay, sizeof(g_umd_seek_read_delay), &g_cfw_config.umd_speed },
	{ "TTY redirection", NULL, g_disenabled, sizeof(g_disenabled), &g_cfw_config.tty_redirection },
	{ "XMB  plugins", NULL, g_endisabled, sizeof(g_endisabled), &g_cfw_config.no_xmb_plugins },
	{ "GAME plugins", NULL, g_endisabled, sizeof(g_endisabled), &g_cfw_config.no_game_plugins },
	{ "POPS plugins", NULL, g_endisabled, sizeof(g_endisabled), &g_cfw_config.no_pops_plugins },
	{ "", NULL, NULL, 0, NULL },
	{ "Button assign", SetButtonAssign, g_buttonassign, sizeof(g_buttonassign), (int *)&g_button_assign_value },
	{ "Import classic plugins", ImportClassicPlugins, NULL, 0, NULL },
	{ "Activate WMA", SetWMA, NULL, 0, NULL },
	{ "Activate Flash Player", SetFlashPlayer, NULL, 0, NULL },
	{ "", NULL, NULL, 0, NULL },
	{ "Reset settings", ResetSettings, NULL, 0, NULL },
};

#define N_GENERAL (sizeof(g_general_entries) / sizeof(Entry))
#define N_ADVANCED (sizeof(g_advanced_entries) / sizeof(Entry))

static Page g_pages[] = {
	{ g_general_entries, N_GENERAL, "General" },
	{ g_advanced_entries, N_ADVANCED, "Advanced" },
	{ g_plugins_tool_entries, 0, "Plugins" }
};

void UpdatePluginCount(int count) {
	g_pages[2].n_entries = count;
}

u32 ReadKey(void) {
	SceCtrlData pad;

	sceCtrlReadBufferPositive(&pad, 1);

	if (pad.Buttons == g_last_btn) {
		if (pad.TimeStamp - g_deadzone_tick < CTRL_DEADZONE_DELAY) {
			return 0;
		}

		if (pad.TimeStamp - g_last_tick < CTRL_DELAY) {
			return 0;
		}

		g_last_tick = pad.TimeStamp;

		return g_last_btn;
	}

	g_last_btn = pad.Buttons;
	g_deadzone_tick = g_last_tick = pad.TimeStamp;

	return g_last_btn;
}

void WaitPress(u32 buttons) {
	while ((ReadKey() & buttons) != buttons);
}

void ChangeValue(int interval) {
	if (g_pages[g_sel_page].entries[g_sel].options) {
		int max = g_pages[g_sel_page].entries[g_sel].size_options / sizeof(char **);
		(*g_pages[g_sel_page].entries[g_sel].value) = (max + (*g_pages[g_sel_page].entries[g_sel].value) + interval) % max;
	}

	if (g_pages[g_sel_page].entries[g_sel].function) {
		g_pages[g_sel_page].entries[g_sel].function(g_sel);
	}
}

void DrawHeader() {
	VGraphSetBackColor(g_themes[g_theme].main_bg);
//  VGraphClear();
	VGraphGoto(0, 0);

	VGraphClearLine(g_themes[g_theme].panel_bg);

	VGraphSetTextColor(g_themes[g_theme].accent_color, g_themes[g_theme].select_text_bg);
	VGraphPrintf(" \x11 L ");

	for (int i = 0; i < sizeof(g_pages) / sizeof(Page);i++) {
		if (g_sel_page == i) {
			VGraphSetTextColor(g_themes[g_theme].panel_select_text, g_themes[g_theme].panel_select_bg);
		} else {
			VGraphSetTextColor(g_themes[g_theme].panel_text, g_themes[g_theme].panel_bg);
		}
		VGraphPutc(' ');

		if (g_sel_page == i) {
			VGraphSetTextColor(g_themes[g_theme].accent_color, g_themes[g_theme].panel_select_bg);
		} else {
			VGraphSetTextColor(g_themes[g_theme].accent_color, g_themes[g_theme].panel_bg);
		}

		VGraphPutc(g_pages[i].title[0]);

		if (g_sel_page == i) {
			VGraphSetTextColor(g_themes[g_theme].panel_select_text, g_themes[g_theme].panel_select_bg);
		} else {
			VGraphSetTextColor(g_themes[g_theme].panel_text, g_themes[g_theme].panel_bg);
		}

		VGraphPrintf(g_pages[i].title+1);
		VGraphPutc(' ');
	}

	VGraphGoto(55, 0);
	VGraphSetTextColor(g_themes[g_theme].accent_color, g_themes[g_theme].select_text_bg);
	VGraphPrintf(" R \x10 ");

	VGraphSetTextColor(g_themes[g_theme].text_color, g_themes[g_theme].main_bg);
	VGraphPrintf("\xC9");
	for (int i = 1; i < 59; i++) {
		VGraphPrintf("\xCD");
	}
	VGraphPrintf("\xBB");
}

void DrawFooter() {
	VGraphGoto(0, 32);
	VGraphSetTextColor(g_themes[g_theme].text_color, g_themes[g_theme].main_bg);
	VGraphPrintf("\xC8");
	for (int i = 1; i < 59; i++) {
		VGraphPrintf("\xCD");
	}
	VGraphPrintf("\xBC");

	VGraphClearLine(g_themes[g_theme].panel_bg);
	VGraphSetTextColor(g_themes[g_theme].accent_color, g_themes[g_theme].panel_bg);
	VGraphPrintf("E");
	VGraphSetTextColor(g_themes[g_theme].panel_text, g_themes[g_theme].panel_bg);
	#if defined(NIGHTLY) && NIGHTLY == 1
	VGraphPrintf("pinephrine Recovery Menu v%d.%d.%d-%s", ADRENALINE_VERSION_MAJOR, ADRENALINE_VERSION_MINOR, ADRENALINE_VERSION_MICRO, EPI_NIGHTLY_VER);
	#else
	VGraphPrintf("pinephrine Recovery Menu v%d.%d.%d", ADRENALINE_VERSION_MAJOR, ADRENALINE_VERSION_MINOR, ADRENALINE_VERSION_MICRO);
	#endif
}

void ShowDialog(char* message) {
	VGraphSetTextColor(g_themes[g_theme].dialog_text, g_themes[g_theme].dialog_bg);
	VGraphGoto(18, 10);
	VGraphPrintf("\xC9");
	for (int i = 1; i < 21; i++) {
		VGraphPrintf("\xCD");
	}
	VGraphPrintf("\xBB");
	VGraphGoto(18, 11);

	VGraphPrintf("\xBA  ");
	VGraphPrintf("%*s",(18 - strlen(message)) / 2, " ");
	VGraphPrintf(message);
	VGraphPrintf("%*s", 18 - ((18-strlen(message)) / 2) - strlen(message) , " ");
	VGraphPrintf("\xBA");
	VGraphGoto(18, 12);

	VGraphPrintf("\xBA  ");
	VGraphPrintf("%18s"," ");
	VGraphPrintf("\xBA");
	VGraphGoto(18, 13);

	VGraphPrintf("\xBA  ");
	VGraphPrintf("%*s", (18-7) / 2, " ");
	VGraphPrintf("Press X");
	VGraphPrintf("%*s", 18 - ((18-7) / 2) - 7 , " ");
	VGraphPrintf("\xBA");
	VGraphGoto(18, 14);

	VGraphPrintf("\xC8");
	for (int i = 1; i < 21; i++) {
		VGraphPrintf("\xCD");
	}
	VGraphPrintf("\xBC");

	VGraphSwap();

	WaitPress(PSP_CTRL_CROSS);
}

void MenuCtrl() {
	u32 key = ReadKey();
/*
	do {
		key = ReadKey();
		sceKernelDelayThread(1000);
	} while(!key);*/

	if ((key & PSP_CTRL_CROSS) || (key &  PSP_CTRL_RIGHT)) {
		ChangeValue(+1);
	}
	if (key & PSP_CTRL_LEFT) {
		ChangeValue(-1);
	}

	if (key & PSP_CTRL_UP) {
		g_sel = (g_pages[g_sel_page].n_entries + g_sel - 1) % g_pages[g_sel_page].n_entries;
		// skip empty lines
		if (strcmp(g_pages[g_sel_page].entries[g_sel].name,"")==0)
			g_sel = (g_pages[g_sel_page].n_entries + g_sel - 1) % g_pages[g_sel_page].n_entries;
	}
	if (key & PSP_CTRL_DOWN) {
		g_sel = (g_pages[g_sel_page].n_entries + g_sel + 1) % g_pages[g_sel_page].n_entries;
		// skip empty lines
		if (strcmp(g_pages[g_sel_page].entries[g_sel].name,"")==0)
			g_sel = (g_pages[g_sel_page].n_entries + g_sel + 1) % g_pages[g_sel_page].n_entries;
	}

	if (key & PSP_CTRL_LTRIGGER) {
		g_sel_page = ((sizeof(g_pages) / sizeof(Page)) + g_sel_page - 1) % (sizeof(g_pages) / sizeof(Page));
		g_sel = 0;
	}
	if (key & PSP_CTRL_RTRIGGER) {
		g_sel_page = ((sizeof(g_pages) / sizeof(Page)) + g_sel_page + 1) % (sizeof(g_pages) / sizeof(Page));
		g_sel = 0;
	}
}

void MenuLoop() {
	DrawHeader();

	VGraphSetTextColor(g_themes[g_theme].text_color, g_themes[g_theme].main_bg);
	for (int i = 2; i < 32; i++) {
		VGraphGoto(0, i);
		VGraphClearLine(g_themes[g_theme].main_bg);
		VGraphPrintf("\xBA");
		VGraphGoto(59, i);
		VGraphPrintf("\xBA");
	}

	for (int i = 0; i < g_pages[g_sel_page].n_entries; i++) {
		VGraphGoto(3, 5 + i);

		if (g_sel == i) {
			VGraphSetTextColor(g_themes[g_theme].accent_color, g_themes[g_theme].main_bg);
			VGraphPrintf("\x10");
		} else {
			VGraphSetTextColor(g_themes[g_theme].main_bg, g_themes[g_theme].main_bg);
			VGraphPrintf(" ");
		}

		VGraphSetTextColor(g_sel == i ? g_themes[g_theme].select_text_color : g_themes[g_theme].text_color, g_sel == i ? g_themes[g_theme].select_text_bg : g_themes[g_theme].main_bg);

		VGraphPrintf(g_pages[g_sel_page].entries[i].name);

		if (g_pages[g_sel_page].entries[i].options) {
			int max = g_pages[g_sel_page].entries[i].size_options / sizeof(char **);
			VGraphGoto(3 + (60 - 22), 5 + i);
			VGraphSetTextColor((*g_pages[g_sel_page].entries[i].value) == 0 ? g_themes[g_theme].default_value_color : g_themes[g_theme].changed_value_color, g_themes[g_theme].main_bg);
			VGraphPrintf("%14s", g_pages[g_sel_page].entries[i].options[(*g_pages[g_sel_page].entries[i].value) % max]);
		}
	}

	DrawFooter();
	VGraphSwap();

	MenuCtrl();
}
