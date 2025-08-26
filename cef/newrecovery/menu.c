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

#include <common.h>

#include "main.h"
#include "menu.h"
#include "options.h"
#include "plugins.h"

typedef struct {
	Entry *entries;
	int n_entries;
	char *title;
} Page;

int sel = 0;
int sel_page = 0;
u8 theme = 0;

static u32 g_last_btn = 0;
static u32 g_last_tick = 0;
static u32 g_deadzone_tick = 0;
#define CTRL_DELAY   100000
#define CTRL_DEADZONE_DELAY 500000

extern u32 button_assign_value;
extern Entry plugins_tool_entries[MAX_PLUGINS + 1];

Theme themes[] = {
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

static char *theme_names[] = {
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

Entry general_entries[] = {
  { "CPU/BUS clock speed in XMB", NULL, cpuspeeds, sizeof(cpuspeeds), &config.vsh_cpu_speed },
  { "CPU/BUS clock speed in PSP game/app", NULL, cpuspeeds, sizeof(cpuspeeds), &config.app_cpu_speed },
  { "UMDemu ISO driver", NULL, umdmodes, sizeof(umdmodes), &config.umd_mode },
  { "Skip Sony logo on coldboot", NULL, disenabled, sizeof(disenabled), &config.skip_logo },
  { "Skip Sony logo on gameboot", NULL, disenabled, sizeof(disenabled), &config.skip_game_boot_logo },
  { "Hide corrupt icons in game menu", NULL, disenabled, sizeof(disenabled), &config.hide_corrupt },
  { "Hide MAC address", NULL, disenabled, sizeof(disenabled), &config.hide_mac_addr },
  { "Hide DLCs in game menu", NULL, disenabled, sizeof(disenabled), &config.hide_dlcs },
  { "Hide PIC0/PIC1 in game menu", NULL, disenabled, sizeof(disenabled), &config.hide_pic0pic1 },
  { "Autorun /PSP/GAME/BOOT/EBOOT.PBP", NULL, disenabled, sizeof(disenabled), &config.startup_program },
  { "Fake VSH Region", NULL, regions, sizeof(regions), &config.fake_region },
  { "Extended screen colors mode", NULL, extendedcolors, sizeof(extendedcolors), &config.extended_colors },
  { "Recovery color", Setrecovery_color, theme_names, sizeof(theme_names), &config.recovery_color },
  { "Use Sony PSP OSK", NULL, disenabled, sizeof(disenabled), &config.use_sony_psposk },
  { "NoDRM engine", NULL, endisabled, sizeof(endisabled), &config.no_nodrm_engine },
  { "XMB Control", NULL, endisabled, sizeof(endisabled), &config.no_xmbctrl },
  { "", NULL, NULL, 0, NULL },
  { "Toggle USB", ToggleUSB, NULL, 0, NULL },
  { "", NULL, NULL, 0, NULL },
  { "Exit", Exit, NULL, 0, NULL },
};

Entry advanced_entries[] = {
  { "Force high memory layout", NULL, highmem, sizeof(highmem), &config.force_high_memory },
  { "Execute BOOT.BIN in UMD/ISO", NULL, disenabled, sizeof(disenabled), &config.execute_boot_bin },
  { "Inferno ISO cache policy", NULL, iso_cache, sizeof(iso_cache), &config.iso_cache },
  { "Inferno ISO cache number", NULL, iso_cache_num, sizeof(iso_cache_num), &config.iso_cache_num },
  { "Inferno ISO cache size", NULL, iso_cache_size, sizeof(iso_cache_size), &config.iso_cache_size },
  { "UMDemu  ISO seek time delay factor", NULL, umd_seek_read_delay, sizeof(umd_seek_read_delay), &config.umd_seek },
  { "UMDemu  ISO read speed delay factor", NULL, umd_seek_read_delay, sizeof(umd_seek_read_delay), &config.umd_speed },
  { "XMB  plugins", NULL, endisabled, sizeof(endisabled), &config.no_xmb_plugins },
  { "GAME plugins", NULL, endisabled, sizeof(endisabled), &config.no_game_plugins },
  { "POPS plugins", NULL, endisabled, sizeof(endisabled), &config.no_pops_plugins },
  { "", NULL, NULL, 0, NULL },
  { "Button assign", SetButtonAssign, buttonassign, sizeof(buttonassign), (int *)&button_assign_value },
  { "Import classic plugins", ImportClassicPlugins, NULL, 0, NULL },
  { "Activate WMA", SetWMA, NULL, 0, NULL },
  { "Activate Flash Player", SetFlashPlayer, NULL, 0, NULL },
  { "", NULL, NULL, 0, NULL },
  { "Reset settings", ResetSettings, NULL, 0, NULL },
};

#define N_GENERAL (sizeof(general_entries) / sizeof(Entry))
#define N_ADVANCED (sizeof(advanced_entries) / sizeof(Entry))

Page pages[] = {
  { general_entries, N_GENERAL, "General" },
  { advanced_entries, N_ADVANCED, "Advanced" },
  { plugins_tool_entries, 0, "Plugins" }
};

void UpdatePluginCount(int count)
{
  pages[2].n_entries = count;
}

u32 ReadKey(void)
{
  SceCtrlData pad;

  sceCtrlReadBufferPositive(&pad, 1);

  if (pad.Buttons == g_last_btn)
  {
    if (pad.TimeStamp - g_deadzone_tick < CTRL_DEADZONE_DELAY)
    {
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

void WaitPress(u32 buttons)
{
  while ((ReadKey() & buttons) != buttons);
}

void ChangeValue(int interval)
{
  if (pages[sel_page].entries[sel].options)
  {
    int max = pages[sel_page].entries[sel].size_options / sizeof(char **);
    (*pages[sel_page].entries[sel].value) = (max + (*pages[sel_page].entries[sel].value) + interval) % max;
  }

  if (pages[sel_page].entries[sel].function)
  {
    pages[sel_page].entries[sel].function(sel);
  }
}

void DrawHeader()
{
  VGraphSetBackColor(themes[theme].main_bg);
//  VGraphClear();
  VGraphGoto(0, 0);

  VGraphClearLine(themes[theme].panel_bg);

  VGraphSetTextColor(themes[theme].accent_color, themes[theme].select_text_bg);
  VGraphPrintf(" \x11 L ");

  for (int i = 0; i < sizeof(pages) / sizeof(Page);i++)
  {
    if (sel_page == i)
    {
      VGraphSetTextColor(themes[theme].panel_select_text, themes[theme].panel_select_bg);
    }
    else
    {
      VGraphSetTextColor(themes[theme].panel_text, themes[theme].panel_bg);
    }
    VGraphPutc(' ');

    if (sel_page == i)
    {
      VGraphSetTextColor(themes[theme].accent_color, themes[theme].panel_select_bg);
    }
    else
    {
      VGraphSetTextColor(themes[theme].accent_color, themes[theme].panel_bg);
    }

    VGraphPutc(pages[i].title[0]);

    if (sel_page == i)
    {
      VGraphSetTextColor(themes[theme].panel_select_text, themes[theme].panel_select_bg);
    }
    else
    {
      VGraphSetTextColor(themes[theme].panel_text, themes[theme].panel_bg);
    }

    VGraphPrintf(pages[i].title+1);
    VGraphPutc(' ');
  }

  VGraphGoto(55, 0);
  VGraphSetTextColor(themes[theme].accent_color, themes[theme].select_text_bg);
  VGraphPrintf(" R \x10 ");

  VGraphSetTextColor(themes[theme].text_color, themes[theme].main_bg);
  VGraphPrintf("\xC9");
  for (int i = 1; i < 59; i++)
    VGraphPrintf("\xCD");
  VGraphPrintf("\xBB");
}

void DrawFooter()
{
  VGraphGoto(0, 32);
  VGraphSetTextColor(themes[theme].text_color, themes[theme].main_bg);
  VGraphPrintf("\xC8");
  for (int i = 1; i < 59; i++)
    VGraphPrintf("\xCD");
  VGraphPrintf("\xBC");

  VGraphClearLine(themes[theme].panel_bg);
  VGraphSetTextColor(themes[theme].accent_color, themes[theme].panel_bg);
  VGraphPrintf("A");
  VGraphSetTextColor(themes[theme].panel_text, themes[theme].panel_bg);
  VGraphPrintf("drenaline Recovery Menu v%d.%d.%d", ADRENALINE_VERSION_MAJOR, ADRENALINE_VERSION_MINOR, ADRENALINE_VERSION_MICRO);
}

void ShowDialog(char* message)
{
  VGraphSetTextColor(themes[theme].dialog_text, themes[theme].dialog_bg);
  VGraphGoto(18, 10);
  VGraphPrintf("\xC9");
  for (int i = 1; i < 21; i++)
    VGraphPrintf("\xCD");
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
  for (int i = 1; i < 21; i++)
    VGraphPrintf("\xCD");
  VGraphPrintf("\xBC");

  VGraphSwap();

  WaitPress(PSP_CTRL_CROSS);
}

void MenuCtrl()
{
  u32 key = ReadKey();
/*
  do {
    key = ReadKey();
    sceKernelDelayThread(1000);
  } while(!key);*/

  if ((key & PSP_CTRL_CROSS) || (key &  PSP_CTRL_RIGHT))
  {
    ChangeValue(+1);
  }
  if (key & PSP_CTRL_LEFT)
  {
    ChangeValue(-1);
  }

  if (key & PSP_CTRL_UP)
  {
    sel = (pages[sel_page].n_entries + sel - 1) % pages[sel_page].n_entries;
    // skip empty lines
    if (strcmp(pages[sel_page].entries[sel].name,"")==0)
      sel = (pages[sel_page].n_entries + sel - 1) % pages[sel_page].n_entries;
  }
  if (key & PSP_CTRL_DOWN)
  {
    sel = (pages[sel_page].n_entries + sel + 1) % pages[sel_page].n_entries;
    // skip empty lines
    if (strcmp(pages[sel_page].entries[sel].name,"")==0)
      sel = (pages[sel_page].n_entries + sel + 1) % pages[sel_page].n_entries;
  }

  if (key & PSP_CTRL_LTRIGGER)
  {
    sel_page = ((sizeof(pages) / sizeof(Page)) + sel_page - 1) % (sizeof(pages) / sizeof(Page));
    sel = 0;
  }
  if (key & PSP_CTRL_RTRIGGER)
  {
    sel_page = ((sizeof(pages) / sizeof(Page)) + sel_page + 1) % (sizeof(pages) / sizeof(Page));
    sel = 0;
  }
}

void MenuLoop()
{
  DrawHeader();

  VGraphSetTextColor(themes[theme].text_color, themes[theme].main_bg);
  for (int i = 2; i < 32; i++) {
    VGraphGoto(0, i);
    VGraphClearLine(themes[theme].main_bg);
    VGraphPrintf("\xBA");
    VGraphGoto(59, i);
    VGraphPrintf("\xBA");
  }

  for (int i = 0; i < pages[sel_page].n_entries; i++)
  {
    VGraphGoto(3, 5 + i);

    if (sel == i)
    {
      VGraphSetTextColor(themes[theme].accent_color, themes[theme].main_bg);
      VGraphPrintf("\x10");
    }
    else
    {
      VGraphSetTextColor(themes[theme].main_bg, themes[theme].main_bg);
      VGraphPrintf(" ");
    }

    VGraphSetTextColor(sel == i ? themes[theme].select_text_color : themes[theme].text_color, sel == i ? themes[theme].select_text_bg : themes[theme].main_bg);

    VGraphPrintf(pages[sel_page].entries[i].name);

    if (pages[sel_page].entries[i].options)
    {
      int max = pages[sel_page].entries[i].size_options / sizeof(char **);
      VGraphGoto(3 + (60 - 22), 5 + i);
      VGraphSetTextColor((*pages[sel_page].entries[i].value) == 0 ? themes[theme].default_value_color : themes[theme].changed_value_color, themes[theme].main_bg);
      VGraphPrintf("%14s", pages[sel_page].entries[i].options[(*pages[sel_page].entries[i].value) % max]);
    }
  }

  DrawFooter();
  VGraphSwap();

  MenuCtrl();
}