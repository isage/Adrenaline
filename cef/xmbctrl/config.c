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

	config.vsh_cpu_speed = se_config.vshcpuspeed;
	config.game_cpu_speed = se_config.umdisocpuspeed;
	config.umd_driver = se_config.umdmode;
	config.skip_sony_coldboot_logo = se_config.skiplogo;
	config.skip_sony_gameboot_logo = se_config.skipgameboot;
	config.hide_corrupt_icons = se_config.hidecorrupt;
	config.hide_mac_addr = se_config.hidemacaddr;
	config.hide_dlcs = se_config.hidedlcs;
	config.hide_pic01 = se_config.hidepic0pic1;
	config.use_sony_osk = se_config.usesonypsposk;
	config.autorun_boot_eboot = se_config.startupprog;
	config.force_highmem = se_config.forcehighmemory;
	config.exec_bootbin = se_config.executebootbin;
	config.vsh_region = se_config.fakeregion;
	config.extended_colors = se_config.useextendedcolors;
	config.enablexmbctrl = se_config.enablexmbctrl;

	config.use_nodrm = !se_config.notusenodrmengine;
	config.vsh_plugins = !se_config.notusexmbplugins;
	config.game_plugins = !se_config.notusegameplugins;
	config.pops_plugins = !se_config.notusepopsplugins;
}

void saveSettings() {
	sctrlSEGetConfig(&se_config);

	int oldspeed = se_config.vshcpuspeed;

	se_config.vshcpuspeed = config.vsh_cpu_speed;
	se_config.umdisocpuspeed = config.game_cpu_speed;
	se_config.umdmode = config.umd_driver;
	se_config.skiplogo = config.skip_sony_coldboot_logo;
	se_config.skipgameboot = config.skip_sony_gameboot_logo;
	se_config.hidecorrupt = config.hide_corrupt_icons;
	se_config.hidemacaddr = config.hide_mac_addr;
	se_config.hidedlcs = config.hide_dlcs;
	se_config.hidepic0pic1 = config.hide_pic01;
	se_config.usesonypsposk = config.use_sony_osk;
	se_config.startupprog = config.autorun_boot_eboot;
	se_config.forcehighmemory = config.force_highmem;
	se_config.executebootbin = config.exec_bootbin;
	se_config.fakeregion = config.vsh_region;
	se_config.useextendedcolors = config.extended_colors;
	se_config.enablexmbctrl = config.enablexmbctrl;

	se_config.notusenodrmengine = !config.use_nodrm;
	se_config.notusexmbplugins = !config.vsh_plugins;
	se_config.notusegameplugins= !config.game_plugins;
	se_config.notusepopsplugins= !config.pops_plugins;

	sctrlSESetConfig(&se_config);

	// Apply VSH config change
	if (config.vsh_cpu_speed != oldspeed) {
		int cpu_list[] = { 0, 20, 75, 100, 133, 222, 266, 300, 333 };
		int bus_list[] = { 0, 10, 37,  50,  66, 111, 133, 150, 166 };

		sctrlHENSetSpeed(cpu_list[config.vsh_cpu_speed], bus_list[config.vsh_cpu_speed]);
	}
}
