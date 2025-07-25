/*
	Adrenaline
	Copyright (C) 2025, isage
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

#ifndef __USBCAM_PATCH_H__
#define __USBCAM_PATCH_H__

#include <psptypes.h>
#include <psploadcore.h>


typedef struct PspUsbCamSetupMicParam {
	int size;
	int alc;
	int gain;
	int noize;
	int freq;
} PspUsbCamSetupMicParam;

typedef struct PspUsbCamSetupMicExParam {
	int size;
	int alc;
	int gain;
	u32 unk2[4]; // noize/hold/decay/attack?
	int freq;
	int unk3;
} PspUsbCamSetupMicExParam;

void PatchUSBCamDriver(SceModule2* mod);

#endif