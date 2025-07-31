#include <pspsdk.h>
#include <pspkernel.h>

#include <adrenaline_log.h>

#include "umd9660.h"

int IsofileGetDiscSize(int umdfd) {
	int res= 0;
	int ret = sceIoLseek(umdfd, 0, PSP_SEEK_CUR);
	int size = sceIoLseek(umdfd, 0, PSP_SEEK_END);

	sceIoLseek(umdfd, ret, PSP_SEEK_SET);

	if (size < 0) {
		res = size;
		goto out;
		// return size;
	}

	res = size / SECTOR_SIZE;
	// return size / SECTOR_SIZE;
out:
	logmsg("%s: umdfd=0x%08X -> 0x%08X\n", __func__, umdfd, res);
	return res;
}

