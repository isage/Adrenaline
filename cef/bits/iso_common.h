/*
	Adrenaline
	Copyright (C) 2025, GrayJack
	Copyright (C) 2021, PRO CFW

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __ISO_COMMON_H
#define __ISO_COMMON_H

#include <psptypes.h>

#include <ciso.h>

#define MAX_FILES_NR 8

typedef struct IoReadArg {
	u32 offset; // 0
	u8 *address; // 4
	u32 size; // 8
} IoReadArg;

extern char g_iso_fn[256];
extern SceUID g_iso_fd;
extern int g_iso_opened;
extern int g_total_sectors;
extern IoReadArg g_read_arg;
extern char* g_sector_buffer;

int iso_read(IoReadArg *args);
int iso_open(void);
void iso_close(void);

int isoGetTitleId(char title_id[10]);

#ifdef __ISO_EXTRA__
int iso_type_check(SceUID fd);
int iso_alloc(u32 com_size);
void iso_free();
int iso_re_open(void);
#endif // __ISO_EXTRA__

#endif // __ISO_COMMON_H