/*
	Adrenaline
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

#ifndef _PATCH_IO_H
#define _PATCH_IO_H

#include <pspkerneltypes.h>
#include <pspiofilemgr_dirent.h>
#include <pspiofilemgr_stat.h>

int GetIsoIndex(const char *file);

SceUID sceIoDopenPatched(const char *dirname);
int sceIoDreadPatched(SceUID fd, SceIoDirent *dir);
int sceIoDclosePatched(SceUID fd);

SceUID sceIoOpenPatched(const char *file, int flags, SceMode mode);
int sceIoClosePatched(SceUID fd);
int sceIoReadPatched(SceUID fd, void *data, SceSize size);
SceOff sceIoLseekPatched(SceUID fd, SceOff offset, int whence);
int sceIoLseek32Patched(SceUID fd, int offset, int whence);
int sceIoGetstatPatched(const char *file, SceIoStat *stat);
int sceIoChstatPatched(const char *file, SceIoStat *stat, int bits);
int sceIoRemovePatched(const char *file);
int sceIoRmdirPatched(const char *path);
int sceIoMkdirPatched(const char *dir, SceMode mode);

void IoPatches();

#endif