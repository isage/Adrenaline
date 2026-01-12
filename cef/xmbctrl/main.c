
/*
	Adrenaline, XmbControl
	Copyright (C) 2011, Total_Noob
	Copyright (C) 2011, Frostegater
	Copyright (C) 2011, codestation
	Copyright (C) 2025, GrayJack

	main.c: XmbControl main code

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
#include <psputility_sysparam.h>

#include <vshctrl.h>
#include <systemctrl.h>
#include <cfwmacros.h>

#define _ADRENALINE_LOG_IMPL_
#include <adrenaline_log.h>

#include "xmbctrl.h"
#include "utils.h"

#include "list.h"
#include "plugins.h"
#include "options.h"

PSP_MODULE_INFO("EPI-XmbControl", 0x0007, 1, 0);

int g_psp_model;
SEConfigADR g_cfw_config;

GetItem g_menu_items[] = {
	{2, 0, "CPU/BUS clock speed in XMB", 0, 0, &g_cfw_config.vsh_cpu_speed},
	{3, 0, "CPU/BUS clock speed in PSP game/app", 0, 0, &g_cfw_config.app_cpu_speed},
	{4, 0, "UMDemu ISO driver", 0, 0, &g_cfw_config.umd_mode},
	{5, 0, "Skip Sony logo on coldboot", 1, 0, &g_cfw_config.skip_logo},
	{6, 0, "Skip Sony logo on gameboot", 1, 0, &g_cfw_config.skip_game_boot_logo},
	{7, 0, "Hide corrupt icons in game menu", 1, 0, &g_cfw_config.hide_corrupt},
	{8, 0, "Hide DLCs in game menu", 1, 0, &g_cfw_config.hide_dlcs},
	{9, 0, "Hide PIC0/PIC1 in game menu", 1, 0, &g_cfw_config.hide_pic0pic1},
	{10, 0, "Hide MAC address", 1, 0, &g_cfw_config.hide_mac_addr},
	{11, 1, "Hide CFW files in PSP games", 1, 0, &g_cfw_config.no_hide_cfw_files},
	{12, 0, "Autorun /PSP/GAME/BOOT/EBOOT.PBP", 1, 0, &g_cfw_config.startup_program},
	{13, 0, "Fake VSH Region", 1, 0, &g_cfw_config.fake_region},
	{14, 0, "Extended screen colors mode", 1, 0, &g_cfw_config.extended_colors},
	{15, 0, "Use Sony PSP OSK", 1, 0, &g_cfw_config.use_sony_psposk},
	{16, 1, "Memory Stick Cache", 1, 0, &g_cfw_config.no_ms_cache},
	{17, 1, "NoDRM engine", 1, 0, &g_cfw_config.no_nodrm_engine},
	{18, 1, "XMB Control", 1, 0, &g_cfw_config.no_xmbctrl},
	{19, 0, "Force high memory layout", 1, 1, &g_cfw_config.force_high_memory},
	{20, 0, "Use Graphic Engine 2", 1, 1, &g_cfw_config.use_ge2},
	{21, 0, "Use Media Engine 2", 1, 1, &g_cfw_config.use_me2},
	{22, 0, "Execute BOOT.BIN in UMDemu ISO", 1, 1, &g_cfw_config.execute_boot_bin},
	{23, 0, "Inferno ISO cache policy", 0, 1, &g_cfw_config.iso_cache},
	{24, 0, "Inferno ISO cache number", 0, 1, &g_cfw_config.iso_cache_num},
	{25, 0, "Inferno ISO cache size", 0, 1, &g_cfw_config.iso_cache_size},
	{26, 0, "UMDemu ISO seek time delay factor", 0, 1, &g_cfw_config.umd_seek},
	{27, 0, "UMDemu ISO read speed delay factor", 0, 1, &g_cfw_config.umd_speed},
	{28, 1, "XMB Plugins", 1, 1, &g_cfw_config.no_xmb_plugins},
	{29, 1, "Game Plugins", 1, 1, &g_cfw_config.no_game_plugins},
	{30, 1, "POPS Plugins", 1, 1, &g_cfw_config.no_pops_plugins},
};

#define N_ITEMS (sizeof(g_menu_items) / sizeof(GetItem))

ItemOptions g_item_opts[] = {
	{0, NULL},                      // None
	{3, g_plugins_options},           // Plugins
	{9, g_cpuspeeds_options},         // Clock VSH
	{9, g_cpuspeeds_options},         // Clock Game
	{3, g_umd_driver_options},        // UMD driver
	{2, g_boolean_options},           // Skip ColdBoot logo
	{2, g_boolean_options},           // Skip GameBoot logo
	{2, g_boolean_options},           // Hide corrupt icons
	{2, g_boolean_options},           // Hide DLCs in game menu
	{NELEMS(g_hide_pics_options), g_hide_pics_options}, // Hide PIC0/PIC1.PNG in game menu
	{2, g_boolean_options},           // Hide MAC address
	{2, g_boolean_options},           // Hide CFW files in PSP games
	{2, g_boolean_options},           // Autorun /PSP/GAME/BOOT/EBOOT.PBP
	{NELEMS(g_region_options), g_region_options}, // Fake region
	{3, g_use_extended_color_option}, // Use extended color
	{2, g_boolean_options},           // Use Sony PSP OSK
	{2, g_boolean_options},           // Memory Stick Cache
	{2, g_boolean_options},           // Use NoDRM Engine
	{2, g_boolean_options},           // XMB Control
	{3, g_highmem_options},           // Force high memory layout
	{2, g_boolean_options},           // Use Graphic Engine 2
	{2, g_boolean_options},           // Use Media Engine 2
	{2, g_boolean_options},           // Execute BOOT.BIN in Game
	{NELEMS(g_iso_cache_options), g_iso_cache_options}, // ISO cache kind
	{NELEMS(g_iso_cache_num_options), g_iso_cache_num_options}, // ISO cache number
	{NELEMS(g_iso_cache_size_options), g_iso_cache_size_options}, // ISO cache size
	{NELEMS(g_iso_umd_seek_read_options), g_iso_umd_seek_read_options}, // ISO UMD seek delay
	{NELEMS(g_iso_umd_seek_read_options), g_iso_umd_seek_read_options}, // ISO UMD read speed delay
	{2, g_boolean_options},           // VSH/XMB Plugins
	{2, g_boolean_options},           // Game Plugins
	{2, g_boolean_options},           // POPS Plugins
};

static STMOD_HANDLER previous = NULL;
int g_num_items = N_ITEMS;

int OnModuleStart(SceModule *mod) {
	char *modname = mod->modname;
	u32 text_addr = mod->text_addr;
	u32 text_size = mod->text_size;

	if (paf_strcmp(modname, "vsh_module") == 0)
		PatchVshMain(text_addr, text_size);
	else if (paf_strcmp(modname, "sceVshAuthPlugin_Module") == 0)
		PatchAuthPlugin(text_addr, text_size);
	else if (paf_strcmp(modname, "sysconf_plugin_module") == 0)
		PatchSysconfPlugin(text_addr, text_size);

	if (previous) {
		previous(mod);
	}

	return 0;
}

int module_start(SceSize args, void *argp) {
	logInit("ms0:/log_xmbctrl.txt");
	logmsg("XMBControl started...\n")

	g_psp_model = kuKernelGetModel();

	sctrlSEGetConfig(&g_cfw_config);

	// Sanity check
	// `g_item_opts` always should have num of `g_menu_items` + 2
	if (NELEMS(g_menu_items) != (NELEMS(g_item_opts)-2)) {
		logmsg("[ERROR]: CFW options not properly set\n");
		return -1;
	}

	previous = sctrlHENSetStartModuleHandler(OnModuleStart);

	return 0;
}
