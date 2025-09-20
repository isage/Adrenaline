#ifndef __UMD9660_DRIVER_H__
#define __UMD9660_DRIVER_H__

#define SECTOR_SIZE	0x0800

void VshCtrlSetUmdFile(const char *file);

int Umd9660ReadSectors2(int lba, int nsectors, void *buf);

#endif