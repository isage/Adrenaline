#include <pspsdk.h>
#include <pspkernel.h>

#include "umd9660.h"

int __inline IsofileGetDiscSize(int umdfd)
{
	int ret = sceIoLseek(umdfd, 0, PSP_SEEK_CUR);
	int size = sceIoLseek(umdfd, 0, PSP_SEEK_END);

	sceIoLseek(umdfd, ret, PSP_SEEK_SET);

	if (size < 0)
		return size;

	return size / SECTOR_SIZE;
}

