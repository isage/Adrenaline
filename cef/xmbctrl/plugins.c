#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#include <systemctrl.h>
#include <systemctrl_se.h>

#include "list.h"
#include "xmbctrl.h"
#include "plugins.h"

List g_plugins;

static char* sample_plugin_path = "ms0:/SEPLUGINS/example.prx";

static int g_cur_place = 0;

static int isRunlevelEnabled(char* line) {
	return (paf_strcasecmp(line, "on") == 0 || paf_strcasecmp(line, "1") == 0 || paf_strcasecmp(line, "enabled") == 0 || paf_strcasecmp(line, "true") == 0);
}

static int isspace(int c) {
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
	while (isspace(text[0])) {
		text++;
	}

	// Scan position
	int pos = paf_strlen(text)-1;
	if (pos<0) {
		return text;
	}

	// Find trailing whitespaces
	while (isspace(text[pos])) {
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
	if (line == NULL || paf_strncmp(line, "//", 2) == 0 || line[0] == ';' || line[0] == '#') {
		return 0;
	}

	// String Token
	char * runlevel = line;
	char * path = NULL;
	char * enabled = NULL;

	// Original string length
	unsigned int length = paf_strlen(line);

	// Fetch string token
	for (u32 i = 0; i < length; i++) {
		// Got all required Token
		if (enabled != NULL) {
			// Handle trailing comments as terminators
			if (paf_strncmp(line + i, "//", 2) == 0 || line[i] == ';' || line[i] == '#') {
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

static void ProcessPluginFile(char* path, int (process_line)(char*, char*, char*), void (*process_custom)(char*)) {
	int fd = sceIoOpen(path, PSP_O_RDONLY, 0777);

	// Opened Plugin Config
	if (fd >= 0) {
		// allocate buffer and read entire file
		int fsize = sceIoLseek(fd, 0, PSP_SEEK_END);
		sceIoLseek(fd, 0, PSP_SEEK_SET);

		u8* buf = paf_malloc(fsize+1);
		if (buf == NULL){
			sceIoClose(fd);
			return;
		}

		sceIoRead(fd, buf, fsize);
		sceIoClose(fd);
		buf[fsize] = 0;

		// Allocate Line Buffer
		char* line = paf_malloc(LINE_BUFFER_SIZE);
		// Buffer Allocation Success
		if (line != NULL) {
			// Read Lines
			int nread = 0;
			int total_read = 0;

			while ((nread=readLine((char*)buf+total_read, line)) > 0) {
				total_read += nread;
				if (line[0] == 0) continue; // empty line
				char* dupline = paf_malloc(paf_strlen(line)+1);
				paf_strcpy(dupline, line);
				// Process Line
				if (processLine(strtrim(line), process_line)) {
					paf_free(dupline);
				} else {
					process_custom(dupline);
				}
			}
			paf_free(line);
		}
		// Close Plugin Config
		sceIoClose(fd);
	}
}

static void list_cleaner(void* item) {
	if (item == NULL) {
		return;
	}

	Plugin* plugin = (Plugin*)item;
	if (plugin->name) paf_free(plugin->name);
	if (plugin->surname) paf_free(plugin->surname);
	if (plugin->path) paf_free(plugin->path);
	if (plugin->runlevel) paf_free(plugin->runlevel);
	paf_free(plugin);
}

static void processCustomLine(char* line) {
	Plugin* plugin = (Plugin*)paf_malloc(sizeof(Plugin));
	paf_memset(plugin, 0, sizeof(Plugin));
	plugin->path = line;
	plugin->place = g_cur_place;
	add_list(&g_plugins, plugin);
}

static void processPlugin(char* runlevel, char* path, char* enabled) {
	int n = g_plugins.count;
	char* name = paf_malloc(20);
	paf_snprintf(name, 20, "plugin_%d", n);

	char* surname = paf_malloc(20);
	paf_snprintf(surname, 20, "plugins%d", n);

	int path_len = paf_strlen(path) + 10;
	char* full_path = (char*)paf_malloc(path_len);
	paf_snprintf(full_path, path_len, "%s", path);

	int run_level_len = paf_strlen(runlevel) + 10;
	char* run_level = (char*)paf_malloc(run_level_len);
	paf_snprintf(run_level, run_level_len, "%s", runlevel);

	Plugin* plugin = (Plugin*)paf_malloc(sizeof(Plugin));
	plugin->name = name;
	plugin->surname = surname;
	plugin->path = full_path;
	plugin->runlevel = run_level;
	plugin->place = g_cur_place;
	plugin->active = isRunlevelEnabled(enabled);

	add_list(&g_plugins, plugin);
}

void loadPlugins(){
	clear_list(&g_plugins, &list_cleaner);

	SceIoStat stat;
	int epiplugins_exists = sceIoGetstat("ms0:/seplugins/EPIplugins.txt", &stat) >= 0;

	if (epiplugins_exists) {
		ProcessPluginFile("ms0:/seplugins/EPIplugins.txt", &processPlugin, &processCustomLine);
	} else {
		ProcessPluginFile("ms0:/seplugins/plugins.txt", &processPlugin, &processCustomLine);
	}


	if (g_plugins.count == 0){
		// Add example plugin
		Plugin* plugin = (Plugin*)paf_malloc(sizeof(Plugin));
		plugin->name = (char*)paf_malloc(20);
		plugin->surname = (char*)paf_malloc(20);
		plugin->runlevel = (char*)paf_malloc(20);
		plugin->path = (char*)paf_malloc(paf_strlen(sample_plugin_path)+1);
		plugin->active = 1;
		plugin->place = 0;
		paf_strcpy(plugin->name, "plugin_0");
		paf_strcpy(plugin->surname, "plugins0");
		paf_strcpy(plugin->runlevel, "ULUS01234");
		paf_strcpy(plugin->path, sample_plugin_path);
		add_list(&g_plugins, plugin);
	}
}

void savePlugins() {
	if (g_plugins.count == 0) {
		return;
	}

	if (g_plugins.count == 1) {
		Plugin *plugin = (Plugin *)(g_plugins.table[0]);
		if (paf_strcmp(plugin->path, sample_plugin_path) == 0) {
			return;
		}
	}

	sctrlSEGetConfig(&g_cfw_config);

	int fd = sceIoOpen("ms0:/seplugins/EPIplugins.txt", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

	if (fd < 0) {
		return;
	}

	for (int i = 0; i < g_plugins.count; i++) {
		Plugin *plugin = (Plugin *)(g_plugins.table[i]);

		if (plugin->active == PLUGIN_REMOVED) {
			continue;
		}

		if (plugin->name != NULL) {
			char buf[256] = {0};
			char *enabled = (plugin->active) ? "on" : "off";
			paf_sprintf(buf, "%s, %s, %s\n", plugin->runlevel, plugin->path, enabled);
			sceIoWrite(fd, buf, paf_strlen(buf));
		}
	}

	if (fd >= 0) {
		sceIoClose(fd);
	}
}