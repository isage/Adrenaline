/*
	Adrenaline System Control (SystemControl API)
	Copyright (C) 2016-2018, TheFloW
	Copyright (C) 2024-2025, isage
	Copyright (C) 2025, GrayJack

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

#ifndef _REBOOTCONFIG_H_
#define _REBOOTCONFIG_H_

#include <psptypes.h>

typedef struct {
	int bootfileindex;
	u8 iso_disc_type;

	char *module_after;
	void *buf;
	int size;
	int flags;

	u32 ram2;
	u32 ram11;

	char umdfilename[256];
	char title_id[10];

	// Maybe make it generic for the whole SEConfigADR
	u8 overwrite_use_psposk;
	u8 overwrite_use_psposk_to;
} RebootexConfig;

#endif // _REBOOTCONFIG_H_