/*
	Janus
	Copyright (C) 2026 shoui520
	Copyright (C) 2016-2018, GrayJack

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

#include <pspkernel.h>
#include <pspctrl.h>
#include <pspdisplay.h>
#include <pspiofilemgr.h>
#include <pspthreadman.h>
#include <pspumd.h>
#include <pspreg.h>

#include <psperror.h>
#include <isoctrl.h>
#include <systemctrl.h>
#include <systemctrl_se.h>

#define _ADRENALINE_LOG_IMPL_
#include <adrenaline_log.h>

#include "janus.h"
#include "blit.h"

PSP_MODULE_INFO("Janus", 0x1000, 1, 0);

static JanusConfig g_config;

static volatile int g_stop = 0;
static DiscEntry g_all_entries[MAX_ENTRIES];
static int g_entries[MAX_ENTRIES];
static int g_all_count = 0;
static int g_entry_count = 0;
static int g_selected = 0;
static int g_current_index = 0;

static char g_current_path[MAX_PATH_LEN] = { 0 };
static char g_current_dir[MAX_PATH_LEN] = { 0 };
static char g_current_name[MAX_NAME_LEN] = { 0 };
static char g_og_path[MAX_PATH_LEN] = { 0 };
static char g_og_dir[MAX_PATH_LEN] = { 0 };
static char g_og_name[MAX_NAME_LEN] = { 0 };
static char g_status[96] = { 0 };

static SceUID g_base_threads[THREAD_LIST_MAX] = { 0 };
static int g_base_thread_count = 0;
static SceUID g_paused_threads[THREAD_LIST_MAX] = { 0 };
static int g_paused_thread_count = 0;

static int g_button_assign = CONFIRM_CIRCLE;
// 0 - Closed, 1 - Opened
static int g_menu_state = 0;

static const char * g_ef_path = "ef0:ISO/";

#define NUM_THEMES 2;
static JanusColorTheme g_themes[] = {
	{
		.name = "Epinephrine",
		.menu_header = 0x007F7F7F,
		.menu_bg = 0xB0000000,
		.menu_selection = 0x00FF1F7F,
		.white = 0x00fafcfc,
		.black = 0x00FFFFFF,
		.red = 0x000000FF,
		.green = 0x0000FF00,
		.blue = 0x00FF0000,
		.yellow = 0x0000FFFF,
		.magenta = 0x00FF00FF,
		.cyan = 0x00FFFF00,
		.orange = 0x00007FFF,
		.purple = 0x00FF007F,
		.bright_white = 0x00ffffff
	},
	{
		.name = "Monokai Pastel",
		.menu_header = 0x00a96d77,
		.menu_bg = 0xB019181a,
		.menu_selection = 0x00c48f69,
		.white = 0x00fafcfc,
		.black = 0x0051443e,
		.red = 0x008861ff,
		.green = 0x0076dca9,
		.blue = 0x00f5b384,
		.yellow = 0x0066d8ff,
		.magenta = 0x009457ff,
		.cyan = 0x00e8dc78,
		.orange = 0x006798fc,
		.purple = 0x00f29dab,
		.bright_white = 0x00ffffff
	}
};

static void run_menu(void);

static int read_config(JanusConfig *config) {
	int read = SCE_ERR_ININDEX;
	int res = 0;
	SceUID fd = sceIoOpen("flash1:/config.janus", PSP_O_RDONLY, 0);

	// Clean if not failed to open.
	memset(config, 0, sizeof(JanusConfig));

	read = sceIoRead(fd, config, sizeof(JanusConfig));

	if (read < sizeof(JanusConfig)) {
		res = SCE_EIO;
		goto exit;
	}

exit:
	if (fd >= 0) {
		sceIoClose(fd);
	}
	return res;
}

static int save_config(JanusConfig *config) {
	SceUID fd = sceIoOpen("flash1:/config.janus", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	if (fd < 0) {
		return fd;
	}

	config->magic = JANUS_MAGIC;

	int written = sceIoWrite(fd, config, sizeof(JanusConfig));

	if (written < sizeof(JanusConfig)) {
		sceIoClose(fd);
		return SCE_EIO;
	}

	sceIoClose(fd);
	return 0;
}

static int id_in_list(SceUID id, const SceUID *list, int count) {
	for (int i = 0; i < count; i++) {
		if (list[i] == id) {
			return 1;
		}
	}

	return 0;
}

static void capture_base_threads(void) {
	g_base_thread_count = 0;
	sceKernelGetThreadmanIdList(SCE_KERNEL_TMID_Thread, g_base_threads, THREAD_LIST_MAX, &g_base_thread_count);
}

static void pause_game_threads(void) {
	SceUID threads[THREAD_LIST_MAX];
	int count = 0;
	SceUID current = sceKernelGetThreadId();
	int i;

	if (g_paused_thread_count > 0) {
		return;
	}

	sceKernelGetThreadmanIdList(SCE_KERNEL_TMID_Thread, threads, THREAD_LIST_MAX, &count);

	for (i = 0; i < count && g_paused_thread_count < THREAD_LIST_MAX; i++) {
		SceUID thid = threads[i];

		if (thid == current) {
			continue;
		}
		if (id_in_list(thid, g_base_threads, g_base_thread_count)) {
			continue;
		}

		if (sceKernelSuspendThread(thid) >= 0) {
			g_paused_threads[g_paused_thread_count++] = thid;
		}
	}
}

static void resume_game_threads(void) {
	for (int i = g_paused_thread_count - 1; i >= 0; i--) {
		sceKernelResumeThread(g_paused_threads[i]);
	}

	g_paused_thread_count = 0;
}

int GetRegistryData(const char *dir, const char *name, int type, void *data, int data_size) {
	int ret = 0;

	struct RegParam reg;
	memset(&reg, 0, sizeof(reg));
	reg.regtype = 1;
	strcpy(reg.name, "/system");
	reg.namelen = strlen(reg.name);
	reg.unk2 = 1;
	reg.unk3 = 1;

	REGHANDLE h;
	if (sceRegOpenRegistry(&reg, 2, &h) == 0) {
		REGHANDLE hd;
		if (sceRegOpenCategory(h, dir, 2, &hd) == 0) {
			unsigned int reg_type;
			SceSize reg_size;

			REGHANDLE hk;
			if (sceRegGetKeyInfo(hd, name, &hk, &reg_type, &reg_size) == 0) {
				if (type == reg_type) {
					if (sceRegGetKeyValue(hd, hk, data, data_size) == 0) {
						ret = 1;
					}
				}
			}

			sceRegFlushCategory(hd);
			sceRegCloseCategory(hd);
		}

		sceRegFlushRegistry(h);
		sceRegCloseRegistry(h);
	}

	return ret;
}

static int starts_with_icase(const char *s, const char *prefix) {
	while (*prefix) {
		char a = *s++;
		char b = *prefix++;

		if (a >= 'A' && a <= 'Z') {
			a = (char)(a + ('a' - 'A'));
		}
		if (b >= 'A' && b <= 'Z') {
			b = (char)(b + ('a' - 'A'));
		}

		if (a != b) {
			return 0;
		}
	}

	return 1;
}

static int ends_with_icase(const char *s, const char *suffix) {
	int ls = (int)strlen(s);
	int lf = (int)strlen(suffix);

	if (lf > ls) {
		return 0;
	}

	return strcasecmp(s + ls - lf, suffix) == 0;
}

static int is_iso_file(const char *name) {
	return ends_with_icase(name, ".iso") || ends_with_icase(name, ".cso") || ends_with_icase(name, ".jso") || ends_with_icase(name, ".zso") || ends_with_icase(name, ".dax");
}

static void copy_string(char *dst, const char *src, int size) {
	if (size <= 0) {
		return;
	}

	strncpy(dst, src ? src : "", size - 1);
	dst[size - 1] = '\0';
}

static void make_full_path(char *out, const char *dir, const char *name) {
	int len;

	copy_string(out, dir, MAX_PATH_LEN);
	len = (int)strlen(out);
	if (len < MAX_PATH_LEN - 1) {
		copy_string(out + len, name, MAX_PATH_LEN - len);
	}
}

static int make_disc_key(const char *name, char *out, int out_size) {
	int i = 0;
	int o = 0;
	int found = 0;

	while (name[i] && o < out_size - 1) {
		if (starts_with_icase(name + i, "disc")) {
			int j = i + 4;

			while (name[j] == ' ' || name[j] == '_' || name[j] == '-') {
				j++;
			}

			if (name[j] >= '0' && name[j] <= '9') {
				if (o < out_size - 1)
					out[o++] = 'd';
				if (o < out_size - 1)
					out[o++] = 'i';
				if (o < out_size - 1)
					out[o++] = 's';
				if (o < out_size - 1)
					out[o++] = 'c';
				if (o < out_size - 1)
					out[o++] = '#';

				while (name[j] >= '0' && name[j] <= '9')
					j++;

				i = j;
				found = 1;
				continue;
			}
		}

		char c = name[i++];
		if (c >= 'A' && c <= 'Z')
			c = (char)(c + ('a' - 'A'));

		out[o++] = c;
	}

	out[o] = '\0';
	return found;
}

static void split_current_path(void) {
	char *slash;

	int res = isoGetUmdFile(g_current_path, sizeof(g_current_path));
	if (res < 0) {
		const char *umd = sctrlSEGetUmdFile();
		copy_string(g_current_path, umd, sizeof(g_current_path));
	}

	slash = strrchr(g_current_path, '/');
	if (slash == NULL) {
		g_current_dir[0] = '\0';
		copy_string(g_current_name, g_current_path, sizeof(g_current_name));
		return;
	}

	copy_string(g_current_name, slash + 1, sizeof(g_current_name));
	slash[1] = '\0';
	copy_string(g_current_dir, g_current_path, sizeof(g_current_dir));
}

static void split_og_path(void) {
	char *slash;

	int res = isoGetUmdFile(g_og_path, sizeof(g_og_path));
	if (res < 0) {
		const char *umd = sctrlSEGetUmdFile();
		copy_string(g_og_path, umd, sizeof(g_og_path));
	}

	slash = strrchr(g_og_path, '/');
	if (slash == NULL) {
		g_og_dir[0] = '\0';
		copy_string(g_og_name, g_og_path, sizeof(g_og_name));
		return;
	}

	copy_string(g_og_name, slash + 1, sizeof(g_og_name));
	slash[1] = '\0';
	copy_string(g_og_dir, g_og_path, sizeof(g_og_dir));
}

static void swap_entries(DiscEntry *a, DiscEntry *b) {
	DiscEntry tmp = *a;
	*a = *b;
	*b = tmp;
}

static void sort_entries(DiscEntry *entries, int count) {
	for (int i = 1; i < count; i++) {
		for (int j = i; j > 0 && strcasecmp(entries[j - 1].name, entries[j].name) > 0; j--) {
			swap_entries(&entries[j - 1], &entries[j]);
		}
	}
}

static void scan_disc_entries_inner(const char *base) {
	SceIoDirent dent;
	memset(&dent, 0, sizeof(dent));
	SceUID dfd = sceIoDopen(base);

	if (dfd < 0) {
		logmsg("[ERROR]: %s: Dopen Failed -> 0x%08X\n", __func__, dfd);
		snprintf(g_status, sizeof(g_status), "Dopen failed %08X", (unsigned int)dfd);
		return;
	}

	while (sceIoDread(dfd, &dent) > 0) {
		char path[MAX_PATH_LEN] = {0};

		if (!strcmp(dent.d_name, ".") || !strcmp(dent.d_name, "..")) {
			continue;
		}

		if (FIO_SO_ISDIR(dent.d_stat.st_attr)) {
			make_full_path(path, base, dent.d_name);
			scan_disc_entries_inner(path);
			continue;
		}

		if (!FIO_SO_ISREG(dent.d_stat.st_attr) || !is_iso_file(dent.d_name)) {
			continue;
		}

		if (g_all_count >= MAX_ENTRIES) {
			continue;
		}

		copy_string(g_all_entries[g_all_count].name, dent.d_name, MAX_NAME_LEN);

		make_full_path(path, base, dent.d_name);
		copy_string(g_all_entries[g_all_count].path, path, MAX_PATH_LEN);

		g_all_count++;
	}

	sceIoDclose(dfd);
}

static void scan_disc_entries(void) {
	g_all_count = 0;
	g_entry_count = 0;
	g_selected = 0;
	g_current_index = -1;

	char current_key[MAX_KEY_LEN];
	split_current_path();
	int has_key = make_disc_key(g_current_path, current_key, sizeof(current_key));

	scan_disc_entries_inner("ms0:ISO/");
	scan_disc_entries_inner(g_ef_path);
	sort_entries(g_all_entries, g_all_count);

	if (has_key) {
		for (int i = 0; i < g_all_count && g_entry_count < MAX_ENTRIES; i++) {
			char key[MAX_KEY_LEN];

			make_disc_key(g_all_entries[i].path, key, sizeof(key));
			if (strcasecmp(key, current_key) == 0) {
				g_entries[g_entry_count++] = i;
			}
		}
	}

	if (g_entry_count < 2) {
		for (int i = 0; i < g_all_count; i++) {
			g_entries[i] = i;
		}
		g_entry_count = g_all_count;
	}

	for (int i = 0; i < g_entry_count; i++) {
		DiscEntry *entry = &g_all_entries[g_entries[i]];

		if (strcasecmp(entry->path, g_current_path) == 0 ||
			strcasecmp(entry->name, g_current_name) == 0) {
			g_selected = i;
			g_current_index = i;
			break;
		}
	}

	snprintf(g_status, sizeof(g_status), "%d ISO%s found", g_entry_count, g_entry_count == 1 ? "" : "s");
}

static void shorten_middle(char *out, const char *in, int max_chars) {
	int len = (int)strlen(in);
	int head;
	int tail;

	if (max_chars < 8) {
		copy_string(out, in, max_chars + 1);
		return;
	}

	if (len <= max_chars) {
		copy_string(out, in, max_chars + 1);
		return;
	}

	head = (max_chars - 3) / 2;
	tail = max_chars - 3 - head;

	memcpy(out, in, head);
	memcpy(out + head, "...", 3);
	memcpy(out + head + 3, in + len - tail, tail);
	out[max_chars] = '\0';
}

static int buttons_are_up(void) {
	SceCtrlData pad;
	int ret;

	memset(&pad, 0, sizeof(pad));
	ret = sceCtrlPeekBufferPositive(&pad, 1);
	if (ret < 0) {
		return 1;
	}

	return (pad.Buttons & MENU_KEYS) == 0;
}

static void wait_buttons_up(void) {
	while (!g_stop && !buttons_are_up()) {
		sceDisplayWaitVblankStart();
		sceKernelDelayThread(1000);
	}
}

static void draw_menu(void) {
	int first;
	int i;
	char line[128];

	if (blit_setup() < 0) {
		return;
	}

	JanusColorTheme* theme = &g_themes[g_config.theme];

	first = (g_selected / VISIBLE_ROWS) * VISIBLE_ROWS;

	blit_rect(24, 30, 432, 216, theme->menu_bg);
	blit_rect(24, 18, 432, 12, theme->menu_header);

	blit_string(CENTER(23), 21, theme->white, theme->menu_header, "JANUS ISO SWITCHER MENU");

	shorten_middle(line, g_current_name, 45);
	blit_string(40, 35, theme->white, OPAQUE(theme->menu_bg), "Current:");
	blit_string(112, 35, theme->green, OPAQUE(theme->menu_bg), line);
	shorten_middle(line, g_og_name, 45);
	blit_string(40, 47, theme->white, OPAQUE(theme->menu_bg), "Initial:");
	blit_string(112, 47, theme->blue, OPAQUE(theme->menu_bg), line);

	blit_string(40, 59, theme->white, OPAQUE(theme->menu_bg), g_status);
	snprintf(line, sizeof(line), "Page %d/%d | Current theme: %s",
			 g_entry_count == 0 ? 0 : (first / VISIBLE_ROWS) + 1,
			 g_entry_count == 0 ? 0 : ((g_entry_count - 1) / VISIBLE_ROWS) + 1, theme->name);
	blit_string(40, 71, theme->white, OPAQUE(theme->menu_bg), line);


	for (i = 0; i < VISIBLE_ROWS; i++) {
		int idx = first + i;
		int y = 89 + i * 10;
		unsigned int bg = (idx == g_selected) ? theme->menu_selection : OPAQUE(theme->menu_bg);
		unsigned int fg = (idx == g_selected) ? theme->bright_white : theme->white;

		if (idx == g_selected) {
			blit_rect(36, y - 2, 408, 11, bg);
		}

		if (idx >= g_entry_count) {
			continue;
		}

		char current[2];
		char prefix[4];
		char name[64];
		snprintf(current, sizeof(current), "%c", idx == g_current_index ? '>' : ' ');
		snprintf(prefix, sizeof(prefix), "%03d", idx + 1);
		shorten_middle(name, g_all_entries[g_entries[idx]].name, 45);

		blit_string(42, y, theme->red, bg, current);
		blit_string(49, y, fg, bg, prefix);
		blit_string(84, y, fg, bg, name);
	}


	blit_string(40, 219, theme->yellow, OPAQUE(theme->menu_bg), "\x05");
	blit_string(51, 219, theme->white, OPAQUE(theme->menu_bg), "/");
	blit_string(61, 219, theme->yellow, OPAQUE(theme->menu_bg), "\x06");
	blit_string(74, 219, theme->white, OPAQUE(theme->menu_bg), "Selection");
	blit_string(154, 219, theme->yellow, OPAQUE(theme->menu_bg), "\x07");
	blit_string(165, 219, theme->white, OPAQUE(theme->menu_bg), "/");
	blit_string(175, 219, theme->yellow, OPAQUE(theme->menu_bg), "\x08");
	blit_string(186, 219, theme->white, OPAQUE(theme->menu_bg), "/");
	blit_string(197, 219, theme->orange, OPAQUE(theme->menu_bg), "L");
	blit_string(209, 219, theme->white, OPAQUE(theme->menu_bg), "/");
	blit_string(220, 219, theme->orange, OPAQUE(theme->menu_bg), "R");
	blit_string(232, 219, theme->white, OPAQUE(theme->menu_bg), "Page");


	if (g_button_assign == CONFIRM_CIRCLE) {
		blit_string(40, 231, theme->red, OPAQUE(theme->menu_bg), "\x02");
		blit_string(52, 231, theme->white, OPAQUE(theme->menu_bg), "Switch");
		blit_string(105, 231, theme->blue, OPAQUE(theme->menu_bg), "\x04");
		blit_string(117, 231, theme->white, OPAQUE(theme->menu_bg), "Cancel");

	} else if (g_button_assign == CONFIRM_CROSS) {
		blit_string(40, 231, theme->blue, OPAQUE(theme->menu_bg), "\x04");
		blit_string(52, 231, theme->white, OPAQUE(theme->menu_bg), "Switch");
		blit_string(105, 231, theme->red, OPAQUE(theme->menu_bg), "\x02");
		blit_string(117, 231, theme->white, OPAQUE(theme->menu_bg), "Cancel");
	}

	blit_string(170, 231, theme->green, OPAQUE(theme->menu_bg), "\x03");
	blit_string(182, 231, theme->white, OPAQUE(theme->menu_bg), "Reset");
	blit_string(227, 231, theme->magenta, OPAQUE(theme->menu_bg), "\x01");
	blit_string(239, 231, theme->white, OPAQUE(theme->menu_bg), "Change Theme");

	sceKernelDcacheWritebackAll();
}

static void show_result(const char *message, int frames) {
	int i;
	copy_string(g_status, message, sizeof(g_status));

	for (i = 0; i < frames && !g_stop; i++) {
		sceDisplayWaitVblankStart();
		draw_menu();
	}
}

static void run_menu(void) {
	SceCtrlData pad;
	unsigned int old_buttons = 0;
	int done = 0;

	pause_game_threads();
	scan_disc_entries();

	memset(&pad, 0, sizeof(pad));
	memset(&pad, 0, sizeof(pad));
	if (sceCtrlPeekBufferPositive(&pad, 1) < 0) {
		memset(&pad, 0, sizeof(pad));
	}

	old_buttons = pad.Buttons;

	while (!g_stop && !done) {
		unsigned int pressed;

		sceDisplayWaitVblankStart();
		memset(&pad, 0, sizeof(pad));
		if (sceCtrlPeekBufferPositive(&pad, 1) < 0) {
			memset(&pad, 0, sizeof(pad));
		}

		pressed = pad.Buttons & ~old_buttons;
		old_buttons = pad.Buttons;

		// restore_framebuffer();

		if (g_entry_count > 0) {
			if (pressed & PSP_CTRL_UP) {
				g_selected = (g_selected + g_entry_count - 1) % g_entry_count;
			}
			if (pressed & PSP_CTRL_DOWN) {
				g_selected = (g_selected + 1) % g_entry_count;
			}
			if (pressed & (PSP_CTRL_LEFT | PSP_CTRL_LTRIGGER)) {
				g_selected -= VISIBLE_ROWS;
				if (g_selected < 0) {
					g_selected = 0;
				}
			}
			if (pressed & (PSP_CTRL_RIGHT | PSP_CTRL_RTRIGGER)) {
				g_selected += VISIBLE_ROWS;
				if (g_selected >= g_entry_count) {
					g_selected = g_entry_count - 1;
				}
			}
		}

		int cancel = (g_button_assign == CONFIRM_CIRCLE) ? PSP_CTRL_CROSS : PSP_CTRL_CIRCLE;
		int confirm = (g_button_assign == CONFIRM_CIRCLE) ? PSP_CTRL_CIRCLE : PSP_CTRL_CROSS;
		if (pressed & (cancel | PSP_CTRL_SELECT | PSP_CTRL_HOME)) {
			done = 1;

		} else if ((pressed & confirm) && g_entry_count > 0) {
			DiscEntry *entry = &g_all_entries[g_entries[g_selected]];

			if (strcasecmp(entry->path, g_current_path) == 0) {
				show_result("Already selected", 30);
				done = 1;
			} else {
				copy_string(g_status, "Changing disc...", sizeof(g_status));
				draw_menu();
				wait_buttons_up();
				resume_game_threads();
				int ret = isoSwapUmdFile(entry->path, NULL, 0);
				pause_game_threads();
				if (ret == 0) {
					copy_string(g_current_path, entry->path, sizeof(g_current_path));
					copy_string(g_current_name, entry->name, sizeof(g_current_name));
					g_current_index = g_selected;
					show_result("Disc changed", 45);
				} else {
					snprintf(g_status, sizeof(g_status), "Change failed %08X", (unsigned int)ret);
					show_result(g_status, 90);
				}
				done = 1;
			}

		} else if (pressed & PSP_CTRL_SQUARE) {
			g_config.theme = (g_config.theme + 1) % NUM_THEMES;
			save_config(&g_config);

		} else if (pressed & PSP_CTRL_TRIANGLE) {
			if (strcasecmp(g_og_path, g_current_path) == 0) {
				show_result("Already selected", 30);
				done = 1;
			} else {
				copy_string(g_status, "Changing disc...", sizeof(g_status));
				draw_menu();
				wait_buttons_up();
				resume_game_threads();
				int ret = isoSwapUmdFile(g_og_path, NULL, 0);
				pause_game_threads();
				if (ret == 0) {
					copy_string(g_current_path, g_og_path, sizeof(g_current_path));
					copy_string(g_current_name, g_og_name, sizeof(g_current_name));
					g_current_index = g_selected;
					show_result("Disc changed", 45);
				} else {
					snprintf(g_status, sizeof(g_status), "Change failed %08X", (unsigned int)ret);
					show_result(g_status, 90);
				}
				done = 1;
			}
		}

		draw_menu();
	}

	wait_buttons_up();
	resume_game_threads();
}

static int menu_thread(SceSize args, void *argp) {
	SceCtrlData pad;
	unsigned int hotkey_down = 0;
	int api_state_seen = 0;

	(void)args;
	(void)argp;

	memset(&pad, 0, sizeof(pad));

	while (!g_stop) {
		int launch_menu = 0;

		if (sceCtrlPeekBufferPositive(&pad, 1) < 0) {
			memset(&pad, 0, sizeof(pad));
		}

		if ((pad.Buttons & HOTKEY) == HOTKEY) {
			if (!hotkey_down) {
				hotkey_down = 1;
				launch_menu = 1;
			}
		} else {
			hotkey_down = 0;
		}

		if (g_menu_state && !api_state_seen) {
			launch_menu = 1;
		}
		api_state_seen = g_menu_state;
		g_menu_state = 0; // Reset

		if (launch_menu) {
			run_menu();
		}

		sceKernelDelayThread(50000);
	}

	return 0;
}

void janusSetMenuState(int state) {
	g_menu_state = state;
}

int module_start(SceSize args, void *argp) {
	SceUID thid;

	(void)args;
	(void)argp;

	g_stop = 0;
	g_paused_thread_count = 0;
	capture_base_threads();

	read_config(&g_config);
	GetRegistryData("/CONFIG/SYSTEM/XMB", "button_assign", REG_TYPE_INT, &g_button_assign, sizeof(u32));

	int res = isoGetUmdFile(g_og_path, sizeof(g_og_path));
	if (res < 0) {
		strcpy(g_og_path, "ms0:ISO/");
	}
	split_og_path();

	if (sceKernelFindModuleByName("Pentazemin") != NULL) {
		g_ef_path = "ms0:__ef0__/ISO/";
	}

	thid = sceKernelCreateThread("JANUS", menu_thread, 32, 0x4000, 0, NULL);
	if (thid >= 0) {
		sceKernelStartThread(thid, 0, NULL);
	}

	return 0;
}

int module_stop(void) {
	g_stop = 1;
	resume_game_threads();
	return 0;
}
