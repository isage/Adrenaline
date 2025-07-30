#ifndef __UMD9660_H__

#define __UMD9660_H__

#define SECTOR_SIZE	0x0800

#define MAX_DESCRIPTORS	8

typedef struct UmdFD
{
	int busy;
	int discpointer;
} UmdFD;

#define GetUmdFile sctrlSEGetUmdFile
void SetUmdFile(char *file);
int  OpenIso();
int  ReadUmdFileRetry(void *buf, int size, int fpointer);
int  Umd9660ReadSectors(int lba, int nsectors, void *buf);
int  InitUmd9660();



#endif

