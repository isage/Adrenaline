#ifndef PLUGINS_H
#define PLUGINS_H

#include <stddef.h>

#include <pspsdk.h>
#include <pspkernel.h>
#include <psputility_sysparam.h>

#include <systemctrl.h>
#include <kubridge.h>

#define LINE_BUFFER_SIZE 1024
#define LINE_TOKEN_DELIMITER ','

#define STAR "★"
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

extern List g_plugins;

void loadPlugins(void);

void savePlugins(void);

#endif
