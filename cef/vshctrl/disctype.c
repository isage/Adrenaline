#include <pspiofilemgr.h>
#include <psputilsforkernel.h>
#include <pspumd.h>
#include <string.h>
#include "isoreader.h"


static int has_file(char *file) {
    int ret;
    u32 size, lba;

    ret = isoGetFileInfo(file, &size, &lba);
    ret = (ret >= 0) ? 1 : 0;

    return ret;
}

int vshDetectDiscType(const char *path) {
	int result = -1;
	u32 k1 = pspSdkSetK1(0);
	int ret = isoOpen(path);

	if (ret < 0) {
		pspSdkSetK1(k1);
		return result;
	}

	result = 0;

	if(has_file("/PSP_GAME/SYSDIR/EBOOT.BIN")) {
		result |= PSP_UMD_TYPE_GAME;
	}

	if(has_file("/UMD_VIDEO/PLAYLIST.UMD")) {
		result |= PSP_UMD_TYPE_VIDEO;
	}

	if(has_file("/UMD_AUDIO/PLAYLIST.UMD")) {
		result |= PSP_UMD_TYPE_AUDIO;
	}

	if(result == 0) {
		result = -2;
	}

	isoClose();
	pspSdkSetK1(k1);

	return result;
}