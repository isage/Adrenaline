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

extern AdrenalineConfig se_config;

static int old_vsh_speed;

void loadSettings() {
	memset(&se_config, 0, sizeof(se_config));

	sctrlSEGetConfig(&se_config);
	old_vsh_speed = se_config.vsh_cpu_speed;
}

void saveSettings() {
	sctrlSESetConfig(&se_config);

	// Apply VSH config change
	if (se_config.vsh_cpu_speed != old_vsh_speed) {
		static int cpu_list[] = { 0, 20, 75, 100, 133, 222, 266, 300, 333 };
		static int bus_list[] = { 0, 10, 37,  50,  66, 111, 133, 150, 166 };

		sctrlHENSetSpeed(cpu_list[se_config.vsh_cpu_speed], bus_list[se_config.vsh_cpu_speed]);
	}
}
