/*
	Adrenaline XmbControl
	Copyright (C) 2011, Total_Noob
	Copyright (C) 2011, Frostegater
	Copyright (C) 2025, GrayJack

	main.h: XmbControl main header file

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

#ifndef __XMBCTRL_H__
#define __XMBCTRL_H__

#include <stddef.h>

#include <psptypes.h>
#include <psploadcore.h>

#include <systemctrl_se.h>

enum {
	CPU_CLOCK_VSH,
	CPU_CLOCK_GAME,
	UMD_DRIVER,
	SKIP_LOGO_COLDBOOT,
	SKIP_LOGO_GAMEBOOT,
	HIDE_CORRUPT_ICONS,
	HIDE_MAC_ADDR,
	HID_DLCS,
	HIDE_PIC_0_1,
	AUTORUN_BOOT,
	VSH_REGION,
	EXT_COLORS,
	USE_SONY_OSK,
	USE_NO_DRM,
	XMBCNTRL,
	FORCE_HIGHMEM,
	EXEC_BOOT_BIN,
	INFERNO_CACHE_POLICY,
	INFERNO_CACHE_NUM,
	INFERNO_CACHE_SIZE,
	INFERNO_SIM_UMD_SEEK,
	INFERNO_SIM_UMD_SPEED,
	VSH_PLUGINS,
	GAME_PLUGINS,
	POPS_PLUGINS,
};

typedef struct {
	u8 mode;
	u8 negative;
	char *item;
	u8 need_reboot;
	u8 advanced;
	u8 *value;
} GetItem;

typedef struct {
  u8 n;
  char **c;
} ItemOptions;

extern SEConfigEPI g_cfw_config;
extern GetItem g_menu_items[];
extern int g_num_items;
extern ItemOptions g_item_opts[];
extern int g_psp_model;
extern int g_is_ef_drive;
extern int g_game_plugin;
extern int g_savedata_plugin;
extern int g_needs_unload;

extern int g_vshmenu_running;
extern void (*g_vshmenu_draw)(void* frame);

#define sysconf_console_id 4
#define sysconf_console_action 2
#define sysconf_console_action_arg 2
#define ms_photo_action 5
#define ms_music_action 6
#define ms_video_action 10
#define ms_game_action 0xF
#define ms_savedata_action 11

enum {
	default_ms0_game_action_arg = 2,
	sysconf_cfwconfig_action_arg = 0x1010,
	sysconf_plugins_action_arg = 0x1012,
	fake_ef0_game_action_arg = 0x1014,
	fake_ef0_photo_action_arg = 0x1018,
	fake_ef0_music_action_arg = 0x1020,
	fake_ef0_video_action_arg = 0x1022,
	fake_ef0_savedata_action_arg = 4,
};

enum FakeEf {
	FAKE_EF_PHOTO = 0,
	FAKE_EF_MUSIC = 1,
	FAKE_EF_VIDEO = 2,
	FAKE_EF_GAME = 3,
	FAKE_EF_SAVEDATA = 4,
};

enum CustomId {
	CUSTOM_ID_PLUGIN_MNG = 91,
	CUSTOM_ID_CFW_CONFIG = 92,
	CUSTOM_ID_FAKE_EF_PHOTO = 93,
	CUSTOM_ID_FAKE_EF_MUSIC = 94,
	CUSTOM_ID_FAKE_EF_VIDEO = 95,
	CUSTOM_ID_FAKE_EF_GAME = 96,
	CUSTOM_ID_FAKE_EF_SAVEDATA = 97,
};

typedef struct {
	/** The name of the context. */
	char text[48];
	/** Marks if the context play sound on enter. */
	int play_sound;
	/** Action value */
	int action;
	/** Action argument value */
	int action_arg;
} SceContextItem;

typedef struct {
	/** A identification number for the item */
	int id;
	/** Depending on the value of this, a item can be reallocated later (often used for itens that can stop existing like MS items)
	 *
	 * It also seem to be used to differentiate to get the free space on ms0 e ef0 drive.
	*/
	int relocate;
	/** The action type */
	int action;
	/** The action argument. Often similar itens will have  */
	int action_arg;
	/** A list of contexts in the context menu */
	SceContextItem *context;
	/** Subtitle information (?) */
	char *subtitle;
	int unk;
	/** Mark that it play sound on item confirmation */
	char play_sound;
	char memstick;
	char umd_icon;
	char image[4];
	char image_shadow[4];
	char image_glow[4];
	/** The name of the item. Seems to also be used to identify certain itens */
	char text[0x25];
} SceVshItem;

typedef struct {
	void *unk;
	int id;
	char *regkey;
	char *text;
	char *subtitle;
	char *page;
} SceSysconfItem;

typedef struct {
	u8 id;
	u8 type;
	u16 unk1;
	u32 label;
	u32 param;
	u32 first_child;
	int child_count;
	u32 next_entry;
	u32 prev_entry;
	u32 parent;
	u32 unknown[2];
} SceRcoEntry;

#define paf_wcslen sce_paf_private_wcslen
#define paf_sprintf sce_paf_private_sprintf
#define paf_snprintf sce_paf_private_snprintf
#define paf_memcpy sce_paf_private_memcpy
#define paf_memset sce_paf_private_memset
#define paf_strlen sce_paf_private_strlen
#define paf_strcpy sce_paf_private_strcpy
#define paf_strncpy sce_paf_private_strncpy
#define paf_strcmp sce_paf_private_strcmp
#define paf_strncmp sce_paf_private_strncmp
#define paf_strcasecmp sce_paf_private_strcasecmp
#define paf_strncasecmp sce_paf_private_strncasecmp
#define paf_strchr sce_paf_private_strchr
#define paf_strrchr sce_paf_private_strrchr
#define paf_strpbrk sce_paf_private_strpbrk
#define paf_strtoul sce_paf_private_strtoul
#define paf_malloc sce_paf_private_malloc
#define paf_free sce_paf_private_free
#define paf_strstr sce_paf_private_strstr
#define paf_memalign sce_paf_private_memalign

int sce_paf_private_wcslen(wchar_t *);
int sce_paf_private_sprintf(char *, const char *, ...);
int sce_paf_private_snprintf(char *, SceSize, const char *, ...);
void *sce_paf_private_memcpy(void *, void *, int);
void *sce_paf_private_memset(void *, char, int);
int sce_paf_private_strlen(char *);
char *sce_paf_private_strcpy(char *, const char *);
char *sce_paf_private_strncpy(char *, const char *, int);
int sce_paf_private_strcmp(const char *, const char *);
int sce_paf_private_strncmp(const char *, const char *, int);
int sce_paf_private_strcasecmp(const char *, const char *);
int sce_paf_private_strncasecmp(const char *, const char *, SceSize);
char *sce_paf_private_strchr(const char *, int);
char *sce_paf_private_strrchr(const char *, int);
int sce_paf_private_strpbrk(const char *, const char *);
int sce_paf_private_strtoul(const char *, char **, int);
void *sce_paf_private_malloc(int);
void *sce_paf_private_memalign(int, int);
void sce_paf_private_free(void *);
char* sce_paf_private_strstr(const char *, const char *);

wchar_t *scePafGetText(void *, char *);
int PAF_Resource_GetPageNodeByID(void *, char *, SceRcoEntry **);
int PAF_Resource_ResolveRefWString(void *, u32 *, int *, char **, int *);

int vshGetRegistryValue(u32 *, char *, void *, int , int *);
int vshSetRegistryValue(u32 *, char *, int , int *);
int vctrlVSHUpdateConfig(SEConfig *config);

int sceVshCommonGuiBottomDialog(void *a0, void *a1, void *a2, int (* cancel_handler)(), void *t0, void *t1, int (* handler)(), void *t3);

void saveSettings(void);
void loadSettings(void);

void PatchVshMain(u32 text_addr, u32 text_size);
void PatchAuthPlugin(u32 text_addr, u32 text_size);
void PatchSysconfPlugin(u32 text_addr, u32 text_size);
void PatchGamePlugin(SceModule *mod);
void PatchSavedataPlugin(SceModule *mod);
void PatchIo(SceModule *mod);
void patchVshClock(u32 addr);

int vshgu_init();

#endif // __XMBCTRL_H__
