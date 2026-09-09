#ifndef __JANUS_H__
#define __JANUS_H__

#include <pspctrl.h>

#define HOTKEY (PSP_CTRL_START | PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER | PSP_CTRL_UP)
#define MENU_KEYS (PSP_CTRL_SELECT | PSP_CTRL_START | PSP_CTRL_UP | PSP_CTRL_DOWN | \
                   PSP_CTRL_LEFT | PSP_CTRL_RIGHT | PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER | \
                   PSP_CTRL_CIRCLE | PSP_CTRL_CROSS | PSP_CTRL_TRIANGLE | PSP_CTRL_SQUARE | \
                   PSP_CTRL_HOME)

#define MAX_ENTRIES 128
#define MAX_PATH_LEN 256
#define MAX_NAME_LEN 256
#define MAX_KEY_LEN 256
#define VISIBLE_ROWS 12

#define THREAD_LIST_MAX 256

enum ButtonAssign {
	CONFIRM_CIRCLE = 0,
	CONFIRM_CROSS = 1,
};

typedef struct DiscEntry {
    char name[MAX_NAME_LEN];
    char path[MAX_PATH_LEN];
} DiscEntry;

#define OPAQUE(color) (((unsigned int)(color) & 0x00FFFFFFu) | 0xFF000000u)
// ABGR
typedef struct {
	char name[64];
	int menu_header;
	int menu_bg;
	int menu_selection;
	int white;
	int black;
	int red;
	int green;
	int blue;
	int yellow;
	int magenta;
	int cyan;
	int orange;
	int purple;
	int bright_white;
} JanusColorTheme;

#define JANUS_MAGIC (0xFBC0727B) // Janus1.0
typedef struct {
	int magic;
	u8 theme;
} JanusConfig;

#endif // __XMBCTRL_H__