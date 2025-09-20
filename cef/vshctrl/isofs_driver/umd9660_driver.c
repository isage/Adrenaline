#include <common.h>
#include <psperror.h>

#include <iso_common.h>

#include "umd9660_driver.h"

void VshCtrlSetUmdFile(const char *file) {
	SetUmdFile(file);
	iso_close();
	strncpy(g_iso_fn, file, sizeof(g_iso_fn)-1);
}

int Umd9660ReadSectors2(int lba, int nsectors, void *buf) {
	if (g_iso_opened == 0) {
		for (int i = 0; i < 0x10; i++) {
			if (sceIoLseek32(g_iso_fd, 0, PSP_SEEK_CUR) >= 0) {
				break;
			}

			iso_open();
		}

		if (g_iso_opened == 0) {
			return SCE_ENODEV;
		}
	}

	IoReadArg read_arg = {
		.offset = lba * SECTOR_SIZE,
		.address = buf,
		.size = SECTOR_SIZE * nsectors
	};

	int read = iso_read(&read_arg);
	return read/SECTOR_SIZE;
}