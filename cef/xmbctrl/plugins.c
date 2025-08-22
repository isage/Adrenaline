#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <sysclib_user.h>

#include "list.h"
#include "plugins.h"
#include "settings.h"
#include "main.h"

extern AdrenalineConfig se_config;
List plugins;

static char* sample_plugin_path = "ULUS01234, ms0:/SEPLUGINS/example.prx";

int cur_place = 0;

static void list_cleaner(void* item) {
	Plugin* plugin = (Plugin*)item;
	user_free(plugin->path);
	if (plugin->name) user_free(plugin->name);
	if (plugin->surname) user_free(plugin->surname);
	user_free(plugin);
}

static void processCustomLine(char* line) {
	Plugin* plugin = (Plugin*)user_malloc(sizeof(Plugin));
	memset(plugin, 0, sizeof(Plugin));
	plugin->path = line;
	plugin->place = cur_place;
	add_list(&plugins, plugin);
}

static void processPlugin(char* runlevel, char* path, char* enabled) {
	int n = plugins.count;
	char* name = user_malloc(20);
	snprintf(name, 20, "plugin_%d", n);

	char* surname = user_malloc(20);
	snprintf(surname, 20, "plugins%d", n);

	int path_len = strlen(path) + 10;
	char* full_path = (char*)user_malloc(path_len);
	snprintf(full_path, path_len, "%s", path);

	int run_level_len = strlen(runlevel) + 10;
	char* run_level = (char*)user_malloc(run_level_len);
	snprintf(run_level, run_level_len, "%s", runlevel);

	Plugin* plugin = (Plugin*)user_malloc(sizeof(Plugin));
	plugin->name = name;
	plugin->surname = surname;
	plugin->path = full_path;
	plugin->runlevel = run_level;
	plugin->place = cur_place;
	plugin->active = isRunlevelEnabled(enabled);

	add_list(&plugins, plugin);
}

void loadPlugins(){
	clear_list(&plugins, &list_cleaner);

	ProcessConfigFile("ms0:/seplugins/EPIplugins.txt", &processPlugin, &processCustomLine);

	if (plugins.count == 0){
		// Add example plugin
		Plugin* plugin = (Plugin*)user_malloc(sizeof(Plugin));
		plugin->name = (char*)user_malloc(20);
		plugin->surname = (char*)user_malloc(20);
		plugin->path = (char*)user_malloc(strlen(sample_plugin_path)+1);
		plugin->active = 1;
		plugin->place = 0;
		strcpy(plugin->name, "plugin_0");
		strcpy(plugin->surname, "plugins0");
		strcpy(plugin->path, sample_plugin_path);
		add_list(&plugins, plugin);
	}
}

void savePlugins() {
	if (plugins.count == 0)
		return;

	if (plugins.count == 1) {
		Plugin *plugin = (Plugin *)(plugins.table[0]);
		if (strcmp(plugin->path, sample_plugin_path) == 0) {
			return;
		}
	}

	sctrlSEGetConfig(&se_config);

	int fd = sceIoOpen("ms0:/seplugins/EPIplugins.txt", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

	if (fd < 0) {
		return;
	}

	for (int i = 0; i < plugins.count; i++) {
		Plugin *plugin = (Plugin *)(plugins.table[i]);

		if (plugin->active == PLUGIN_REMOVED) {
			continue;
		}

		if (plugin->name != NULL) {
			char buf[256] = {0};
			char *enabled = (plugin->active) ? "on" : "off";
			sprintf(buf, "%s, %s, %s\n", plugin->runlevel, plugin->path, enabled);
			sceIoWrite(fd, buf, strlen(buf));
		}
	}

	if (fd >= 0) {
		sceIoClose(fd);
	}
}