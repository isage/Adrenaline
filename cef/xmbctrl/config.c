#include <pspsdk.h>
#include <pspkernel.h>
#include <psputility_sysparam.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <kubridge.h>
#include <sysclib_user.h>

#include "main.h"
#include "list.h"
#include "settings.h"

extern CFWConfig config;
extern AdrenalineConfig se_config;

void loadSettings() {
	memset(&config, 0, sizeof(config));

	sctrlSEGetConfig(&se_config);

	config.vsh_cpu_speed = se_config.vsh_cpu_speed;
	config.game_cpu_speed = se_config.app_cpu_speed;
	config.umd_driver = se_config.umd_mode;
	config.skip_sony_coldboot_logo = se_config.skip_logo;
	config.skip_sony_gameboot_logo = se_config.skip_game_boot_logo;
	config.hide_corrupt_icons = se_config.hide_corrupt;
	config.hide_mac_addr = se_config.hide_mac_addr;
	config.hide_dlcs = se_config.hide_dlcs;
	config.hide_pic01 = se_config.hide_pic0pic1;
	config.use_sony_osk = se_config.use_sony_psposk;
	config.autorun_boot_eboot = se_config.startup_program;
	config.force_highmem = se_config.force_high_memory;
	config.exec_bootbin = se_config.execute_boot_bin;
	config.vsh_region = se_config.fake_region;
	config.extended_colors = se_config.extended_colors;
	config.enable_xmbctrl = se_config.enable_xmbctrl;

	config.use_nodrm = !se_config.no_nodrm_engine;
	config.vsh_plugins = !se_config.no_xmb_plugins;
	config.game_plugins = !se_config.no_game_plugins;
	config.pops_plugins = !se_config.no_pops_plugins;
}

void saveSettings() {
	sctrlSEGetConfig(&se_config);

	int oldspeed = se_config.vsh_cpu_speed;

	se_config.vsh_cpu_speed = config.vsh_cpu_speed;
	se_config.app_cpu_speed = config.game_cpu_speed;
	se_config.umd_mode = config.umd_driver;
	se_config.skip_logo = config.skip_sony_coldboot_logo;
	se_config.skip_game_boot_logo = config.skip_sony_gameboot_logo;
	se_config.hide_corrupt = config.hide_corrupt_icons;
	se_config.hide_mac_addr = config.hide_mac_addr;
	se_config.hide_dlcs = config.hide_dlcs;
	se_config.hide_pic0pic1 = config.hide_pic01;
	se_config.use_sony_psposk = config.use_sony_osk;
	se_config.startup_program = config.autorun_boot_eboot;
	se_config.force_high_memory = config.force_highmem;
	se_config.execute_boot_bin = config.exec_bootbin;
	se_config.fake_region = config.vsh_region;
	se_config.extended_colors = config.extended_colors;
	se_config.enable_xmbctrl = config.enable_xmbctrl;

	se_config.no_nodrm_engine = !config.use_nodrm;
	se_config.no_xmb_plugins = !config.vsh_plugins;
	se_config.no_game_plugins= !config.game_plugins;
	se_config.no_pops_plugins= !config.pops_plugins;

	sctrlSESetConfig(&se_config);

	// Apply VSH config change
	if (config.vsh_cpu_speed != oldspeed) {
		int cpu_list[] = { 0, 20, 75, 100, 133, 222, 266, 300, 333 };
		int bus_list[] = { 0, 10, 37,  50,  66, 111, 133, 150, 166 };

		sctrlHENSetSpeed(cpu_list[config.vsh_cpu_speed], bus_list[config.vsh_cpu_speed]);
	}
}
