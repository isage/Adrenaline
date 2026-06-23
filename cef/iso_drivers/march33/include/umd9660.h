#ifndef __UMD9660_H__
#define __UMD9660_H__

#define SECTOR_SIZE	0x0800

#define MAX_DESCRIPTORS	8

typedef struct UmdFD {
	int busy;
	int discpointer;
	u32 last_read_byte_pos;
} UmdFD;

int  InitUmd9660();

#endif // __UMD9660_H__

