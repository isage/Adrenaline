#include <pspsdk.h>
#include <pspkernel.h>
#include <psputility_sysparam.h>

#include <kubridge.h>
#include <systemctrl.h>
#include <systemctrl_se.h>

#include "xmbctrl.h"
#include "list.h"
#include "plugins.h"

static int g_old_vsh_speed;

void loadSettings() {
	paf_memset(&g_cfw_config, 0, sizeof(g_cfw_config));

	sctrlSEGetConfig(&g_cfw_config);
	g_old_vsh_speed = g_cfw_config.vsh_cpu_speed;
}

void saveSettings() {
	sctrlSESetConfig(&g_cfw_config);

	// Apply VSH config change
	if (g_cfw_config.vsh_cpu_speed != g_old_vsh_speed) {
		static int cpu_list[] = { 0, 20, 75, 100, 133, 222, 266, 300, 333 };
		static int bus_list[] = { 0, 10, 37,  50,  66, 111, 133, 150, 166 };

		sctrlHENSetSpeed(cpu_list[g_cfw_config.vsh_cpu_speed], bus_list[g_cfw_config.vsh_cpu_speed]);
	}
}
