#ifndef PLUGINS_H
#define PLUGINS_H

#include <pspsdk.h>
#include <pspkernel.h>
#include <psputility_sysparam.h>
#include <systemctrl.h>
#include <kubridge.h>
#include <stddef.h>

#define ADVANCED "▲"

enum{
	PLACE_VSH,
	PLACE_GAME,
	PLACE_POPS,
};

enum{
	PLUGIN_OFF,
	PLUGIN_ON,
	PLUGIN_REMOVED,
};

typedef struct{
	char* name;
	char* path;
	char* surname;
	char* runlevel;
	int active;
	int place;
} Plugin;

void loadPlugins(void);

void savePlugins(void);

#endif
