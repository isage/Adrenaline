/*
	Adrenaline
	Copyright (C) 2016-2018, TheFloW

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

#include <stdio.h>
#include <string.h>

#include <pspiofilemgr.h>

#include <systemctrl_se.h>

#include "main.h"
#include "menu.h"
#include "utils.h"
#include "options.h"
#include "plugins.h"

void *user_malloc(SceSize size);
void user_free(void *ptr);

typedef struct {
	char name[138];
	char path[128];
	char runlevel[64];
	int active;
} Plugin;

Entry g_plugins_tool_entries[MAX_PLUGINS + 1];

static Plugin g_plugins[MAX_PLUGINS] = {0};
static int g_plugin_count = 0;

#define LINE_BUFFER_SIZE 1024
#define LINE_TOKEN_DELIMITER ','

int findEmpySlot() {
	for (int i = 0; i < MAX_PLUGINS; i++) {
		if (g_plugins[i].name[0] == 0) {
			return i;
		}
	}
	return -1;
}

int isRunlevelEnabled(char* line) {
	return (strcasecmp(line, "on") == 0 || strcasecmp(line, "1") == 0 || strcasecmp(line, "enabled") == 0 || strcasecmp(line, "true") == 0);
}

static int is_space(int c) {
	if (c == ' ' || c == '\t' || c == '\r' || c == '\v' || c == '\f' || c == '\n') {
		return 1;
	}

	return 0;
}

// Trim leading and trailing whitespaces
static char * strtrim(char * text) {
	// Invalid argument
	if (text == NULL) {
		return NULL;
	}

	// Remove leading whitespaces
	while (is_space(text[0])) {
		text++;
	}

	// Scan position
	int pos = strlen(text)-1;
	if (pos<0) {
		return text;
	}

	// Find trailing whitespaces
	while (is_space(text[pos])) {
		pos--;
	}

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
		if ((ch = source[i]) == 0) {
			*str = 0;
			return n;
		}

		if(ch < 0x20) {
			if(n != 0){
				*str = 0;
				return n;
			}
		} else {
			*str++ = ch;
			n++;
		}
		i++;
	}
}

// Parse and Process Line
static int processLine(char * line, int (process_line)(char*, char*, char*)) {
	// Skip lines comments
	if (line == NULL || strncmp(line, "//", 2) == 0 || line[0] == ';' || line[0] == '#') {
		return 0;
	}

	// String Token
	char * runlevel = line;
	char * path = NULL;
	char * enabled = NULL;

	// Original string length
	unsigned int length = strlen(line);

	// Fetch string token
	for (u32 i = 0; i < length; i++) {
		// Got all required Token
		if (enabled != NULL) {
			// Handle trailing comments as terminators
			if (strncmp(line + i, "//", 2) == 0 || line[i] == ';' || line[i] == '#') {
				// Terminate dtring
				line[i] = 0;

				// Stop token scan
				break;
			}
		}

		if (line[i] == LINE_TOKEN_DELIMITER) {
			// Terminate string
			line[i] = 0;

			// Path start
			if(path == NULL) {
				path = line + i + 1;
			} else if(enabled == NULL) {
				// Enabled start
				enabled = line + i + 1;
			} else {
				// Got all data
				break;
			}
		}
	}

	// Insufficient plugin information
	if (enabled == NULL) {
		return 0;
	}

	// Trim whitespaces
	runlevel = strtrim(runlevel);
	path = strtrim(path);
	enabled = strtrim(enabled);

	return process_line(runlevel, path, enabled);
}

static void ProcessConfigFile(char* path, int (process_line)(char*, char*, char*), void (*process_custom)(char*)) {
	int fd = sceIoOpen(path, PSP_O_RDONLY, 0777);

	// Opened Plugin Config
	if (fd >= 0) {
		// allocate buffer and read entire file
		int fsize = sceIoLseek(fd, 0, PSP_SEEK_END);
		sceIoLseek(fd, 0, PSP_SEEK_SET);

		u8* buf = user_malloc(fsize+1);
		if (buf == NULL){
			sceIoClose(fd);
			return;
		}

		sceIoRead(fd, buf, fsize);
		sceIoClose(fd);
		buf[fsize] = 0;

		// Allocate Line Buffer
		char* line = user_malloc(LINE_BUFFER_SIZE);
		// Buffer Allocation Success
		if (line != NULL) {
			// Read Lines
			int nread = 0;
			int total_read = 0;

			while ((nread=readLine((char*)buf+total_read, line)) > 0) {
				total_read += nread;
				if (line[0] == 0) continue; // empty line
				char* dupline = user_malloc(strlen(line)+1);
				strcpy(dupline, line);
				// Process Line
				if (processLine(strtrim(line), process_line)) {
					user_free(dupline);
				} else {
					process_custom(dupline);
				}
			}
			user_free(line);
		}
		// Close Plugin Config
		sceIoClose(fd);
	}
}

// Ignore ill formed line
static void processCustomLine(char* line) {
	return;
}

static void processPlugin(char* runlevel, char* path, char* enabled) {
	int idx = findEmpySlot();

	int path_len = strlen(path) + 10;

	char *p = strrchr(path, '/');

	if (p != NULL) {
		snprintf(g_plugins[idx].name, 138, "%s [%s]", p+1, runlevel);
	} else {
		snprintf(g_plugins[idx].name, 138, "%s [%s]", path, runlevel);
	}

	snprintf(g_plugins[idx].path, path_len, "%s", path);

	int run_level_len = strlen(runlevel) + 10;
	snprintf(g_plugins[idx].runlevel, run_level_len, "%s", runlevel);

	g_plugins[idx].active = isRunlevelEnabled(enabled);

	g_plugin_count += 1;
}

void savePlugins() {
	if (g_plugin_count == 0) {
		return;
	}

	int fd = sceIoOpen("ms0:/seplugins/EPIplugins.txt", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

	if (fd < 0) {
		return;
	}

	for (int i = 0; i < g_plugin_count; i++) {
		Plugin* plugin = &g_plugins[i];

		if (plugin->name[0] != 0) {
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

void readPlugins() {
	memset(g_plugins, 0, MAX_PLUGINS*sizeof(Plugin));

	ProcessConfigFile("ms0:/seplugins/EPIplugins.txt", &processPlugin, &processCustomLine);
}

void SetPlugins(int sel) {
	savePlugins();
}

int Plugins() {
	memset(g_plugins_tool_entries, 0, sizeof(g_plugins_tool_entries));

	readPlugins();

	for (int i = 0; i < g_plugin_count; i++) {
		Plugin* plugin = &g_plugins[i];
	    printf(plugin->name);
		g_plugins_tool_entries[i].name = plugin->name;
		g_plugins_tool_entries[i].function = (void *)SetPlugins;
		g_plugins_tool_entries[i].options = g_disenabled;
		g_plugins_tool_entries[i].size_options = sizeof(g_disenabled);
		g_plugins_tool_entries[i].value = &plugin->active;
	}
	return g_plugin_count;
}

#define CHUNK_SIZE 512
void ImportClassicPlugins(int sel) {
	char* buf = user_malloc(CHUNK_SIZE);
	int read = 0;

	SceUID game_fd = sceIoOpen("ms0:/seplugins/game.txt", PSP_O_RDONLY, 0777);
	SceUID vsh_fd = sceIoOpen("ms0:/seplugins/vsh.txt", PSP_O_RDONLY, 0777);
	SceUID pops_fd = sceIoOpen("ms0:/seplugins/pops.txt", PSP_O_RDONLY, 0777);
	SceUID plugins_fd = sceIoOpen("ms0:/seplugins/EPIplugins.txt", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

	// XMB/VSH
	memset(buf, 0, CHUNK_SIZE);
	while ( (read = sceIoRead(vsh_fd, buf, CHUNK_SIZE)) > 0 ) {
		for (int i = 0; i < read; i++) {
			if (i == 0 || buf[i-1] == '\n' || buf[i-1] == '\0') {
				sceIoWrite(plugins_fd, "vsh, ", 5);
			}
			if (buf[i] == ' ' && i != 0) {
				sceIoWrite(plugins_fd, ",", 1);
			}
			sceIoWrite(plugins_fd, &buf[i], 1);
		}
	}
	// Newline at the end
	sceIoWrite(plugins_fd, "\n", 1);
	sceIoClose(vsh_fd);

	// game
	memset(buf, 0, CHUNK_SIZE);
	while ( (read = sceIoRead(game_fd, buf, CHUNK_SIZE)) > 0 ) {
		for (int i = 0; i < read; i++) {
			if (i == 0 || buf[i-1] == '\n' || buf[i-1] == '\0') {
				sceIoWrite(plugins_fd, "game, ", 6);
			}
			if (buf[i] == ' ' && i != 0) {
				sceIoWrite(plugins_fd, ",", 1);
			}
			sceIoWrite(plugins_fd, &buf[i], 1);
		}
	}
	// Newline at the end
	sceIoWrite(plugins_fd, "\n", 1);
	sceIoClose(game_fd);

	// POPS
	memset(buf, 0, CHUNK_SIZE);
	while ( (read = sceIoRead(pops_fd, buf, CHUNK_SIZE)) > 0 ) {
		for (int i = 0; i < read; i++) {
			if (i == 0 || buf[i-1] == '\n' || buf[i-1] == '\0') {
				sceIoWrite(plugins_fd, "pops, ", 6);
			}
			if (buf[i] == ' ' && i != 0) {
				sceIoWrite(plugins_fd, ",", 1);
			}
			sceIoWrite(plugins_fd, &buf[i], 1);
		}
	}
	// Newline at the end
	sceIoWrite(plugins_fd, "\n", 1);
	sceIoClose(pops_fd);

	sceIoClose(plugins_fd);
	user_free(buf);
	readPlugins();
}