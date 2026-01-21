/*
	Adrenaline
	Copyright (C) 2025, GrayJack
	Copyright (C) 2021, ARK CFW

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

#include <string.h>

#include <pspinit.h>
#include <pspsysmem.h>
#include <pspmodulemgr.h>
#include <pspiofilemgr.h>

#include <systemctrl_se.h>
#include <rebootexconfig.h>

#include <adrenaline_log.h>

#include "main.h"
#include "plugin.h"

#define LINE_BUFFER_SIZE 1024
#define LINE_TOKEN_DELIMITER ','

#define MAX_PLUGINS 64
#define MAX_PLUGIN_PATH 128

typedef struct{
	int count;
	char paths[MAX_PLUGINS][MAX_PLUGIN_PATH];
} Plugins;

Plugins* g_plugins = NULL;

static int (*g_plugin_handler)(const char* path, int modid) = NULL;

enum {
	RUNLEVEL_UNKNOWN,
	RUNLEVEL_VSH,
	RUNLEVEL_UMD,
	RUNLEVEL_POPS,
	RUNLEVEL_HOMEBREW,
};
static int cur_runlevel = RUNLEVEL_UNKNOWN;

int g_disable_plugins = 0;
static int g_is_plugins_loading = 0;

int isLoadingPlugins() {
	return g_is_plugins_loading;
}

static void addPlugin(const char* path) {
	for (int i=0; i<g_plugins->count; i++) {
		char* cmp1 = strchr(g_plugins->paths[i], ':');
		char* cmp2 = strchr(path, ':');
		if (!cmp1) {
			cmp1 = g_plugins->paths[i];
		}
		if (!cmp2) {
			cmp2 = (char*)path;
		}
		if (strcasecmp(cmp1, cmp2) == 0) {
			return; // plugin already added
		}
	}
	if (g_plugins->count < MAX_PLUGINS) {
		strcpy(g_plugins->paths[g_plugins->count++], path);
	}
}

static void removePlugin(const char* path) {
	for (int i=0; i<g_plugins->count; i++) {
		if (strcasecmp(g_plugins->paths[i], path) == 0) {
			if (--g_plugins->count > i) {
				strcpy(g_plugins->paths[i], g_plugins->paths[g_plugins->count]);
			}
			break;
		}
	}
}

// Load and Start Plugin Module
static void startPlugins() {
	for (int i=0; i<g_plugins->count; i++) {
		int res = 0;
		char* path = g_plugins->paths[i];
		// Load Module
		logmsg4("[DEBUG]: Processing plugin: %s\n", path);
		int uid = sceKernelLoadModule(path, 0, NULL);
		if (uid >= 0) {
			// Call handler
			if (g_plugin_handler) {
				res = g_plugin_handler(path, uid);
				// Unload Module on Error
				if (res < 0) {
					sceKernelUnloadModule(uid);
					continue;
				}
			}
			// Start Module
			res = sceKernelStartModule(uid, strlen(path) + 1, path, NULL, NULL);
			// Unload Module on Error
			if (res < 0) {
				sceKernelUnloadModule(uid);
			}
			logmsg3("[INFO]: Loaded plugin: %s\n", path);
		}
	}
}

static int isVshRunlevel() {
	if (!cur_runlevel) {
		// Fetch Apitype
		int apitype = sceKernelInitApitype();
		if (apitype >= PSP_INIT_APITYPE_VSH_KERNEL) {
			cur_runlevel = RUNLEVEL_VSH;
		}
	}
	return cur_runlevel == RUNLEVEL_VSH;
}

static int isPopsRunlevel() {
	if (!cur_runlevel) {
		// Fetch Apitype
		int apitype = sceKernelInitApitype();
		if (apitype == PSP_INIT_APITYPE_MS5 || apitype == PSP_INIT_APITYPE_EF5) {
			cur_runlevel = RUNLEVEL_POPS;
		}
	}
	return cur_runlevel == RUNLEVEL_POPS;
}

static int isUmdRunlevel() {
	if (!cur_runlevel) {
		// Fetch Apitype
		int apitype = sceKernelInitApitype();
		if (apitype == PSP_INIT_APITYPE_UMD || apitype == PSP_INIT_APITYPE_UMD2
			|| (apitype >= PSP_INIT_APITYPE_UMD_EMU_MS1 && apitype <= PSP_INIT_APITYPE_UMD_EMU_EF2)
			|| apitype == PSP_INIT_APITYPE_USBWLAN
			|| (apitype >= PSP_INIT_APITYPE_GAME_EBOOT && apitype <= PSP_INIT_APITYPE_EMU_BOOT_EF)) {

			cur_runlevel = RUNLEVEL_UMD;
		}
	}
	return cur_runlevel == RUNLEVEL_UMD;
}

static int isHomebrewRunlevel() {
	if (!cur_runlevel) {
		// Fetch Apitype
		int apitype = sceKernelInitApitype();
		if (apitype == PSP_INIT_APITYPE_MS2 || apitype == PSP_INIT_APITYPE_EF2) {
			cur_runlevel = RUNLEVEL_HOMEBREW;
		}
	}
	return cur_runlevel == RUNLEVEL_HOMEBREW;
}

static int isPath(char* runlevel) {
	return (
		strcasecmp(runlevel, sceKernelInitFileName())==0 ||
		strcasecmp(runlevel, sctrlSEGetUmdFile())==0
	);
}

static int isTitleId(char* runlevel) {
	if (g_rebootex_config.title_id[0] == 0) {
		return 0;
	}
	char gameid[10];
	memset(gameid, 0, sizeof(gameid));
	memcpy(gameid, g_rebootex_config.title_id, 9);
	lowerString(gameid, gameid, strlen(gameid)+1);
	return (strstr(runlevel, gameid) != NULL);
}

// Runlevel Check
static int matchingRunlevel(char * runlevel) {
	lowerString(runlevel, runlevel, strlen(runlevel)+1);

	char* cfw_type = strstr(runlevel, "cfw=");
	// Plugin is for specific CFW only
	if (cfw_type) {
		// Not for Adrenaline, treat as disabled
		if (strncasecmp(cfw_type+4, "adr", 3) != 0 || strncasecmp(cfw_type+4, "epi", 3) != 0 || strncasecmp(cfw_type+4, "tn", 2) != 0 ) {
			return 0;
		}
	}


	if (strcasecmp(runlevel, "all") == 0 || strcasecmp(runlevel, "always") == 0) {
		// always on
		return 1;
	}

	if (strchr(runlevel, '/')) {
		// it's a path
		return isPath(runlevel);
	}

	if (isVshRunlevel() && !g_cfw_config.no_xmb_plugins) {
		return (strstr(runlevel, "vsh") != NULL || strstr(runlevel, "xmb") != NULL);
	}

	if (isPopsRunlevel() && !g_cfw_config.no_pops_plugins) {
		// check if plugin loads on specific title
		if (isTitleId(runlevel)) {
			return 1;
		}
		// check keywords
		return (strstr(runlevel, "pops") != NULL || strstr(runlevel, "ps1") != NULL || strstr(runlevel, "psx") != NULL); // PS1 games only
	}

	if (isHomebrewRunlevel() && !g_cfw_config.no_game_plugins) {
		if (strstr(runlevel, "hbw") != NULL || strstr(runlevel, "homebrew") != NULL || strstr(runlevel, "app") != NULL || strstr(runlevel, "game") != NULL) {
			// homebrews only
			return 1;
		}
	}

	if (isUmdRunlevel() && !g_cfw_config.no_game_plugins) {
		// check if plugin loads on specific title
		if (isTitleId(runlevel)) {
			return 1;
		}
		// check keywords
		if (strstr(runlevel, "umd") != NULL || strstr(runlevel, "psp") != NULL || strstr(runlevel, "umdemu") != NULL || strstr(runlevel, "app") != NULL || strstr(runlevel, "game") != NULL) {
			// Retail games only
			return 1;
		}
	}

	// Unsupported Runlevel (we don't touch those to keep stability up)
	return 0;
}

// Boolean String Parser
static int booleanOn(char * text) {
	// Different Variations of "true"
	if (strcasecmp(text, "true") == 0 || strcasecmp(text, "on") == 0 || strcmp(text, "1") == 0 || strcasecmp(text, "enabled") == 0) {
		return 1;
	}

	// Default to False
	return 0;
}

// Whitespace Detection
static int is_space(int c) {
	// Whitespaces
	if (c == ' ' || c == '\t' || c == '\r' || c == '\v' || c == '\f' || c == '\n') {
		return 1;
	}

	// Normal Character
	return 0;
}

// Trim Leading and Trailing Whitespaces
static char * strtrim(char * text) {
	// Invalid Argument
	if (text == NULL) {
		return NULL;
	}

	// Remove Leading Whitespaces
	while (is_space(text[0])) {
		text++;
	}

	// Scan Position
	int pos = strlen(text)-1;
	if (pos < 0) {
		return text;
	}

	// Find Trailing Whitespaces
	while (is_space(text[pos])) pos--;

	// Terminate String
	text[pos+1] = (char)0;

	// Return Trimmed String
	return text;
}

static int readLine(char* source, char *str) {
	u8 ch = 0;
	int n = 0;
	int i = 0;
	while (1) {
		if ( (ch = source[i]) == 0) {
			*str = 0;
			return n;
		}
		n++; i++;
		if (ch < 0x20) {
			*str = 0;
			return n;
		} else {
			*str++ = ch;
		}
	}
}

// Parse and Process Line
static void processLine(const char* parent, char* line, void (*enabler)(const char*), void (*disabler)(const char*)) {
	// Skip Comment Lines
	if (!enabler || line == NULL || strncmp(line, "//", 2) == 0 || line[0] == ';' || line[0] == '#') {
		return;
	}

	// String Token
	char * runlevel = line;
	char * path = NULL;
	char * enabled = NULL;

	// Original String Length
	unsigned int length = strlen(line);

	// Fetch String Token
	unsigned int i = 0;
	for (; i < length; i++) {
		// Got all required Token
		if (enabled != NULL) {
			// Handle Trailing Comments as Terminators
			if (strncmp(line + i, "//", 2) == 0 || line[i] == ';' || line[i] == '#') {
				// Terminate String
				line[i] = 0;

				// Stop Token Scan
				break;
			}
		}

		// Found Delimiter
		if (line[i] == LINE_TOKEN_DELIMITER) {
			// Terminate String
			line[i] = 0;

			// Path Start
			if (path == NULL) path = line + i + 1;

			// Enabled Start
			else if (enabled == NULL) enabled = line + i + 1;

			// Got all Data
			else break;
		}
	}

	// Insufficient Plugin Information
	if (enabled == NULL) {
		return;
	}

	// Trim Whitespaces
	runlevel = strtrim(runlevel);
	path = strtrim(path);
	enabled = strtrim(enabled);

	// Matching Plugin Runlevel
	if (matchingRunlevel(runlevel)) {
		char full_path[MAX_PLUGIN_PATH];
		if (parent && strchr(path, ':') == NULL) { // relative path
			strcpy(full_path, parent);
			strcat(full_path, path);

		} else { // already full path
			strcpy(full_path, path);
		}

		// Enabled Plugin
		if (booleanOn(enabled)) {
			// Start Plugin
			enabler(full_path);
		} else {
			if (disabler) {
				disabler(full_path);
			}
		}
	}
}

// Load Plugins
static int ProcessConfigFile(const char* parent, const char* path, void (*enabler)(const char*), void (*disabler)(const char*)) {
	int fd = sceIoOpen(path, PSP_O_RDONLY, 0777);

	// Opened Plugin Config
	if (fd >= 0) {
		// allocate buffer in user ram and read entire file
		int fsize = sceIoLseek(fd, 0, PSP_SEEK_END);
		sceIoLseek(fd, 0, PSP_SEEK_SET);

		SceUID memid = sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_KERNEL, "", PSP_SMEM_Low, fsize+1, NULL);
		u8* buf = sceKernelGetBlockHeadAddr(memid);
		if (buf == NULL) {
			sceIoClose(fd);
			return -1;
		}

		sceIoRead(fd, buf, fsize);
		sceIoClose(fd);
		buf[fsize] = 0;

		// Allocate Line Buffer
		char * line = (char *)oe_malloc(LINE_BUFFER_SIZE);

		// Buffer Allocation Success
		if (line != NULL) {
			// Read Lines
			int nread = 0;
			while ((nread=readLine((char*)buf, line))>0) {
				buf += nread;
				if (line[0] == 0) {
					// empty line
					continue;
				}
				// Process Line
				processLine(parent, strtrim(line), enabler, disabler);
			}

			// Free Buffer
			oe_free(line);
		}

		sceKernelFreePartitionMemory(memid);

		// Close Plugin Config
		return 0;
	}
	return -1;
}

void loadPlugins() {
	if (g_disable_plugins) {
		return;
	}
	g_is_plugins_loading = 1;
	// allocate resources
	g_plugins = oe_malloc(sizeof(Plugins));
	g_plugins->count = 0; // initialize plugins table

	SceIoStat stat;
	int epiplugins_exists = sceIoGetstat("ms0:/seplugins/EPIplugins.txt", &stat) >= 0;

	if (epiplugins_exists) {
		ProcessConfigFile("ms0:/seplugins/", "ms0:/seplugins/EPIplugins.txt", addPlugin, removePlugin);
	} else {
		ProcessConfigFile("ms0:/seplugins/", "ms0:/seplugins/plugins.txt", addPlugin, removePlugin);
	}


	// start all loaded plugins
	startPlugins();
	// free resources
	oe_free(g_plugins);
	g_plugins = NULL;
	g_is_plugins_loading = 0;
}
