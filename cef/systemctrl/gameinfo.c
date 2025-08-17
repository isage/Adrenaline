#include <common.h>

#include <adrenaline_log.h>

#include "main.h"
#include "../../adrenaline_compat.h"

extern SceAdrenaline *adrenaline;

typedef struct LbaParams {
	int unknown1; // 0
	int cmd; // 4
	int lba_top; // 8
	int lba_size; // 12
	int byte_size_total;  // 16
	int byte_size_centre; // 20
	int byte_size_start; // 24
	int byte_size_last;  // 28
} LbaParams;

SceGameInfo default_gameinfo = {
	.game_id = "HOME0000\0\0\0\0\0\0\0\0",
};

// 0 - Not able to get | 1 - Able to get
int readGameIdFromDisc() {
	int disc_fd = sceIoOpen("disc0:/UMD_DATA.BIN", PSP_O_RDONLY, 0777);

	if (disc_fd < 0) {
		return 0;
	}

	// Country code
	sceIoRead(disc_fd, rebootex_config.game_id, 4);
	// Delimiter skip
	sceIoLseek(disc_fd, 1, PSP_SEEK_CUR);
	// Game ID
	sceIoRead(disc_fd, rebootex_config.game_id + 4, 5);

	sceIoClose(disc_fd);
	return 1;
}

// 0 - Not able to get | 1 - Able to get
int readGameIdFromPBP() {
	u32 size = sizeof(rebootex_config.game_id);
	int res = sctrlGetInitPARAM("DISC_ID", NULL, &size, rebootex_config.game_id);

	if (res < 0) {
		return 0;
	}
	return 1;
}

// 0 - Not able to get | 1 - Able to get
int readGameIdFromISO() {
	int (*isoGetGameId)(char game_id[10]) = (void*)FindProc("EPI-InfernoDriver", "inferno_driver", 0xFF8838D4);

	if (isoGetGameId == NULL) {
		// Try galaxy
		isoGetGameId = (void*)FindProc("EPI-GalaxyController", "galaxy_driver", 0xFF8838D4);

		if (isoGetGameId == NULL) {
			// FIXME: Add logic for M33 driver once we have the M33 driver code on Adrenaline
			return 0;
		}
	}

	logmsg("%s: Found `isoGetGameId`\n", __func__);

	return isoGetGameId(rebootex_config.game_id);
}

void findAndSetGameId() {
	int apitype = sceKernelInitApitype();
	if (apitype == SCE_EXEC_APITYPE_MS2 || apitype == SCE_EXEC_APITYPE_EF2 || apitype == SCE_EXEC_APITYPE_DISC || apitype == SCE_EXEC_APITYPE_DISC2 || apitype >= SCE_EXEC_APITYPE_KERNEL1){
		return;
	}

	if (rebootex_config.game_id[0] == '\0') {
		int is_iso = sceKernelFindModuleByName("EPI-InfernoDriver") != NULL || sceKernelFindModuleByName("EPI-GalaxyController") != NULL;
		if (is_iso) {
			readGameIdFromISO();
		} else {
			readGameIdFromPBP();
		}
	}

	SceGameInfo* gameinfo = sceKernelGetGameInfo();
	if (gameinfo != NULL) {
		memcpy(gameinfo->game_id, rebootex_config.game_id, 9);
	}

	if (rebootex_config.game_id[0] == '\0') {
		memcpy(default_gameinfo.game_id, rebootex_config.game_id, 9);
	}

	if (rebootex_config.game_id[0] == '\0' && adrenaline != NULL) {
		memcpy(adrenaline->titleid, rebootex_config.game_id, 9);
	}
}