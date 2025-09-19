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
#include <common.h>
#include <vshctrl.h>
#include <psputility_sysparam.h>

#include "main.h"
#include "utils.h"

#include "list.h"
#include "plugins.h"
#include "settings.h"

static int sysconf_action = 0;
static u32 sysconf_unk = 0;
static u32 sysconf_option = 0;
static int startup = 1;

static int is_cfw_config = 0;
static int unload = 0;
static int context_mode = 0;
static u32 backup[4];

static SceVshItem *new_item = NULL;
static SceVshItem *new_item2 = NULL;
static void *xmb_arg0 = NULL;
static void *xmb_arg1 = NULL;

static char user_buffer[LINE_BUFFER_SIZE];
static char buf[64];

static char *need_reboot_subtitle = "Requires restarting VSH to take effect";

static unsigned char console_item[] __attribute__((aligned(16))) = {
	0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
	0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x42, 0x58, 0x00, 0x00, 0x43,
	0x4C, 0x00, 0x00, 0x43, 0x5A, 0x00, 0x00, 0x6D, 0x73, 0x67, 0x74, 0x6F,
	0x70, 0x5F, 0x73, 0x79, 0x73, 0x63, 0x6F, 0x6E, 0x66, 0x5F, 0x63, 0x6F,
	0x6E, 0x73, 0x6F, 0x6C, 0x65, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static unsigned char usb_item[] __attribute__((aligned(16))) = {
	0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
	0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x00, 0x00, 0x42, 0x55, 0x00, 0x00, 0x43,
	0x49, 0x00, 0x00, 0x43, 0x57, 0x00, 0x00, 0x6D, 0x73, 0x67, 0x74, 0x6F,
	0x70, 0x5F, 0x73, 0x79, 0x73, 0x63, 0x6F, 0x6E, 0x66, 0x5F, 0x75, 0x73,
	0x62, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

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

static void (*LoadStartAuth)() = NULL;
static int (*auth_handler)(int a0) = NULL;
static void (*OnRetry)() = NULL;

static void (*AddSysconfItem)(u32 *option, SceSysconfItem **item) = NULL;
static void (*OnInitMenuPspConfig)() = NULL;

static SceOff findPkgOffset(const char *filename, unsigned *size, const char *pkgpath) {
	int pkg = sceIoOpen(pkgpath, PSP_O_RDONLY, 0777);
	if (pkg < 0)
		return 0;

	unsigned pkgsize = sceIoLseek32(pkg, 0, PSP_SEEK_END);
	unsigned size2 = 0;

	sceIoLseek32(pkg, 0, PSP_SEEK_SET);

	if (size != NULL)
		*size = 0;

	unsigned offset = 0;
	char name[64];

	while (offset != 0xFFFFFFFF) {
		sceIoRead(pkg, &offset, 4);
		if (offset == 0xFFFFFFFF) {
			sceIoClose(pkg);
			return 0;
		}
		unsigned namelength;
		sceIoRead(pkg, &namelength, 4);
		sceIoRead(pkg, name, namelength + 1);

		if (!strncmp(name, filename, namelength)) {
			sceIoRead(pkg, &size2, 4);

			if (size2 == 0xFFFFFFFF)
				size2 = pkgsize;

			if (size != NULL)
				*size = size2 - offset;

			sceIoClose(pkg);
			return offset;
		}
	}
	return 0;
}

static void *addCustomVshItem(int id, char *text, int action_arg, SceVshItem *orig) {
	SceVshItem *item = (SceVshItem *)sce_paf_private_malloc(sizeof(SceVshItem));
	sce_paf_private_memcpy(item, orig, sizeof(SceVshItem));

	item->id = id; // custom id
	item->action = sysconf_action;
	item->action_arg = action_arg;
	item->play_sound = 1;
	item->context = NULL;
	sce_paf_private_strcpy(item->text, text);

	return item;
}

static void AddSysconfContextItem(char *text, char *subtitle, char *regkey) {
	SceSysconfItem *item = (SceSysconfItem *)sce_paf_private_malloc(sizeof(SceSysconfItem));

	item->id = 5;
	item->unk = (u32 *)sysconf_unk;
	item->regkey = regkey;
	item->text = text;
	item->subtitle = subtitle;
	item->page = "page_psp_config_umd_autoboot";

	((u32 *)sysconf_option)[2] = 1;

	AddSysconfItem((u32 *)sysconf_option, &item);
}

static void HijackContext(SceRcoEntry *src, char **options, int n) {
	SceRcoEntry *plane = (SceRcoEntry *)((u32)src + src->first_child);
	SceRcoEntry *mlist = (SceRcoEntry *)((u32)plane + plane->first_child);
	u32 *mlist_param = (u32 *)((u32)mlist + mlist->param);

	/* Backup */
	if (backup[0] == 0 && backup[1] == 0 && backup[2] == 0 && backup[3] == 0) {
		backup[0] = mlist->first_child;
		backup[1] = mlist->child_count;
		backup[2] = mlist_param[16];
		backup[3] = mlist_param[18];
	}

	if (context_mode) {
		SceRcoEntry *base = (SceRcoEntry *)((u32)mlist + mlist->first_child);

		SceRcoEntry *item =
			(SceRcoEntry *)sce_paf_private_malloc(base->next_entry * n);
		u32 *item_param = (u32 *)((u32)item + base->param);

		mlist->first_child = (u32)item - (u32)mlist;
		mlist->child_count = n;
		mlist_param[16] = 13;
		mlist_param[18] = 6;

		for (int i = 0; i < n; i++) {
			sce_paf_private_memcpy(item, base, base->next_entry);

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
		mlist->first_child = backup[0];
		mlist->child_count = backup[1];
		mlist_param[16] = backup[2];
		mlist_param[18] = backup[3];
	}

	sceKernelDcacheWritebackAll();
}

static int auth_handler_new(int a0) {
	startup = a0;
	return auth_handler(a0);
}

////////////////////////////////////////////////////////////////////////////////
// PATCHED IMPLEMENTATIONS
////////////////////////////////////////////////////////////////////////////////

int AddVshItemPatched(void *a0, int topitem, SceVshItem *item) {
	static int cfw_conf_added = 0;
	static int plugin_mgr_added = 0;

	if (sce_paf_private_strcmp(item->text, "msgtop_sysconf_console") == 0) {
		sysconf_action = item->action;

		// Plugin Manager is added before the `msgtop_sysconf_console`
		// So apply re-patch the action for it
		new_item->action = item->action;
	}

	// prevent adding more than once
	// Add the item just after "msgtop_sysconf_usb" (that is also just before "msgtop_sysconf_video")
	if (!plugin_mgr_added && sce_paf_private_strcmp(item->text, "msgtop_sysconf_video") == 0) {
		plugin_mgr_added = 1;
		startup = 0;

		int cur_icon = 0;

		if (psp_model == PSP_11000) {
			u32 value = 0;
			vctrlGetRegistryValue("/CONFIG/SYSTEM/XMB/THEME", "custom_theme_mode", &value);
			cur_icon = !value;
		}

		// Add Plugins Manager
		new_item = addCustomVshItem(91, "msgtop_sysconf_pluginsmgr", sysconf_plugins_action_arg, (cur_icon) ? item : (SceVshItem*)usb_item);
		AddVshItem(a0, topitem, new_item);
	}

	// prevent adding more than once
	// Add the item just after "msgtop_sysconf_console" (that is also just before "msgtop_sysconf_theme")
	if (!cfw_conf_added && sce_paf_private_strcmp(item->text, "msgtop_sysconf_theme") == 0) {
		cfw_conf_added = 1;
		startup = 0;

		int cur_icon = 0;

		if (psp_model == PSP_11000) {
			u32 value = 0;
			vctrlGetRegistryValue("/CONFIG/SYSTEM/XMB/THEME", "custom_theme_mode", &value);
			cur_icon = !value;
		}

		// Add CFW Settings
		new_item2 = addCustomVshItem(92, "msgtop_sysconf_cfwconfig", sysconf_cfwconfig_action_arg, (cur_icon) ? item : (SceVshItem*)console_item);
		AddVshItem(a0, topitem, new_item2);
	}

	return AddVshItem(a0, topitem, item);
}

int OnXmbPushPatched(void *arg0, void *arg1) {
	xmb_arg0 = arg0;
	xmb_arg1 = arg1;
	return OnXmbPush(arg0, arg1);
}

int OnXmbContextMenuPatched(void *arg0, void *arg1) {
	new_item->context = NULL;
	new_item2->context = NULL;
	return OnXmbContextMenu(arg0, arg1);
}

int ExecuteActionPatched(int action, int action_arg) {
	int old_is_cfw_config = is_cfw_config;

	if (action == sysconf_console_action) {
		if (action_arg == sysconf_cfwconfig_action_arg) {
			is_cfw_config = 1;
			action = sysconf_console_action;
			action_arg = sysconf_console_action_arg;
		} else if (action_arg == sysconf_plugins_action_arg) {
			is_cfw_config = 2;
			action = sysconf_console_action;
			action_arg = sysconf_console_action_arg;
		} else {
			is_cfw_config = 0;
		}
	}
	if (old_is_cfw_config != is_cfw_config) {
		sce_paf_private_memset(backup, 0, sizeof(backup));
		context_mode = 0;

		unload = 1;
	}

	return ExecuteAction(action, action_arg);
}

int UnloadModulePatched(int skip) {
	if (unload) {
		skip = -1;
		unload = 0;
	}
	return UnloadModule(skip);
}

void OnInitMenuPspConfigPatched() {
	if (is_cfw_config == 1) {
		if (((u32 *)sysconf_option)[2] == 0) {
			loadSettings();
			for (int i = 0; i < num_items; i++) {
				if (GetItemes[i].need_reboot == 1) {
					AddSysconfContextItem(GetItemes[i].item, "cfw_need_restart", GetItemes[i].item);
				} else {
					AddSysconfContextItem(GetItemes[i].item, NULL, GetItemes[i].item);
				}
			}
		}
	} else if (is_cfw_config == 2) {
		if (((u32 *)sysconf_option)[2] == 0) {
			loadPlugins();
			for (int i = 0; i < plugins.count; i++) {
				Plugin *plugin = (Plugin *)(plugins.table[i]);
				if (plugin->name != NULL) {
					AddSysconfContextItem(plugin->name, plugin->surname, plugin->name);
				}
			}
		}
	} else {
		OnInitMenuPspConfig();
	}
}

SceSysconfItem *GetSysconfItemPatched(void *a0, void *a1) {
	SceSysconfItem *item = GetSysconfItem(a0, a1);

	if (is_cfw_config == 1) {
		for (int i = 0; i < num_items; i++) {
			if (sce_paf_private_strcmp(item->text, GetItemes[i].item) == 0) {
				context_mode = GetItemes[i].mode;
			}
		}
	} else if (is_cfw_config == 2) {
		context_mode = PLUGINS_CONTEXT;
	}
	return item;
}

wchar_t *scePafGetTextPatched(void *a0, char *name) {
	if (name) {
		if (is_cfw_config == 1) {
			if (sce_paf_private_strncmp(name, "cfw_need_restart", 17) == 0) {
				utf8_to_unicode((wchar_t *)user_buffer, need_reboot_subtitle);
				return (wchar_t *)user_buffer;
			}
			for (int i = 0; i < num_items; i++) {
				if (sce_paf_private_strcmp(name, GetItemes[i].item) == 0) {
					if (GetItemes[i].advanced) {
						static char buff[128];
						sce_paf_private_sprintf(buff, "%s %s", ADVANCED, GetItemes[i].item);
						utf8_to_unicode((wchar_t *)user_buffer, buff);
					}else {
						utf8_to_unicode((wchar_t *)user_buffer, GetItemes[i].item);
					}
					return (wchar_t *)user_buffer;
				}
			}
		} else if (is_cfw_config == 2) {
			if (sce_paf_private_strncmp(name, "plugin_", 7) == 0) {
				u32 i = sce_paf_private_strtoul(name + 7, NULL, 10);
				Plugin *plugin = (Plugin *)(plugins.table[i]);
				static char file[128];
				sce_paf_private_strcpy(file, plugin->path);

				char *p = sce_paf_private_strrchr(plugin->path, '/');
				char buf[64] = {0};
				if (p) {
					char *p2 = sce_paf_private_strchr(p + 1, '.');
					if (p2) {
						int len = (int)(p2 - (p + 1));
						sce_paf_private_strncpy(file, p + 1, len);

						snprintf(buf, 63, " [%s]", plugin->runlevel);
						int rl_len = sce_paf_private_strlen(buf);

						sce_paf_private_strncpy(file + len, buf, rl_len);
						file[len+rl_len] = '\0';
					}
				} else {
					char *p2 = sce_paf_private_strchr(file, '.');
					if (p2) {
						int len = (int)(p2 - (file));

						snprintf(buf, 63, " [%s]", plugin->runlevel);
						int rl_len = sce_paf_private_strlen(buf);

						sce_paf_private_strncpy(file + len, buf, rl_len);
						file[len+rl_len] = '\0';
					}
				}

				utf8_to_unicode((wchar_t *)user_buffer, file);
				return (wchar_t *)user_buffer;
			} else if (sce_paf_private_strncmp(name, "plugins", 7) == 0) {
				u32 i = sce_paf_private_strtoul(name + 7, NULL, 10);
				Plugin *plugin = (Plugin *)(plugins.table[i]);
				utf8_to_unicode((wchar_t *)user_buffer, plugin->path);
				return (wchar_t *)user_buffer;
			}
		}

		if (sce_paf_private_strcmp(name, "msgtop_sysconf_cfwconfig") == 0) {
			sce_paf_private_sprintf(buf, "%s Adrenaline CFW Settings", STAR);
			utf8_to_unicode((wchar_t *)user_buffer, buf);
			return (wchar_t *)user_buffer;
		} else if (sce_paf_private_strcmp(name, "msgtop_sysconf_pluginsmgr") == 0) {
			sce_paf_private_sprintf(buf, "%s Plugins Manager", STAR);
			utf8_to_unicode((wchar_t *)user_buffer, buf);
			return (wchar_t *)user_buffer;
		}
	}

	wchar_t *res = scePafGetText(a0, name);

	return res;
}

int vshGetRegistryValuePatched(u32 *option, char *name, void *arg2, int size,int *value) {
	if (name) {
		if (is_cfw_config == 1) {
			for (int i = 0; i < num_items; i++) {
				if (sce_paf_private_strcmp(name, GetItemes[i].item) == 0) {
					context_mode = GetItemes[i].mode;
					*value = (GetItemes[i].negative) ? !(*GetItemes[i].value) : *GetItemes[i].value;
					return 0;
				}
			}
		} else if (is_cfw_config == 2) {
			if (sce_paf_private_strncmp(name, "plugin_", 7) == 0) {
				u32 i = sce_paf_private_strtoul(name + 7, NULL, 10);
				Plugin *plugin = (Plugin *)(plugins.table[i]);
				context_mode = PLUGINS_CONTEXT;
				*value = plugin->active;
				return 0;
			}
		}
	}

	int res = vshGetRegistryValue(option, name, arg2, size, value);

	return res;
}

int vshSetRegistryValuePatched(u32 *option, char *name, int size, int *value) {
	if (name) {
		if (is_cfw_config == 1) {
			for (int i = 0; i < num_items; i++) {
				if (sce_paf_private_strcmp(name, GetItemes[i].item) == 0) {
					*GetItemes[i].value = GetItemes[i].negative ? !(*value) : *value;
					context_mode = GetItemes[i].mode;
					saveSettings();
					return 0;
				}
			}
		} else if (is_cfw_config == 2) {
			if (sce_paf_private_strncmp(name, "plugin_", 7) == 0) {
				u32 i = sce_paf_private_strtoul(name + 7, NULL, 10);
				Plugin *plugin = (Plugin *)(plugins.table[i]);
				context_mode = PLUGINS_CONTEXT;
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
		if (is_cfw_config == 1 || is_cfw_config == 2) {
			if (sce_paf_private_strcmp(name, "page_psp_config_umd_autoboot") == 0) {
				HijackContext(*child, item_opts[context_mode].c, item_opts[context_mode].n);
			}
		}
	}

	return res;
}

int PAF_Resource_ResolveRefWString_Patched(void *resource, u32 *data, int *a2, char **string, int *t0) {
	if (data[0] == 0xDEAD) {
		utf8_to_unicode((wchar_t *)user_buffer, (char *)data[1]);
		*(wchar_t **)string = (wchar_t *)user_buffer;
		return 0;
	}

	return PAF_Resource_ResolveRefWString(resource, data, a2, string, t0);
}

int OnInitAuthPatched(void *a0, int (*handler)(), void *a2, void *a3, int (*OnInitAuth)()) {
	return OnInitAuth(a0, startup ? auth_handler_new : handler, a2, a3);
}

int sceVshCommonGuiBottomDialogPatched(void *a0, void *a1, void *a2, int (*cancel_handler)(), void *t0, void *t1, int (*handler)(), void *t3) {
	return sceVshCommonGuiBottomDialog(a0, a1, a2,
		startup ? OnRetry : (void *)cancel_handler,
		t0, t1, handler, t3);
}

////////////////////////////////////////////////////////////////////////////////
// MODULE PATCHERS
////////////////////////////////////////////////////////////////////////////////

void PatchVshMain(u32 text_addr, u32 text_size) {
	int patches = 13;
	u32 scePafGetText_call = VREAD32((u32)&scePafGetText);

	for (u32 addr = text_addr; addr < text_addr + text_size && patches; addr += 4) {
		u32 data = VREAD32(addr);
		if (data == 0x00063100) {
			AddVshItem = (int (*)(void *, int,  SceVshItem *))U_EXTRACT_CALL(addr + 12);
			MAKE_CALL(addr + 12, AddVshItemPatched);
			patches--;
		} else if (data == 0x3A14000F) {
			ExecuteAction = (void *)addr - 72;
			MAKE_CALL(addr - 72 - 36, ExecuteActionPatched);
			patches--;
		} else if (data == 0xA0C3019C) {
			UnloadModule = (void *)addr - 52;
			patches--;
		} else if (data == 0x9042001C) {
			OnXmbPush = (void *)addr - 124;
			patches--;
		} else if (data == 0x00021202 && OnXmbContextMenu == NULL) {
			OnXmbContextMenu = (void *)addr - 24;
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
		if (strcmp((char *)addr, "fiji") == 0) {
		sysconf_unk = addr + 216;
		if (VREAD32(sysconf_unk + 4) == 0)
			sysconf_unk -= 4;                   // adjust on TT/DT firmware
		sysconf_option = sysconf_unk + 0x4cc; // CHECK
		break;
		}
	}

	sctrlFlushCache();
}