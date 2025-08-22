#include <pspsdk.h>
#include <pspkernel.h>
#include <psputility_sysparam.h>
#include <systemctrl.h>
#include <vshctrl.h>
#include <kubridge.h>
#include <sysclib_user.h>

#include "settings.h"
#include "list.h"

int isRunlevelEnabled(char* line) {
	return (strcasecmp(line, "on") == 0 || strcasecmp(line, "1") == 0 || strcasecmp(line, "enabled") == 0 || strcasecmp(line, "true") == 0);
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
	int pos = strlen(text)-1;
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

int readLine(char* source, char *str) {
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

void ProcessConfigFile(char* path, int (process_line)(char*, char*, char*), void (*process_custom)(char*)) {
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
