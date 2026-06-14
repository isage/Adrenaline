#ifndef __UMD9660_DRIVER_H__

#define __UMD9660_DRIVER_H__

#define SECTOR_SIZE	0x800


int isoReadUmdFile(u32 offset, void *buf, u32 outsize);


#endif

