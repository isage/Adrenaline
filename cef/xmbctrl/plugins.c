#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

#include "list.h"
#include "plugins.h"
#include "settings.h"
#include "main.h"

extern AdrenalineConfig se_config;
List plugins;

static char* sample_plugin_path = "ULUS01234, ms0:/SEPLUGINS/example.prx";

int cur_place = 0;

static void list_cleaner(void* item){
	Plugin* plugin = (Plugin*)item;
	my_free(plugin->path);
	if (plugin->name) my_free(plugin->name);
	if (plugin->surname) my_free(plugin->surname);
	my_free(plugin);
}

static void processCustomLine(char* line){
	Plugin* plugin = (Plugin*)my_malloc(sizeof(Plugin));
	memset(plugin, 0, sizeof(Plugin));
	plugin->path = line;
	plugin->place = cur_place;
	add_list(&plugins, plugin);
}

static void processPlugin(char* path, char* enabled) {
	int n = plugins.count;
	char* name = my_malloc(20);
	snprintf(name, 20, "plugin_%d", n);

	char* surname = my_malloc(20);
	snprintf(surname, 20, "plugins%d", n);

	int path_len = strlen(path) + 10;
	char* full_path = (char*)my_malloc(path_len);
	snprintf(full_path, path_len, "%s", path);

	Plugin* plugin = (Plugin*)my_malloc(sizeof(Plugin));
	plugin->name = name;
	plugin->surname = surname;
	plugin->path = full_path;
	plugin->place = cur_place;
	plugin->active = isRunlevelEnabled(enabled);

	add_list(&plugins, plugin);
}

void loadPlugins(){
	sctrlSEGetConfig(&se_config);
	clear_list(&plugins, &list_cleaner);

	if (!se_config.notusexmbplugins) {
		cur_place = PLACE_VSH;
		ProcessConfigFile("ms0:/seplugins/vsh.txt", &processPlugin, &processCustomLine);
	}

	if (!se_config.notusegameplugins) {
		cur_place = PLACE_GAME;
		ProcessConfigFile("ms0:/seplugins/game.txt", &processPlugin, &processCustomLine);
	}

	if (!se_config.notusepopsplugins) {
		cur_place = PLACE_POPS;
		ProcessConfigFile("ms0:/seplugins/pops.txt", &processPlugin, &processCustomLine);
	}

	if (plugins.count == 0){
		// Add example plugin
		Plugin* plugin = (Plugin*)my_malloc(sizeof(Plugin));
		plugin->name = (char*)my_malloc(20);
		plugin->surname = (char*)my_malloc(20);
		plugin->path = (char*)my_malloc(strlen(sample_plugin_path)+1);
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

	int fd[3] = {
		(!se_config.notusexmbplugins)? sceIoOpen("ms0:/seplugins/vsh.txt", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777) : -1,
		(!se_config.notusegameplugins)? sceIoOpen("ms0:/seplugins/game.txt", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777) : -1,
		(!se_config.notusepopsplugins)? sceIoOpen("ms0:/seplugins/pops.txt", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777) : -1,
	};

	for (int i = 0; i < plugins.count; i++) {
		Plugin *plugin = (Plugin *)(plugins.table[i]);
		if (fd[plugin->place] != -1) {
			if (plugin->active == PLUGIN_REMOVED)
				continue;

			sceIoWrite(fd[plugin->place], plugin->path, strlen(plugin->path));
			if (plugin->name != NULL) {
				char *sep = " ";
				char *enabled = (plugin->active) ? "1" : "0";
				sceIoWrite(fd[plugin->place], sep, 1);
				sceIoWrite(fd[plugin->place], enabled, strlen(enabled));
			}
			sceIoWrite(fd[plugin->place], "\n", 1);
		}
	}

	for (int i = 0; i < 3; i++) {
		if (fd[i] != -1) {
			sceIoClose(fd[i]);
		}
	}
}