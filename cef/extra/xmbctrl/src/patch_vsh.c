/*
	Adrenaline
	Copyright (C) 2016-2018, TheFloW
	Copyright (C) 2025, GrayJack

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
#include <psputility_sysparam.h>

#include <vshctrl.h>
#include <cfwmacros.h>
#include <systemctrl_adrenaline.h>

#include <adrenaline_log.h>

#include "xmbctrl.h"
#include "utils.h"

#include "list.h"
#include "plugins.h"

static int g_sysconf_action = 0;
static u32 g_sysconf_unk = 0;
static u32 g_sysconf_option = 0;
static int g_startup = 1;

static int g_is_cfw_config = 0;
static int g_unload = 0;
static int g_context_mode = 0;
static u32 g_backup[4];

static SceVshItem *g_plugin_mgr_item = NULL;
static SceVshItem *g_cfw_conf_item = NULL;
static SceVshItem *g_ef_item[5] = {NULL};
static void *g_xmb_arg0 = NULL;
static void *g_xmb_arg1 = NULL;

static char g_user_buffer[LINE_BUFFER_SIZE];
static char g_buf[64];

static char *g_need_reboot_subtitle = "Requires restarting VSH to take effect";

static unsigned char g_console_item[] __attribute__((aligned(16))) = {
	0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
	0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x42, 0x58, 0x00, 0x00, 0x43,
	0x4C, 0x00, 0x00, 0x43, 0x5A, 0x00, 0x00, 0x6D, 0x73, 0x67, 0x74, 0x6F,
	0x70, 0x5F, 0x73, 0x79, 0x73, 0x63, 0x6F, 0x6E, 0x66, 0x5F, 0x63, 0x6F,
	0x6E, 0x73, 0x6F, 0x6C, 0x65, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static unsigned char g_usb_item[] __attribute__((aligned(16))) = {
	0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
	0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x42, 0x55, 0x00, 0x00, 0x43,
	0x49, 0x00, 0x00, 0x43, 0x57, 0x00, 0x00, 0x6D, 0x73, 0x67, 0x74, 0x6F,
	0x70, 0x5F, 0x73, 0x79, 0x73, 0x63, 0x6F, 0x6E, 0x66, 0x5F, 0x75, 0x73,
	0x62, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static SceVshItem *g_ef_camera_item = NULL;
static SceVshItem *g_ef_music_item = NULL;
static SceVshItem *g_ef_video_item = NULL;
static SceVshItem *g_ef_game_item = NULL;
static SceVshItem *g_ef_savedata_item = NULL;

static int g_ef_drv_id[5] = {-1};
static int g_ef_drv_arg[5] = {-1};
int g_is_ef_drive = 0;

static u32 g_last_action_arg = ms_game_action;
static u32 g_unk_strukt_addr = 0;

#define PLUGINS_CONTEXT 1

////////////////////////////////////////////////////////////////////////////////
// HELPERS
////////////////////////////////////////////////////////////////////////////////

static int (*AddVshItem)(void *a0, int topitem, SceVshItem *item) = NULL;
static SceSysconfItem *(*GetSysconfItem)(void *a0, void *a1) = NULL;
static int (*ExecuteAction)(int action, int action_arg) = NULL;
static int (*UnloadModule)(int skip) = NULL;
static int (*OnXmbPush)(void *arg0, void *arg1) = NULL;
static int (*OnXmbContextMenu)(void *arg0, void *arg1) = NULL;
SceVshItem *(*GetBackupVshItem)(int topitem, u32 unk, SceVshItem *item) = NULL;
static int (*RegisterDriveCallbacks)(u32 drives, int unk0, int unk2, int unk3) = NULL;


static void (*LoadStartAuth)() = NULL;
static int (*auth_handler)(int a0) = NULL;
static void (*OnRetry)() = NULL;

static void (*AddSysconfItem)(u32 *option, SceSysconfItem **item) = NULL;
static void (*OnInitMenuPspConfig)() = NULL;

static void *addCustomVshItem(int id, char *text, int action_arg, SceVshItem *orig) {
	SceVshItem *item = (SceVshItem *)paf_malloc(sizeof(SceVshItem));
	paf_memcpy(item, orig, sizeof(SceVshItem));

	item->id = id; // custom id
	item->action = g_sysconf_action;
	item->action_arg = action_arg;
	item->play_sound = 1;
	item->context = NULL;
	paf_strcpy(item->text, text);

	return item;
}

static void AddSysconfContextItem(char *text, char *subtitle, char *regkey) {
	SceSysconfItem *item = (SceSysconfItem *)paf_malloc(sizeof(SceSysconfItem));

	item->id = 5;
	item->unk = (u32 *)g_sysconf_unk;
	item->regkey = regkey;
	item->text = text;
	item->subtitle = subtitle;
	item->page = "page_psp_config_umd_autoboot";

	((u32 *)g_sysconf_option)[2] = 1;

	AddSysconfItem((u32 *)g_sysconf_option, &item);
}

static void HijackContext(SceRcoEntry *src, char **options, int n) {
	SceRcoEntry *plane = (SceRcoEntry *)((u32)src + src->first_child);
	SceRcoEntry *mlist = (SceRcoEntry *)((u32)plane + plane->first_child);
	u32 *mlist_param = (u32 *)((u32)mlist + mlist->param);

	/* Backup */
	if (g_backup[0] == 0 && g_backup[1] == 0 && g_backup[2] == 0 && g_backup[3] == 0) {
		g_backup[0] = mlist->first_child;
		g_backup[1] = mlist->child_count;
		g_backup[2] = mlist_param[16];
		g_backup[3] = mlist_param[18];
	}

	if (g_context_mode) {
		SceRcoEntry *base = (SceRcoEntry *)((u32)mlist + mlist->first_child);

		SceRcoEntry *item =
			(SceRcoEntry *)paf_malloc(base->next_entry * n);
		u32 *item_param = (u32 *)((u32)item + base->param);

		mlist->first_child = (u32)item - (u32)mlist;
		mlist->child_count = n;
		mlist_param[16] = 13;
		mlist_param[18] = 6;

		for (int i = 0; i < n; i++) {
			paf_memcpy(item, base, base->next_entry);

			item_param[0] = 0xDEAD;
			item_param[1] = (u32)options[i];

			if (i != 0)
				item->prev_entry = item->next_entry;
			if (i == n - 1)
				item->next_entry = 0;

			item = (SceRcoEntry *)((u32)item + base->next_entry);
			item_param = (u32 *)((u32)item + base->param);
		}
	} else {
		// Restore
		mlist->first_child = g_backup[0];
		mlist->child_count = g_backup[1];
		mlist_param[16] = g_backup[2];
		mlist_param[18] = g_backup[3];
	}

	sceKernelDcacheWritebackAll();
}

static int auth_handler_new(int a0) {
	g_startup = a0;
	return auth_handler(a0);
}

////////////////////////////////////////////////////////////////////////////////
// PATCHED IMPLEMENTATIONS
////////////////////////////////////////////////////////////////////////////////

int AddVshItemPatched(void *a0, int topitem, SceVshItem *item) {
	static int cfw_conf_added = 0;
	static int plugin_mgr_added = 0;

	if (topitem != 0) {
		logmsg4("[DEBUG]: text=%s topitem=%d id=%d action=%d arg=%d subtitle=%s relocate=%d unk=%d memstick=%d umd_icon=%d context.text=%s\n", item->text, topitem, item->id, item->action, item->action_arg, item->subtitle, item->relocate, item->unk, item->memstick, item->umd_icon, item->context->text);
	}

	if (paf_strcmp(item->text, "msgtop_sysconf_console") == 0) {
		g_sysconf_action = item->action;

		// Plugin Manager is added before the `msgtop_sysconf_console`
		// So apply re-patch the action for it
		g_plugin_mgr_item->action = g_sysconf_action;
	}

	// prevent adding more than once
	// Add the item just after "msgtop_sysconf_usb" (that is also just before "msgtop_sysconf_video")
	if (!plugin_mgr_added && paf_strcmp(item->text, "msgtop_sysconf_video") == 0) {
		plugin_mgr_added = 1;
		g_startup = 0;

		int cur_icon = 0;

		if (g_psp_model == PSP_11000) {
			u32 value = 0;
			vctrlGetRegistryValue("/CONFIG/SYSTEM/XMB/THEME", "custom_theme_mode", &value);
			cur_icon = !value;
		}

		// Add Plugins Manager
		g_plugin_mgr_item = addCustomVshItem(CUSTOM_ID_PLUGIN_MNG, "msgtop_sysconf_pluginsmgr", sysconf_plugins_action_arg, (cur_icon) ? item : (SceVshItem*)g_usb_item);
		AddVshItem(a0, topitem, g_plugin_mgr_item);
	}

	// prevent adding more than once
	// Add the item just after "msgtop_sysconf_console" (that is also just before "msgtop_sysconf_theme")
	if (!cfw_conf_added && paf_strcmp(item->text, "msgtop_sysconf_theme") == 0) {
		cfw_conf_added = 1;
		g_startup = 0;

		int cur_icon = 0;

		if (g_psp_model == PSP_11000) {
			u32 value = 0;
			vctrlGetRegistryValue("/CONFIG/SYSTEM/XMB/THEME", "custom_theme_mode", &value);
			cur_icon = !value;
		}

		// Add CFW Settings
		g_cfw_conf_item = addCustomVshItem(CUSTOM_ID_CFW_CONFIG, "msgtop_sysconf_cfwconfig", sysconf_cfwconfig_action_arg, (cur_icon) ? item : (SceVshItem*)g_console_item);
		AddVshItem(a0, topitem, g_cfw_conf_item);
	}

	int is_ef_enable = sctrlIsEfEnable();

	if (is_ef_enable && topitem == 5 && paf_strcmp(item->text, "msgtop_game_savedata") == 0) {
		g_ef_drv_id[FAKE_EF_SAVEDATA] = g_ef_savedata_item->id;
		g_ef_drv_arg[FAKE_EF_SAVEDATA] = g_ef_savedata_item->action_arg;

		// g_ef_item[FAKE_EF_SAVEDATA] = addCustomVshItem(CUSTOM_ID_FAKE_EF_SAVEDATA, "msgtop_game_savedata", fake_ef0_savedata_action_arg, item);
		g_ef_item[FAKE_EF_SAVEDATA] = addCustomVshItem(g_ef_savedata_item->id, "msgtop_game_savedata", g_ef_savedata_item->action_arg, item);
		g_ef_item[FAKE_EF_SAVEDATA]->action = g_ef_savedata_item->action;
		g_ef_item[FAKE_EF_SAVEDATA]->unk = g_ef_savedata_item->unk;
		g_ef_item[FAKE_EF_SAVEDATA]->memstick = g_ef_savedata_item->memstick;
		// g_ef_item[FAKE_EF_SAVEDATA]->context = g_ef_savedata_item->context;

		logmsg4("[INFO]: text=%s topitem=%d id=%d action=%d arg=%d subtitle=%s relocate=%d unk=%d memstick=%d umd_icon=%d context.text=%s\n", g_ef_savedata_item->text, topitem, g_ef_savedata_item->id, g_ef_savedata_item->action, g_ef_savedata_item->action_arg, g_ef_savedata_item->subtitle, g_ef_savedata_item->relocate, g_ef_savedata_item->unk, g_ef_savedata_item->memstick, g_ef_savedata_item->umd_icon, g_ef_savedata_item->context->text);
		// logmsg4("[DEBUG]: text=%s topitem=%d id=%d action=%d arg=%d subtitle=%s relocate=%d unk=%d memstick=%d umd_icon=%d\n", g_ef_item[FAKE_EF_SAVEDATA]->text, topitem, g_ef_item[FAKE_EF_SAVEDATA]->id, g_ef_item[FAKE_EF_SAVEDATA]->action, g_ef_item[FAKE_EF_SAVEDATA]->action_arg, g_ef_item[FAKE_EF_SAVEDATA]->subtitle, g_ef_item[FAKE_EF_SAVEDATA]->relocate, g_ef_item[FAKE_EF_SAVEDATA]->unk, g_ef_item[FAKE_EF_SAVEDATA]->memstick, g_ef_item[FAKE_EF_SAVEDATA]->umd_icon);

		AddVshItem(a0, topitem, g_ef_item[FAKE_EF_SAVEDATA]);
		// AddVshItem(a0, topitem, g_ef_savedata_item);
	}

	int ret = AddVshItem(a0, topitem, item);

	if (is_ef_enable && topitem == 2 && paf_strcmp(item->text, "msgshare_ms") == 0) {
		logmsg4("[SAVEDATA]: text=%s topitem=%d id=%d action=%d arg=%d subtitle=%s relocate=%d unk=%d memstick=%d umd_icon=%d context.text=%s\n", g_ef_savedata_item->text, topitem, g_ef_savedata_item->id, g_ef_savedata_item->action, g_ef_savedata_item->action_arg, g_ef_savedata_item->subtitle, g_ef_savedata_item->relocate, g_ef_savedata_item->unk, g_ef_savedata_item->memstick, g_ef_savedata_item->umd_icon, g_ef_savedata_item->context->text);
		g_ef_drv_id[FAKE_EF_PHOTO] = g_ef_camera_item->id;
		g_ef_drv_arg[FAKE_EF_PHOTO] = g_ef_camera_item->action_arg;
		g_ef_item[FAKE_EF_PHOTO] = addCustomVshItem(g_ef_camera_item->id, "msg_em", g_ef_camera_item->action_arg, item);
		// g_ef_item[FAKE_EF_PHOTO] = addCustomVshItem(CUSTOM_ID_FAKE_EF_PHOTO, "msg_em", fake_ef0_photo_action_arg, item);
		g_ef_item[FAKE_EF_PHOTO]->action = g_ef_camera_item->action;
		g_ef_item[FAKE_EF_PHOTO]->unk = g_ef_camera_item->unk;
		// g_ef_item[FAKE_EF_PHOTO]->context = g_ef_camera_item->context;

		logmsg4("[INFO]: text=%s topitem=%d id=%d action=%d arg=%d subtitle=%s relocate=%d unk=%d memstick=%d umd_icon=%d context.text=%s\n", g_ef_camera_item->text, topitem, g_ef_camera_item->id, g_ef_camera_item->action, g_ef_camera_item->action_arg, g_ef_camera_item->subtitle, g_ef_camera_item->relocate, g_ef_camera_item->unk, g_ef_camera_item->memstick, g_ef_camera_item->umd_icon, g_ef_camera_item->context->text);
		logmsg4("[DEBUG]: text=%s topitem=%d id=%d action=%d arg=%d subtitle=%s relocate=%d unk=%d memstick=%d umd_icon=%d\n", g_ef_item[FAKE_EF_PHOTO]->text, topitem, g_ef_item[FAKE_EF_PHOTO]->id, g_ef_item[FAKE_EF_PHOTO]->action, g_ef_item[FAKE_EF_PHOTO]->action_arg, g_ef_item[FAKE_EF_PHOTO]->subtitle, g_ef_item[FAKE_EF_PHOTO]->relocate, g_ef_item[FAKE_EF_PHOTO]->unk, g_ef_item[FAKE_EF_PHOTO]->memstick, g_ef_item[FAKE_EF_PHOTO]->umd_icon);
		AddVshItem(a0, topitem, g_ef_item[FAKE_EF_PHOTO]);
	}

	if (is_ef_enable && topitem == 3 && paf_strcmp(item->text, "msgshare_ms") == 0) {
		g_ef_drv_id[FAKE_EF_MUSIC] = g_ef_music_item->id;
		g_ef_drv_arg[FAKE_EF_MUSIC] = g_ef_music_item->action_arg;
		g_ef_item[FAKE_EF_MUSIC] = addCustomVshItem(g_ef_music_item->id, "msg_em",  g_ef_music_item->action_arg, item);
		// g_ef_item[FAKE_EF_MUSIC] = addCustomVshItem(CUSTOM_ID_FAKE_EF_MUSIC, "msg_em", fake_ef0_music_action_arg, item);
		g_ef_item[FAKE_EF_MUSIC]->action = g_ef_music_item->action;
		g_ef_item[FAKE_EF_MUSIC]->unk = g_ef_music_item->unk;
		// g_ef_item[FAKE_EF_MUSIC]->context = g_ef_music_item->context;

		logmsg4("[INFO]: text=%s topitem=%d id=%d action=%d arg=%d subtitle=%s relocate=%d unk=%d memstick=%d umd_icon=%d context.text=%s\n", g_ef_music_item->text, topitem, g_ef_music_item->id, g_ef_music_item->action, g_ef_music_item->action_arg, g_ef_music_item->subtitle, g_ef_music_item->relocate, g_ef_music_item->unk, g_ef_music_item->memstick, g_ef_music_item->umd_icon, g_ef_music_item->context->text);
		logmsg4("[DEBUG]: text=%s topitem=%d id=%d action=%d arg=%d subtitle=%s relocate=%d unk=%d memstick=%d umd_icon=%d\n", g_ef_item[FAKE_EF_MUSIC]->text, topitem, g_ef_item[FAKE_EF_MUSIC]->id, g_ef_item[FAKE_EF_MUSIC]->action, g_ef_item[FAKE_EF_MUSIC]->action_arg, g_ef_item[FAKE_EF_MUSIC]->subtitle, g_ef_item[FAKE_EF_MUSIC]->relocate, g_ef_item[FAKE_EF_MUSIC]->unk, g_ef_item[FAKE_EF_MUSIC]->memstick, g_ef_item[FAKE_EF_MUSIC]->umd_icon);
		AddVshItem(a0, topitem, g_ef_item[FAKE_EF_MUSIC]);
	}

	if (is_ef_enable && topitem == 4 && paf_strcmp(item->text, "msgshare_ms") == 0) {
		g_ef_drv_id[FAKE_EF_VIDEO] = g_ef_video_item->id;
		g_ef_drv_arg[FAKE_EF_VIDEO] = g_ef_video_item->action_arg;
		g_ef_item[FAKE_EF_VIDEO] = addCustomVshItem(g_ef_video_item->id, "msg_em", g_ef_video_item->action_arg, item);
		// g_ef_item[FAKE_EF_VIDEO] = addCustomVshItem(CUSTOM_ID_FAKE_EF_VIDEO, "msg_em", fake_ef0_video_action_arg, item);
		g_ef_item[FAKE_EF_VIDEO]->action = g_ef_video_item->action;
		g_ef_item[FAKE_EF_VIDEO]->unk = g_ef_video_item->unk;
		// g_ef_item[FAKE_EF_VIDEO]->context = g_ef_video_item->context;

		logmsg4("[INFO]: text=%s topitem=%d id=%d action=%d arg=%d subtitle=%s relocate=%d unk=%d memstick=%d umd_icon=%d context.text=%s\n", g_ef_video_item->text, topitem, g_ef_video_item->id, g_ef_video_item->action, g_ef_video_item->action_arg, g_ef_video_item->subtitle, g_ef_video_item->relocate, g_ef_video_item->unk, g_ef_video_item->memstick, g_ef_video_item->umd_icon, g_ef_video_item->context->text);
		logmsg4("[DEBUG]: text=%s topitem=%d id=%d action=%d arg=%d subtitle=%s relocate=%d unk=%d memstick=%d umd_icon=%d\n", g_ef_item[FAKE_EF_VIDEO]->text, topitem, g_ef_item[FAKE_EF_VIDEO]->id, g_ef_item[FAKE_EF_VIDEO]->action, g_ef_item[FAKE_EF_VIDEO]->action_arg, g_ef_item[FAKE_EF_VIDEO]->subtitle, g_ef_item[FAKE_EF_VIDEO]->relocate, g_ef_item[FAKE_EF_VIDEO]->unk, g_ef_item[FAKE_EF_VIDEO]->memstick, g_ef_item[FAKE_EF_VIDEO]->umd_icon);
		AddVshItem(a0, topitem, g_ef_item[FAKE_EF_VIDEO]);
	}

	if (is_ef_enable && topitem == 5 && (paf_strcmp(item->text, "msgshare_ms") == 0 || paf_strcmp(item->text, "gc4") == 0 || paf_strstr(item->text, "gcv_") != NULL || paf_strstr(item->text, "gcw_") != NULL)) {
		g_ef_drv_id[FAKE_EF_GAME] = g_ef_game_item->id;
		g_ef_drv_arg[FAKE_EF_GAME] = g_ef_game_item->action_arg;
		// g_ef_item[FAKE_EF_GAME] = addCustomVshItem(CUSTOM_ID_FAKE_EF_GAME, "msg_em", fake_ef0_game_action_arg, item);
		// g_ef_item[FAKE_EF_GAME] = addCustomVshItem(g_ef_game_item->id, "msg_em", fake_ef0_game_action_arg, item);
		g_ef_item[FAKE_EF_GAME] = addCustomVshItem(g_ef_game_item->id, "msg_em", g_ef_game_item->action_arg, item);
		g_ef_item[FAKE_EF_GAME]->action = g_ef_game_item->action;
		g_ef_item[FAKE_EF_GAME]->unk = g_ef_game_item->unk;
		g_ef_item[FAKE_EF_GAME]->context = g_ef_game_item->context;

		logmsg4("[INFO]: text=%s topitem=%d id=%d action=%d arg=%d subtitle=%s relocate=%d unk=%d memstick=%d umd_icon=%d context.text=%s\n", g_ef_game_item->text, topitem, g_ef_game_item->id, g_ef_game_item->action, g_ef_game_item->action_arg, g_ef_game_item->subtitle, g_ef_game_item->relocate, g_ef_game_item->unk, g_ef_game_item->memstick, g_ef_game_item->umd_icon, g_ef_game_item->context->text);
		logmsg4("[DEBUG]: text=%s topitem=%d id=%d action=%d arg=%d subtitle=%s relocate=%d unk=%d memstick=%d umd_icon=%d\n", g_ef_item[FAKE_EF_GAME]->text, topitem, g_ef_item[FAKE_EF_GAME]->id, g_ef_item[FAKE_EF_GAME]->action, g_ef_item[FAKE_EF_GAME]->action_arg, g_ef_item[FAKE_EF_GAME]->subtitle, g_ef_item[FAKE_EF_GAME]->relocate, g_ef_item[FAKE_EF_GAME]->unk, g_ef_item[FAKE_EF_GAME]->memstick, g_ef_item[FAKE_EF_GAME]->umd_icon);
		AddVshItem(a0, topitem, g_ef_item[FAKE_EF_GAME]);

	}

	return ret;
}

int OnXmbPushPatched(void *arg0, void *arg1) {
	g_xmb_arg0 = arg0;
	g_xmb_arg1 = arg1;
	return OnXmbPush(arg0, arg1);
}

int OnXmbContextMenuPatched(void *arg0, void *arg1) {
	g_plugin_mgr_item->context = NULL;
	g_cfw_conf_item->context = NULL;
	// g_ef_item[FAKE_EF_PHOTO]->context = NULL;
	// g_ef_item[FAKE_EF_MUSIC]->context = NULL;
	// g_ef_item[FAKE_EF_VIDEO]->context = NULL;
	// g_ef_item[FAKE_EF_GAME]->context = NULL;
	// g_ef_item[FAKE_EF_SAVEDATA]->context = NULL;
	return OnXmbContextMenu(arg0, arg1);
}

int ExecuteActionPatched(int action, int action_arg) {
	int old_is_cfw_config = g_is_cfw_config;

	if (g_game_plugin || g_savedata_plugin) {
		if (action_arg != g_last_action_arg) {
			g_unload = 1;
		}
	}

	g_last_action_arg = action_arg;

	if (action == sysconf_console_action) {
		if (action_arg == sysconf_cfwconfig_action_arg) {
			g_is_cfw_config = 1;
			action = sysconf_console_action;
			action_arg = sysconf_console_action_arg;
		} else if (action_arg == sysconf_plugins_action_arg) {
			g_is_cfw_config = 2;
			action = sysconf_console_action;
			action_arg = sysconf_console_action_arg;
		} else {
			g_is_cfw_config = 0;
		}
	}

	if (action == ms_savedata_action) {
		if (action_arg == fake_ef0_savedata_action_arg) {
			g_is_ef_drive = 1;
			action_arg = g_ef_drv_arg[FAKE_EF_SAVEDATA];
		} else if (action_arg == g_ef_drv_arg[FAKE_EF_SAVEDATA]) {
			g_is_ef_drive = 1;
		} else {
			g_last_action_arg = action_arg;
			g_is_ef_drive = 0;
		}
	}

	if (action == ms_game_action) {
		if (action_arg == fake_ef0_game_action_arg) {
			g_is_ef_drive = 1;
			action_arg = default_ms0_game_action_arg;
		} else if (action_arg == g_ef_drv_arg[FAKE_EF_GAME]) {
			g_is_ef_drive = 1;
			action_arg = default_ms0_game_action_arg;
		} else {
			g_last_action_arg = action_arg;
			g_is_ef_drive = 0;
		}

	}

	if (action == ms_photo_action) {
		if (action_arg == fake_ef0_photo_action_arg) {
			g_is_ef_drive = 1;
			action_arg = g_ef_drv_arg[FAKE_EF_PHOTO];
		} else if (action_arg == g_ef_drv_arg[FAKE_EF_PHOTO]) {
			g_is_ef_drive = 1;
		} else {
			g_is_ef_drive = 0;
		}
	}

	if (action == ms_video_action) {
		if (action_arg == fake_ef0_video_action_arg) {
			g_is_ef_drive = 1;
			action_arg = g_ef_drv_arg[FAKE_EF_VIDEO];
		} else if (action_arg == g_ef_drv_arg[FAKE_EF_VIDEO]) {
			g_is_ef_drive = 1;
		} else {
			g_is_ef_drive = 0;
		}
	}

	if (action == ms_music_action) {
		if (action_arg == fake_ef0_music_action_arg) {
			g_is_ef_drive = 1;
			action_arg = g_ef_drv_arg[FAKE_EF_MUSIC];
		} else if (action_arg == g_ef_drv_arg[FAKE_EF_MUSIC]) {
			g_is_ef_drive = 1;
		} else {
			g_is_ef_drive = 0;
		}
	}


	if (old_is_cfw_config != g_is_cfw_config) {
		paf_memset(g_backup, 0, sizeof(g_backup));
		g_context_mode = 0;

		g_unload = 1;
	}

	return ExecuteAction(action, action_arg);
}

int UnloadModulePatched(int skip) {
	if (g_unload) {
		skip = -1;
		g_game_plugin = 0;
		g_savedata_plugin = 0;
		g_unload = 0;
	}
	return UnloadModule(skip);
}

void OnInitMenuPspConfigPatched() {
	if (g_is_cfw_config == 1) {
		if (((u32 *)g_sysconf_option)[2] == 0) {
			loadSettings();
			for (int i = 0; i < g_num_items; i++) {
				if (g_menu_items[i].need_reboot == 1) {
					AddSysconfContextItem(g_menu_items[i].item, "cfw_need_restart", g_menu_items[i].item);
				} else {
					AddSysconfContextItem(g_menu_items[i].item, NULL, g_menu_items[i].item);
				}
			}
		}
	} else if (g_is_cfw_config == 2) {
		if (((u32 *)g_sysconf_option)[2] == 0) {
			loadPlugins();
			for (int i = 0; i < g_plugins.count; i++) {
				Plugin *plugin = (Plugin *)(g_plugins.table[i]);
				if (plugin->name != NULL && plugin->surname != NULL) {
					AddSysconfContextItem(plugin->name, plugin->surname, plugin->name);
					logmsg4("[DEBUG]: Plugin Manager: Registered %s\n", plugin->name);
				}
			}
		}
	} else {
		OnInitMenuPspConfig();
	}
}

SceSysconfItem *GetSysconfItemPatched(void *a0, void *a1) {
	SceSysconfItem *item = GetSysconfItem(a0, a1);

	if (g_is_cfw_config == 1) {
		for (int i = 0; i < g_num_items; i++) {
			if (paf_strcmp(item->text, g_menu_items[i].item) == 0) {
				g_context_mode = g_menu_items[i].mode;
			}
		}
	} else if (g_is_cfw_config == 2) {
		g_context_mode = PLUGINS_CONTEXT;
	}
	return item;
}

wchar_t *scePafGetTextPatched(void *a0, char *name) {
	if (name) {
		if (g_is_cfw_config == 1) {
			if (paf_strncmp(name, "cfw_need_restart", 17) == 0) {
				utf8_to_unicode((wchar_t *)g_user_buffer, g_need_reboot_subtitle);
				return (wchar_t *)g_user_buffer;
			}
			for (int i = 0; i < g_num_items; i++) {
				if (paf_strcmp(name, g_menu_items[i].item) == 0) {
					if (g_menu_items[i].advanced) {
						static char buff[128];
						paf_snprintf(buff, 128, ADVANCED " %s", g_menu_items[i].item);
						utf8_to_unicode((wchar_t *)g_user_buffer, buff);
					}else {
						utf8_to_unicode((wchar_t *)g_user_buffer, g_menu_items[i].item);
					}
					return (wchar_t *)g_user_buffer;
				}
			}
		} else if (g_is_cfw_config == 2) {
			if (paf_strncmp(name, "plugin_", 7) == 0) {
				u32 i = paf_strtoul(name + 7, NULL, 10);
				Plugin *plugin = (Plugin *)(g_plugins.table[i]);
				int len = paf_strlen(plugin->path) + paf_strlen(plugin->runlevel) + 5;
				char *file = paf_malloc(len);

				if (file != NULL) {
					paf_memset(file, 0, len);
					utf8_to_unicode((wchar_t *)g_user_buffer, getPluginDisplayName(plugin, file, len));
					paf_free(file);
				} else {
					logmsg("[ERROR]: Failed to allocate plugin display name\n");
				}

				return (wchar_t *)g_user_buffer;
			} else if (paf_strncmp(name, "plugins", 7) == 0) {
				u32 i = paf_strtoul(name + 7, NULL, 10);
				Plugin *plugin = (Plugin *)(g_plugins.table[i]);
				utf8_to_unicode((wchar_t *)g_user_buffer, plugin->path);
				return (wchar_t *)g_user_buffer;
			}
		}

		if (paf_strcmp(name, "msgtop_sysconf_cfwconfig") == 0) {
			paf_snprintf(g_buf, 63, STAR " Epinephrine CFW Settings");
			utf8_to_unicode((wchar_t *)g_user_buffer, g_buf);
			return (wchar_t *)g_user_buffer;
		} else if (paf_strcmp(name, "msgtop_sysconf_pluginsmgr") == 0) {
			paf_snprintf(g_buf, 63, STAR " Plugins Manager");
			utf8_to_unicode((wchar_t *)g_user_buffer, g_buf);
			return (wchar_t *)g_user_buffer;
		}
	}

	wchar_t *res = scePafGetText(a0, name);

	return res;
}

int vshGetRegistryValuePatched(u32 *option, char *name, void *arg2, int size,int *value) {
	if (name) {
		if (g_is_cfw_config == 1) {
			for (int i = 0; i < g_num_items; i++) {
				if (paf_strcmp(name, g_menu_items[i].item) == 0) {
					g_context_mode = g_menu_items[i].mode;
					*value = (g_menu_items[i].negative) ? !(*g_menu_items[i].value) : *g_menu_items[i].value;
					return 0;
				}
			}
		} else if (g_is_cfw_config == 2) {
			if (paf_strncmp(name, "plugin_", 7) == 0) {
				u32 i = paf_strtoul(name + 7, NULL, 10);
				Plugin *plugin = (Plugin *)(g_plugins.table[i]);
				g_context_mode = PLUGINS_CONTEXT;
				*value = plugin->active;
				return 0;
			}
		}
	}

	int res = vshGetRegistryValue(option, name, arg2, size, value);

	return res;
}

int vshSetRegistryValuePatched(u32 *option, char *name, int size, int *value) {
	logmsg4("[DEBUG]: %s: name=%s\n", __func__, name);
	if (name) {
		if (g_is_cfw_config == 1) {
			for (int i = 0; i < g_num_items; i++) {
				if (paf_strcmp(name, g_menu_items[i].item) == 0) {
					*g_menu_items[i].value = g_menu_items[i].negative ? !(*value) : *value;
					g_context_mode = g_menu_items[i].mode;
					saveSettings();
					return 0;
				}
			}
		} else if (g_is_cfw_config == 2) {
			if (paf_strncmp(name, "plugin_", 7) == 0) {
				u32 i = paf_strtoul(name + 7, NULL, 10);
				Plugin *plugin = (Plugin *)(g_plugins.table[i]);
				g_context_mode = PLUGINS_CONTEXT;
				plugin->active = *value;
				savePlugins();
				if (*value == PLUGIN_REMOVED) {
					sctrlKernelExitVSH(NULL);
				}
				return 0;
			}
		}
	}

	return vshSetRegistryValue(option, name, size, value);
}

int PAF_Resource_GetPageNodeByID_Patched(void *resource, char *name, SceRcoEntry **child) {
	int res = PAF_Resource_GetPageNodeByID(resource, name, child);

	if (name) {
		if (g_is_cfw_config == 1 || g_is_cfw_config == 2) {
			if (paf_strcmp(name, "page_psp_config_umd_autoboot") == 0) {
				HijackContext(*child, g_item_opts[g_context_mode].c, g_item_opts[g_context_mode].n);
			}
		}
	}

	return res;
}

int PAF_Resource_ResolveRefWString_Patched(void *resource, u32 *data, int *a2, char **string, int *t0) {
	if (data[0] == 0xDEAD) {
		utf8_to_unicode((wchar_t *)g_user_buffer, (char *)data[1]);
		*(wchar_t **)string = (wchar_t *)g_user_buffer;
		return 0;
	}

	return PAF_Resource_ResolveRefWString(resource, data, a2, string, t0);
}

int OnInitAuthPatched(void *a0, int (*handler)(), void *a2, void *a3, int (*OnInitAuth)()) {
	return OnInitAuth(a0, g_startup ? auth_handler_new : handler, a2, a3);
}

int sceVshCommonGuiBottomDialogPatched(void *a0, void *a1, void *a2, int (*cancel_handler)(), void *t0, void *t1, int (*handler)(), void *t3) {
	return sceVshCommonGuiBottomDialog(a0, a1, a2,
		g_startup ? OnRetry : (void *)cancel_handler,
		t0, t1, handler, t3);
}

SceVshItem *GetBackupVshItemPatched(u32 unk, int topitem, SceVshItem *item) {
	SceVshItem *res = GetBackupVshItem(unk, topitem, item);

	if (item->id == CUSTOM_ID_FAKE_EF_GAME) {
		item->id = g_ef_drv_id[FAKE_EF_GAME];
		return item;
	} else if (item->id == CUSTOM_ID_FAKE_EF_PHOTO) {
		item->id = g_ef_drv_id[FAKE_EF_PHOTO];
		return item;
	} else if (item->id == CUSTOM_ID_FAKE_EF_MUSIC) {
		item->id = g_ef_drv_id[FAKE_EF_MUSIC];
		return item;
	} else if (item->id == CUSTOM_ID_FAKE_EF_VIDEO) {
		item->id = g_ef_drv_id[FAKE_EF_VIDEO];
		return item;
	} else if (item->id == CUSTOM_ID_FAKE_EF_SAVEDATA) {
		item->id = g_ef_drv_id[FAKE_EF_SAVEDATA];
		return item;
	}

	return res;
}

static int RegisterDriveCallbacksPatched(u32 drives, int unk0, int unk2, int unk3) {
	logmsg4("[DEBUG]: %s: og drives=0x%08d\n", __func__, drives);

	// Forces to register the callback to check the `ef0:`
	int flags = drives;
	if ((flags & 0x8) != 0) {
		flags &= ~0x8;
	}

	int res = RegisterDriveCallbacks(flags, unk0, unk2, unk3);

	return res;
}

////////////////////////////////////////////////////////////////////////////////
// MODULE PATCHERS
////////////////////////////////////////////////////////////////////////////////

void PatchVshMain(u32 text_addr, u32 text_size) {
	int patches = 13;
	u32 scePafGetText_call = VREAD32((u32)&scePafGetText);

	g_ef_camera_item = (void *)text_addr+0x55cfc;
	g_ef_music_item = (void *)text_addr+0x55dec;
	g_ef_video_item = (void *)text_addr+0x55f2c;
	g_ef_game_item = (void *)text_addr+0x5615c;
	g_ef_savedata_item = (void *)text_addr+0x5606c;
	g_unk_strukt_addr = text_addr+0x5b84c;

	GetBackupVshItem = (void *)U_EXTRACT_CALL(text_addr+0x22598);
	MAKE_CALL(text_addr+0x22598, GetBackupVshItemPatched);

	HIJACK_FUNCTION(text_addr+0x16A70, ExecuteActionPatched, ExecuteAction);
	HIJACK_FUNCTION(text_addr+0x22648, AddVshItemPatched, AddVshItem);
	HIJACK_FUNCTION(text_addr+0x169B4, OnXmbPushPatched, OnXmbPush);
	HIJACK_FUNCTION(text_addr+0x16468, OnXmbContextMenuPatched, OnXmbContextMenu);
	HIJACK_FUNCTION(text_addr+0x16E64, UnloadModulePatched, UnloadModule);

	RegisterDriveCallbacks = (void*)text_addr+0x37f34;
	MAKE_CALL(text_addr+0x14c64, RegisterDriveCallbacksPatched);

	for (u32 addr = text_addr; addr < text_addr + text_size && patches; addr += 4) {
		u32 data = VREAD32(addr);
		if (data == 0x00063100) {
			// AddVshItem = (int (*)(void *, int,  SceVshItem *))U_EXTRACT_CALL(addr + 12);
			MAKE_CALL(addr + 12, AddVshItemPatched);
			patches--;
		} else if (data == 0x3A14000F) {
			// ExecuteAction = (void *)addr - 72;
			MAKE_CALL(addr - 72 - 36, ExecuteActionPatched);
			patches--;
		} else if (data == 0xA0C3019C) {
			// UnloadModule = (void *)addr - 52;
			patches--;
		} else if (data == 0x9042001C) {
			// OnXmbPush = (void *)addr - 124;
			patches--;
		} else if (data == 0x00021202 && OnXmbContextMenu == NULL) {
			// OnXmbContextMenu = (void *)addr - 24;
			patches--;
		} else if (data == 0x34420080 && LoadStartAuth == NULL) {
			LoadStartAuth = (void *)addr - 208;
			patches--;
		} else if (data == 0xA040014D) {
			auth_handler = (void *)addr - 32;
			patches--;
		} else if (data == 0x8E050038) {
			MAKE_CALL(addr + 4, ExecuteActionPatched);
			patches--;
		} else if (data == 0xAC520124) {
			MAKE_CALL(addr + 4, UnloadModulePatched);
			patches--;
		} else if (data == 0x24040010 && VREAD32(addr + 20) == 0x0040F809) {
			MAKE_INSTRUCTION(addr + 16, 0x8C48000C); // lw $t0, 12($v0)
			MAKE_CALL(addr + 20, OnInitAuthPatched);
			patches--;
		} else if (data == scePafGetText_call) {
			REDIRECT_FUNCTION(addr, scePafGetTextPatched);
			patches--;
		} else if (data == (u32)OnXmbPush && OnXmbPush != NULL && addr > text_addr + 0x50000) {
			VWRITE32(addr, (u32)OnXmbPushPatched);
			patches--;
		} else if (data == (u32)OnXmbContextMenu && OnXmbContextMenu != NULL && addr > text_addr + 0x50000) {
			VWRITE32(addr, (u32)OnXmbContextMenuPatched);
			patches--;
		}
	}
	sctrlFlushCache();
}

void PatchAuthPlugin(u32 text_addr, u32 text_size) {
	for (u32 addr = text_addr; addr < text_addr + text_size; addr += 4) {
		u32 data = VREAD32(addr);
		if (data == 0x27BE0040) {
			u32 a = addr - 4;
			do {
				a -= 4;
			} while (VREAD32(a) != 0x27BDFFF0);
			OnRetry = (void *)a;
		} else if (data == 0x44816000 && VREAD32(addr - 4) == 0x3C0141F0) {
			MAKE_CALL(addr + 4, sceVshCommonGuiBottomDialogPatched);
			break;
		}
	}
	sctrlFlushCache();
}

void PatchSysconfPlugin(u32 text_addr, u32 text_size) {
	u32 PAF_Resource_GetPageNodeByID_call = VREAD32((u32)&PAF_Resource_GetPageNodeByID);
	u32 PAF_Resource_ResolveRefWString_call = VREAD32((u32)&PAF_Resource_ResolveRefWString);
	u32 scePafGetText_call = VREAD32((u32)&scePafGetText);
	int patches = 10;
	for (u32 addr = text_addr; addr < text_addr + text_size && patches;
		addr += 4) {
		u32 data = VREAD32(addr);
		if (data == 0x24420008 && VREAD32(addr - 4) == 0x00402821) {
			AddSysconfItem = (void *)addr - 36;
			patches--;
		} else if (data == 0x8C840008 && VREAD32(addr + 4) == 0x27BDFFD0) {
			GetSysconfItem = (void *)addr;
			patches--;
		} else if (data == 0xAFBF0060 && VREAD32(addr + 4) == 0xAFB3005C &&
				VREAD32(addr - 12) == 0xAFB00050) {
			OnInitMenuPspConfig = (void *)addr - 20;
			patches--;
		} else if (data == 0x2C420012) {
			// Allows more than 18 items
			VWRITE16(addr, 0xFF);
			patches--;
		} else if (data == 0x01202821) {
			MAKE_CALL(addr + 8, vshGetRegistryValuePatched);
			MAKE_CALL(addr + 44, vshSetRegistryValuePatched);
			patches--;
		} else if (data == 0x2C620012 && VREAD32(addr - 4) == 0x00408821) {
			MAKE_CALL(addr - 16, GetSysconfItemPatched);
			patches--;
		} else if (data == (u32)OnInitMenuPspConfig && OnInitMenuPspConfig != NULL) {
			VWRITE32(addr, (u32)OnInitMenuPspConfigPatched);
			patches--;
		} else if (data == PAF_Resource_GetPageNodeByID_call) {
			REDIRECT_FUNCTION(addr, PAF_Resource_GetPageNodeByID_Patched);
			patches--;
		} else if (data == PAF_Resource_ResolveRefWString_call) {
			REDIRECT_FUNCTION(addr, PAF_Resource_ResolveRefWString_Patched);
			patches--;
		} else if (data == scePafGetText_call) {
			REDIRECT_FUNCTION(addr, scePafGetTextPatched);
			patches--;
		}
	}

	for (u32 addr = text_addr + 0x33000; addr < text_addr + 0x40000; addr++) {
		if (paf_strcmp((char *)addr, "fiji") == 0) {
			g_sysconf_unk = addr + 216;
			if (VREAD32(g_sysconf_unk + 4) == 0) {
				g_sysconf_unk -= 4;  // adjust on TT/DT firmware
			}
			g_sysconf_option = g_sysconf_unk + 0x4cc; // CHECK
			break;
		}
	}

	sctrlFlushCache();
}