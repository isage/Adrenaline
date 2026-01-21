#ifndef _PGD_H
#define _PGD_H

#include <pspkernel.h>

typedef struct {
	u8  vkey[16];

	int open_flag;
	int key_index;
	int drm_type;
	int mac_type;
	int cipher_type;

	int data_size;
	int align_size;
	int block_size;
	int block_nr;
	int data_offset;
	int table_offset;

	u8 *buf;
} PGD_DESC;

int kirk7(u8 *buf, int size, int type);

int get_version_key(u8 *version_key, char *path);
int get_edat_key(u8 *vkey, u8 *pgd_buf);
int dumpPS1key(const char *path, u8 *buf);

#endif /* _PGD_H */
