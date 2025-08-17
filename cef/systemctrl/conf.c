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

#include <common.h>

#include <adrenaline_log.h>

#include "main.h"

#define ADRENALINE717_CFG_MAGIC_2 0x334F4E33

typedef struct {
	int magic[2];
	int hide_corrupt;
	int	skip_logo;
	int startup_program;
	int umd_mode;
	int	vsh_cpu_speed;
	int	app_cpu_speed;
	int fake_region;
	int skip_game_boot_logo;
	int hide_mac_addr;
	int hide_dlcs;
	int hide_pic0pic1;
	int extended_colors;
	int use_sony_psposk;
	int no_nodrm_engine;
	int no_xmb_plugins;
	int no_game_plugins;
	int no_pops_plugins;
	int force_high_memory;
	int execute_boot_bin;
	int recovery_color;
} AdrenalineConfig717;

typedef struct {
	int magic[2];
} AdrenalineMagics;

static int getMagicsFromFile(SceUID fd, AdrenalineMagics* magic) {
	int read = sceIoRead(fd, magic, sizeof(AdrenalineMagics));
	sceIoLseek(fd, 0, PSP_SEEK_SET);

	if (read < sizeof(AdrenalineMagics)) {
		read = SCE_ERR_INSIZE;
	}

	return read;
}

static void migrate_config717(AdrenalineConfig717* old, AdrenalineConfig* new){
	new->hide_corrupt = old->hide_corrupt;
	new->skip_logo = old->skip_logo;
	new->startup_program = old->startup_program;
	new->umd_mode = old->umd_mode;
	new->vsh_cpu_speed = old->vsh_cpu_speed;
	new->app_cpu_speed = old->app_cpu_speed;
	new->fake_region = old->fake_region;
	new->skip_game_boot_logo = old->skip_game_boot_logo;
	new->hide_mac_addr = old->hide_mac_addr;
	new->hide_dlcs = old->hide_dlcs;
	new->hide_pic0pic1 = old->hide_pic0pic1;
	new->extended_colors = old->extended_colors;
	new->use_sony_psposk = old->use_sony_psposk;
	new->no_nodrm_engine = old->no_nodrm_engine;
	new->no_xmb_plugins = old->no_xmb_plugins;
	new->no_game_plugins = old->no_game_plugins;
	new->no_pops_plugins = old->no_pops_plugins;
	new->force_high_memory = old->force_high_memory;
	new->execute_boot_bin = old->execute_boot_bin;
	new->recovery_color = old->recovery_color;
}


int sctrlSEGetConfigEx(AdrenalineConfig *config, int size) {
	int k1 = pspSdkSetK1(0);
	int read = SCE_ERR_ININDEX;
	int res = 0;

	memset(config, 0, size);
	SceUID fd = sceIoOpen("flash1:/config.adrenaline", PSP_O_RDONLY, 0);
	if (fd < 0) {
		res = SCE_ENOENT;
		goto exit;
	}

	int file_size = sceIoLseek32(fd, 0, PSP_SEEK_END);
	sceIoLseek(fd, 0, PSP_SEEK_SET);

	// Needs migration
	if (file_size != sizeof(AdrenalineConfig)) {
		AdrenalineMagics magics = {0};
		getMagicsFromFile(fd, &magics);

		switch (magics.magic[1]) {
			case ADRENALINE717_CFG_MAGIC_2:
				logmsg("[INFO]: Migrated Adrenaline CFW 7.1.7 config\n");
				AdrenalineConfig717 old_conf;
				read = sceIoRead(fd, &old_conf, sizeof(AdrenalineConfig717));
				migrate_config717(&old_conf, config);

				if (read < sizeof(AdrenalineConfig717)) {
					res = SCE_EIO;
					goto exit;
				}

				// Close fd and also set the new config
				sceIoClose(fd);
				sctrlSESetConfig(config);
				logmsg("[INFO]: Migrated Adrenaline CFW 7.1.7 config\n");
				break;
			default:
				read = sceIoRead(fd, config, size);

				if (read < size) {
					res = SCE_EIO;
					goto exit;
				}
				break;
		}
	} else {
		read = sceIoRead(fd, config, size);

		if (read < size) {
			res = SCE_EIO;
			goto exit;
		}
	}

exit:
	if (fd >= 0) {
		sceIoClose(fd);
	}
	pspSdkSetK1(k1);
	return res;
}

int sctrlSESetConfigEx(AdrenalineConfig *config, int size) {
	int k1 = pspSdkSetK1(0);

	SceUID fd = sceIoOpen("flash1:/config.adrenaline", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	if (fd < 0) {
		pspSdkSetK1(k1);
		return -1;
	}

	config->magic[0] = ADRENALINE_CFG_MAGIC_1;
	config->magic[1] = ADRENALINE_CFG_MAGIC_2;

	if (sceIoWrite(fd, config, size) < size) {
		sceIoClose(fd);
		pspSdkSetK1(k1);
		return -1;
	}

	sceIoClose(fd);
	pspSdkSetK1(k1);

	return 0;
}

int sctrlSEGetConfig(AdrenalineConfig *config) {
	return sctrlSEGetConfigEx(config, sizeof(AdrenalineConfig));
}

int sctrlSESetConfig(AdrenalineConfig *config) {
	return sctrlSESetConfigEx(config, sizeof(AdrenalineConfig));
}

int sctrlSEApplyConfigEX(AdrenalineConfig *conf, int size) {
	if (size == sizeof(AdrenalineConfig)){
		memcpy(&config, conf, size);
		return 0;
	}
	return -1;
}

void sctrlSEApplyConfig(AdrenalineConfig *conf) {
	memcpy(&config, conf, sizeof(AdrenalineConfig));
}