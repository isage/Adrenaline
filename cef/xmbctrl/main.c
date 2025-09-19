
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
#define COMMON_H_SYSCLIB_USER
#include <common.h>
#include <vshctrl.h>
#include <psputility_sysparam.h>

#include "main.h"
#include "utils.h"

#include "list.h"
#include "plugins.h"
#include "settings.h"
#include "options.h"

PSP_MODULE_INFO("EPI-XmbControl", 0x0007, 1, 0);

extern List plugins;

int psp_model;
AdrenalineConfig se_config;

GetItem GetItemes[] = {
	{2, 0, "CPU/BUS clock speed in XMB", 0, 0, &se_config.vsh_cpu_speed},
	{3, 0, "CPU/BUS clock speed in PSP game/app", 0, 0, &se_config.app_cpu_speed},
	{4, 0, "UMDemu ISO driver", 0, 0, &se_config.umd_mode},
	{5, 0, "Skip Sony logo on coldboot", 1, 0, &se_config.skip_logo},
	{6, 0, "Skip Sony logo on gameboot", 1, 0, &se_config.skip_game_boot_logo},
	{7, 0, "Hide corrupt icons in game menu", 1, 0, &se_config.hide_corrupt},
	{8, 0, "Hide MAC address", 1, 0, &se_config.hide_mac_addr},
	{9, 0, "Hide DLCs in game menu", 1, 0, &se_config.hide_dlcs},
	{10, 0, "Hide PIC0/PIC1 in game menu", 1, 0, &se_config.hide_pic0pic1},
	{11, 0, "Autorun /PSP/GAME/BOOT/EBOOT.PBP", 1, 0, &se_config.startup_program},
	{12, 0, "Fake VSH Region", 1, 0, &se_config.fake_region},
	{13, 0, "Extended screen colors mode", 1, 0, &se_config.extended_colors},
	{14, 0, "Use Sony PSP OSK", 1, 0, &se_config.use_sony_psposk},
	{15, 1, "NoDRM engine", 1, 0, &se_config.no_nodrm_engine},
	{16, 1, "XMB Control", 1, 0, &se_config.no_xmbctrl},
	{17, 0, "Force high memory layout", 1, 1, &se_config.force_high_memory},
	{18, 0, "Execute BOOT.BIN in UMDemu ISO", 1, 1, &se_config.execute_boot_bin},
	{19, 0, "Inferno ISO cache policy", 0, 1, &se_config.iso_cache},
	{20, 0, "Inferno ISO cache number", 0, 1, &se_config.iso_cache_num},
	{21, 0, "Inferno ISO cache size", 0, 1, &se_config.iso_cache_size},
	{22, 0, "UMDemu ISO seek time delay factor", 0, 1, &se_config.umd_seek},
	{23, 0, "UMDemu ISO read speed delay factor", 0, 1, &se_config.umd_speed},
	{24, 1, "XMB Plugins", 1, 1, &se_config.no_xmb_plugins},
	{25, 1, "Game Plugins", 1, 1, &se_config.no_game_plugins},
	{26, 1, "POPS Plugins", 1, 1, &se_config.no_pops_plugins},
};

#define N_ITEMS (sizeof(GetItemes) / sizeof(GetItem))

ItemOptions item_opts[] = {
	{0, NULL},                      // None
	{3, plugins_options},           // Plugins
	{9, cpuspeeds_options},         // Clock VSH
	{9, cpuspeeds_options},         // Clock Game
	{3, umd_driver_options},        // UMD driver
	{2, boolean_options},           // Skip ColdBoot logo
	{2, boolean_options},           // Skip GameBoot logo
	{2, boolean_options},           // Hide corrupt icons
	{2, boolean_options},           // Hide MAC address
	{2, boolean_options},           // Hide DLCs in game menu
	{NELEMS(hide_pics_options), hide_pics_options}, // Hide PIC0/PIC1.PNG in game menu
	{2, boolean_options},           // Autorun /PSP/GAME/BOOT/EBOOT.PBP
	{NELEMS(region_options), region_options}, // Fake region
	{3, use_extended_color_option}, // Use extended color
	{2, boolean_options},           // Use Sony PSP OSK
	{2, boolean_options},           // Use NoDRM Engine
	{2, boolean_options},           // XMB Control
	{3, highmem_options},           // Force high memory layout
	{2, boolean_options},           // Execute BOOT.BIN in Game
	{NELEMS(iso_cache_options), iso_cache_options}, // ISO cache kind
	{NELEMS(iso_cache_num_options), iso_cache_num_options}, // ISO cache number
	{NELEMS(iso_cache_size_options), iso_cache_size_options}, // ISO cache size
	{NELEMS(iso_umd_seek_read_options), iso_umd_seek_read_options}, // ISO UMD seek delay
	{NELEMS(iso_umd_seek_read_options), iso_umd_seek_read_options}, // ISO UMD read speed delay
	{2, boolean_options},           // VSH/XMB Plugins
	{2, boolean_options},           // Game Plugins
	{2, boolean_options},           // POPS Plugins
};

STMOD_HANDLER previous = NULL;
int num_items = N_ITEMS;

int OnModuleStart(SceModule *mod) {
	char *modname = mod->modname;
	u32 text_addr = mod->text_addr;
	u32 text_size = mod->text_size;

	if (strcmp(modname, "vsh_module") == 0)
		PatchVshMain(text_addr, text_size);
	else if (strcmp(modname, "sceVshAuthPlugin_Module") == 0)
		PatchAuthPlugin(text_addr, text_size);
	else if (strcmp(modname, "sysconf_plugin_module") == 0)
		PatchSysconfPlugin(text_addr, text_size);

	if (previous)
		previous(mod);

	return 0;
}

int module_start(SceSize args, void *argp) {
	psp_model = kuKernelGetModel();

	sctrlSEGetConfig(&se_config);

	previous = sctrlHENSetStartModuleHandler(OnModuleStart);

	return 0;
}
