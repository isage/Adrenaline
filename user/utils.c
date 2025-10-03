/*
	Adrenaline
	Copyright (C) 2016-2018, TheFloW

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

#include <psp2/ctrl.h>
#include <psp2/display.h>
#include <psp2/rtc.h>
#include <psp2/system_param.h>
#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/processmgr.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <malloc.h>

#include "main.h"
#include "utils.h"
#include "msfs.h"

Pad old_pad, current_pad, pressed_pad, released_pad, hold_pad, hold2_pad;
Pad hold_count, hold2_count;

int SCE_CTRL_ENTER = SCE_CTRL_CROSS, SCE_CTRL_CANCEL = SCE_CTRL_CIRCLE;

int debugPrintf(char *text, ...) {
	va_list list;
	char string[512];

	va_start(list, text);
	vsprintf(string, text, list);
	va_end(list);

	SceUID fd = sceIoOpen("ux0:data/adrenaline_user_log.txt", SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 0777);
	if (fd >= 0) {
		sceIoWrite(fd, string, strlen(string));
		sceIoClose(fd);
	}

	return 0;
}

int ReadFile(char *file, void *buf, int size) {
	SceUID fd = sceIoOpen(file, SCE_O_RDONLY, 0);
	if (fd < 0) {
		return fd;
	}

	int read = sceIoRead(fd, buf, size);

	sceIoClose(fd);
	return read;
}

int WriteFile(char *file, void *buf, int size) {
	SceUID fd = sceIoOpen(file, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
	if (fd < 0) {
		return fd;
	}

	int written = sceIoWrite(fd, buf, size);

	sceIoClose(fd);
	return written;
}

extern int enter_button;
void readPad() {
	SceCtrlData pad;
	sceCtrlPeekBufferPositive(0, &pad, 1);

	SceCtrlData home;
	kuCtrlPeekBufferPositive(0, &home, 1);

	pad.buttons &= ~SCE_CTRL_PSBUTTON;
	pad.buttons |= (home.buttons & SCE_CTRL_PSBUTTON);

	memcpy(&old_pad, current_pad, sizeof(Pad));
	memset(&current_pad, 0, sizeof(Pad));

	if (pad.buttons & SCE_CTRL_UP)
		current_pad[PAD_UP] = 1;
	if (pad.buttons & SCE_CTRL_DOWN)
		current_pad[PAD_DOWN] = 1;
	if (pad.buttons & SCE_CTRL_LEFT)
		current_pad[PAD_LEFT] = 1;
	if (pad.buttons & SCE_CTRL_RIGHT)
		current_pad[PAD_RIGHT] = 1;
	if (pad.buttons & SCE_CTRL_LTRIGGER)
		current_pad[PAD_LTRIGGER] = 1;
	if (pad.buttons & SCE_CTRL_RTRIGGER)
		current_pad[PAD_RTRIGGER] = 1;
	if (pad.buttons & SCE_CTRL_TRIANGLE)
		current_pad[PAD_TRIANGLE] = 1;
	if (pad.buttons & SCE_CTRL_CIRCLE)
		current_pad[PAD_CIRCLE] = 1;
	if (pad.buttons & SCE_CTRL_CROSS)
		current_pad[PAD_CROSS] = 1;
	if (pad.buttons & SCE_CTRL_SQUARE)
		current_pad[PAD_SQUARE] = 1;
	if (pad.buttons & SCE_CTRL_START)
		current_pad[PAD_START] = 1;
	if (pad.buttons & SCE_CTRL_SELECT)
		current_pad[PAD_SELECT] = 1;
	if (pad.buttons & SCE_CTRL_PSBUTTON)
		current_pad[PAD_PSBUTTON] = 1;

	if (pad.ly < ANALOG_CENTER - ANALOG_THRESHOLD) {
		current_pad[PAD_LEFT_ANALOG_UP] = 1;
	} else if (pad.ly > ANALOG_CENTER + ANALOG_THRESHOLD) {
		current_pad[PAD_LEFT_ANALOG_DOWN] = 1;
	}

	if (pad.lx < ANALOG_CENTER - ANALOG_THRESHOLD) {
		current_pad[PAD_LEFT_ANALOG_LEFT] = 1;
	} else if (pad.lx > ANALOG_CENTER + ANALOG_THRESHOLD) {
		current_pad[PAD_LEFT_ANALOG_RIGHT] = 1;
	}

	if (pad.ry < ANALOG_CENTER - ANALOG_THRESHOLD) {
		current_pad[PAD_RIGHT_ANALOG_UP] = 1;
	} else if (pad.ry > ANALOG_CENTER + ANALOG_THRESHOLD) {
		current_pad[PAD_RIGHT_ANALOG_DOWN] = 1;
	}

	if (pad.rx < ANALOG_CENTER - ANALOG_THRESHOLD) {
		current_pad[PAD_RIGHT_ANALOG_LEFT] = 1;
	} else if (pad.rx > ANALOG_CENTER + ANALOG_THRESHOLD) {
		current_pad[PAD_RIGHT_ANALOG_RIGHT] = 1;
	}

	for (int i = 0; i < PAD_N_BUTTONS; i++) {
		pressed_pad[i] = current_pad[i] & ~old_pad[i];
		released_pad[i] = ~current_pad[i] & old_pad[i];

		hold_pad[i] = pressed_pad[i];
		hold2_pad[i] = pressed_pad[i];

		if (current_pad[i]) {
			if (hold_count[i] >= 10) {
				hold_pad[i] = 1;
				hold_count[i] = 6;
			}

			if (hold2_count[i] >= 10) {
				hold2_pad[i] = 1;
				hold2_count[i] = 10;
			}

			hold_count[i]++;
			hold2_count[i]++;
		} else {
			hold_count[i] = 0;
			hold2_count[i] = 0;
		}
	}

	if (enter_button == SCE_SYSTEM_PARAM_ENTER_BUTTON_CIRCLE) {
		old_pad[PAD_ENTER] = old_pad[PAD_CIRCLE];
		current_pad[PAD_ENTER] = current_pad[PAD_CIRCLE];
		pressed_pad[PAD_ENTER] = pressed_pad[PAD_CIRCLE];
		released_pad[PAD_ENTER] = released_pad[PAD_CIRCLE];
		hold_pad[PAD_ENTER] = hold_pad[PAD_CIRCLE];
		hold2_pad[PAD_ENTER] = hold2_pad[PAD_CIRCLE];

		old_pad[PAD_CANCEL] = old_pad[PAD_CROSS];
		current_pad[PAD_CANCEL] = current_pad[PAD_CROSS];
		pressed_pad[PAD_CANCEL] = pressed_pad[PAD_CROSS];
		released_pad[PAD_CANCEL] = released_pad[PAD_CROSS];
		hold_pad[PAD_CANCEL] = hold_pad[PAD_CROSS];
		hold2_pad[PAD_CANCEL] = hold2_pad[PAD_CROSS];
	} else {
		old_pad[PAD_ENTER] = old_pad[PAD_CROSS];
		current_pad[PAD_ENTER] = current_pad[PAD_CROSS];
		pressed_pad[PAD_ENTER] = pressed_pad[PAD_CROSS];
		released_pad[PAD_ENTER] = released_pad[PAD_CROSS];
		hold_pad[PAD_ENTER] = hold_pad[PAD_CROSS];
		hold2_pad[PAD_ENTER] = hold2_pad[PAD_CROSS];

		old_pad[PAD_CANCEL] = old_pad[PAD_CIRCLE];
		current_pad[PAD_CANCEL] = current_pad[PAD_CIRCLE];
		pressed_pad[PAD_CANCEL] = pressed_pad[PAD_CIRCLE];
		released_pad[PAD_CANCEL] = released_pad[PAD_CIRCLE];
		hold_pad[PAD_CANCEL] = hold_pad[PAD_CIRCLE];
		hold2_pad[PAD_CANCEL] = hold2_pad[PAD_CIRCLE];
	}
}

int doubleClick(uint32_t buttons, uint64_t max_time) {
	static uint32_t old_buttons, current_buttons, released_buttons;
	static uint64_t last_time = 0;
	static int clicked = 0;
	int double_clicked = 0;

	SceCtrlData pad;
	kuCtrlPeekBufferPositive(0, &pad, 1);

	old_buttons = current_buttons;
	current_buttons = pad.buttons;
	released_buttons = ~current_buttons & old_buttons;

	if (released_buttons & buttons) {
		if (clicked) {
			if ((sceKernelGetProcessTimeWide() - last_time) < max_time) {
				double_clicked = 1;
				clicked = 0;
				last_time = 0;
			} else {
				clicked = 1;
				last_time = sceKernelGetProcessTimeWide();
			}
		} else {
			clicked = 1;
			last_time = sceKernelGetProcessTimeWide();
		}
	}

	return double_clicked;
}

void getSizeString(char string[16], uint64_t size) {
	int i = 0;
	static char *units[] = { "B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB" };
	while (size > 1024) {
		size >>= 10;
		i++;
	}

	snprintf(string, 16, "%.*lld %s", (i == 0) ? 0 : 2, size, units[i]);
}

void convertUtcToLocalTime(SceDateTime *time_local, SceDateTime *time_utc) {
	SceRtcTick tick;
	sceRtcGetTick(time_utc, &tick);
	sceRtcConvertUtcToLocalTime(&tick, &tick);
	sceRtcSetTick(time_local, &tick);
}

void getTimeString(char string[16], int time_format, SceDateTime *time) {
	SceDateTime time_local;
	convertUtcToLocalTime(&time_local, time);

	switch(time_format) {
		case SCE_SYSTEM_PARAM_TIME_FORMAT_12HR:
			snprintf(string, 16, "%02d:%02d %s", (time_local.hour > 12) ? (time_local.hour-12) : ((time_local.hour == 0) ? 12 : time_local.hour),
				time_local.minute, time_local.hour >= 12 ? "PM" : "AM");
			break;

		case SCE_SYSTEM_PARAM_TIME_FORMAT_24HR:
			snprintf(string, 16, "%02d:%02d", time_local.hour, time_local.minute);
			break;
	}
}

void getDateString(char string[24], int date_format, SceDateTime *time) {
	SceDateTime time_local;
	convertUtcToLocalTime(&time_local, time);

	switch (date_format) {
		case SCE_SYSTEM_PARAM_DATE_FORMAT_YYYYMMDD:
			snprintf(string, 24, "%04d/%02d/%02d", time_local.year, time_local.month, time_local.day);
			break;

		case SCE_SYSTEM_PARAM_DATE_FORMAT_DDMMYYYY:
			snprintf(string, 24, "%02d/%02d/%04d", time_local.day, time_local.month, time_local.year);
			break;

		case SCE_SYSTEM_PARAM_DATE_FORMAT_MMDDYYYY:
			snprintf(string, 24, "%02d/%02d/%04d", time_local.month, time_local.day, time_local.year);
			break;
	}
}

void SetPspemuFrameBuffer(void *base) {
	SceDisplayFrameBuf framebuf;
	memset(&framebuf, 0, sizeof(SceDisplayFrameBuf));
	framebuf.size        = sizeof(SceDisplayFrameBuf);
	framebuf.base        = base;
	framebuf.pitch       = PSP_SCREEN_LINE;
	framebuf.pixelformat = SCE_DISPLAY_PIXELFORMAT_A8B8G8R8;
	framebuf.width       = PSP_SCREEN_WIDTH;
	framebuf.height      = PSP_SCREEN_HEIGHT;
	sceDisplaySetFrameBuf(&framebuf, SCE_DISPLAY_SETBUF_NEXTFRAME);
}

char *getPspemuMemoryStickLocation() {
	switch (config.ms_location) {
		case MEMORY_STICK_LOCATION_UR0:
			return "ur0:pspemu";
		case MEMORY_STICK_LOCATION_IMC0:
			return "imc0:pspemu";
		case MEMORY_STICK_LOCATION_XMC0:
			return "xmc0:pspemu";
		case MEMORY_STICK_LOCATION_UMA0:
			return "uma0:pspemu";
		default:
			return "ux0:pspemu";
	}
}

#define THUMB_SHUFFLE(x) ((((x) & 0xFFFF0000) >> 16) | (((x) & 0xFFFF) << 16))

uint32_t encode_movw(uint8_t rd, uint16_t imm16) {
	uint32_t imm4 = (imm16 >> 12) & 0xF;
	uint32_t i = (imm16 >> 11) & 0x1;
	uint32_t imm3 = (imm16 >> 8) & 0x7;
	uint32_t imm8 = imm16 & 0xFF;
	return THUMB_SHUFFLE(0xF2400000 | (rd << 8) | (i << 26) | (imm4 << 16) | (imm3 << 12) | imm8);
}

uint32_t encode_movt(uint8_t rd, uint16_t imm16) {
	uint32_t imm4 = (imm16 >> 12) & 0xF;
	uint32_t i = (imm16 >> 11) & 0x1;
	uint32_t imm3 = (imm16 >> 8) & 0x7;
	uint32_t imm8 = imm16 & 0xFF;
	return THUMB_SHUFFLE(0xF2C00000 | (rd << 8) | (i << 26) | (imm4 << 16) | (imm3 << 12) | imm8);
}

uint32_t encode_bl(uint32_t patch_offset, uint32_t target_offset) {
	uint32_t displacement = target_offset - (patch_offset & ~0x1) - 4;
	uint32_t signbit = (displacement >> 31) & 0x1;
	uint32_t i1 = (displacement >> 23) & 0x1;
	uint32_t i2 = (displacement >> 22) & 0x1;
	uint32_t imm10 = (displacement >> 12) & 0x03FF;
	uint32_t imm11 = (displacement >> 1) & 0x07FF;
	uint32_t j1 = i1 ^ (signbit ^ 1);
	uint32_t j2 = i2 ^ (signbit ^ 1);
	uint32_t value = (signbit << 26) | (j1 << 13) | (j2 << 11) | (imm10 << 16) | imm11;
	value |= 0xF000D000;  // BL
	return THUMB_SHUFFLE(value);
}

static void* mspace;
void* mspace_create(void* base, size_t size);
void* mspace_malloc(void* mspace, size_t size);
void* mspace_realloc(void* mspace, void* ptr, size_t size);
void mspace_free(void* mspace, void* ptr);

void mspace_init() {
	SceUID mblock = sceKernelAllocMemBlock("AdrHeap", SCE_KERNEL_MEMBLOCK_TYPE_USER_RW, 0x800000, NULL);
	void* base;
	sceKernelGetMemBlockBase(mblock, &base);
	mspace = mspace_create(base, 0x800000);
}

void* adr_malloc(size_t size) {
	return mspace_malloc(mspace, size);
}

void* adr_realloc(void* ptr, size_t size) {
	return mspace_realloc(mspace, ptr, size);
}

void adr_free(void* ptr) {
	mspace_free(mspace, ptr);
}

#define MAX_PATH_LENGTH 1024
#define MAX_NAME_LENGTH 256
#define MAX_SHORT_NAME_LENGTH 64
#define MAX_MOUNT_POINT_LENGTH 16

#define DIRECTORY_SIZE (4 * 1024)
// #define TRANSFER_SIZE (128 * 1024)

static int hasEndSlash(const char *path) {
	return path[strlen(path) - 1] == '/';
}

int copyFile(const char *src_path, const char *dst_path) {
	// The source and destination paths are identical
	if (strcasecmp(src_path, dst_path) == 0) {
		return -1;
	}

	// The destination is a subfolder of the source folder
	int len = strlen(src_path);
	if (strncasecmp(src_path, dst_path, len) == 0 && (dst_path[len] == '/' || dst_path[len - 1] == '/')) {
		return -2;
	}
	SceUID fdsrc = sceIoOpen(src_path, SCE_O_RDONLY, 0);
	if (fdsrc < 0) {
		return fdsrc;
	}

	SceUID fddst = sceIoOpen(dst_path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
	if (fddst < 0) {
		sceIoClose(fdsrc);
		return fddst;
	}

	void *buf = memalign(4096, TRANSFER_SIZE);

	while (1) {
		int read = sceIoRead(fdsrc, buf, TRANSFER_SIZE);

		if (read < 0) {
			free(buf);

			sceIoClose(fddst);
			sceIoClose(fdsrc);

			sceIoRemove(dst_path);

			return read;
		}

		if (read == 0)
			break;

		int written = sceIoWrite(fddst, buf, read);

		if (written < 0) {
			free(buf);

			sceIoClose(fddst);
			sceIoClose(fdsrc);

			sceIoRemove(dst_path);

			return written;
		}
	}

	free(buf);

	// Inherit file stat
	SceIoStat stat;
	memset(&stat, 0, sizeof(SceIoStat));
	sceIoGetstatByFd(fdsrc, &stat);
	sceIoChstatByFd(fddst, &stat, 0x3B);

	sceIoClose(fddst);
	sceIoClose(fdsrc);

	return 1;
}

int copyPath(const char *src_path, const char *dst_path) {
	// The source and destination paths are identical
	if (strcasecmp(src_path, dst_path) == 0) {
		return -1;
	}

	// The destination is a subfolder of the source folder
	int len = strlen(src_path);
	if (strncasecmp(src_path, dst_path, len) == 0 && (dst_path[len] == '/' || dst_path[len - 1] == '/')) {
		return -2;
	}

	SceUID dfd = sceIoDopen(src_path);
	if (dfd >= 0) {
		SceIoStat stat;
		memset(&stat, 0, sizeof(SceIoStat));
		sceIoGetstatByFd(dfd, &stat);

		stat.st_mode |= SCE_S_IWUSR;

		int ret = sceIoMkdir(dst_path, stat.st_mode & 0xFFF);

		if (ret < 0 && ret != SCE_ERROR_ERRNO_EEXIST) {
			sceIoDclose(dfd);
			return ret;
		}

		if (ret == SCE_ERROR_ERRNO_EEXIST) {
			sceIoChstat(dst_path, &stat, 0x3B);
		}

		int res = 0;

		do {
			SceIoDirent dir;
			memset(&dir, 0, sizeof(SceIoDirent));

			res = sceIoDread(dfd, &dir);
			if (res > 0) {
				char *new_src_path = malloc(strlen(src_path) + strlen(dir.d_name) + 2);
				snprintf(new_src_path, MAX_PATH_LENGTH, "%s%s%s", src_path, hasEndSlash(src_path) ? "" : "/", dir.d_name);

				char *new_dst_path = malloc(strlen(dst_path) + strlen(dir.d_name) + 2);
				snprintf(new_dst_path, MAX_PATH_LENGTH, "%s%s%s", dst_path, hasEndSlash(dst_path) ? "" : "/", dir.d_name);

				int ret = 0;

				if (SCE_S_ISDIR(dir.d_stat.st_mode)) {
					ret = copyPath(new_src_path, new_dst_path);
				} else {
					ret = copyFile(new_src_path, new_dst_path);
				}

				free(new_dst_path);
				free(new_src_path);

				if (ret <= 0) {
					sceIoDclose(dfd);
					return ret;
				}
			}
		} while (res > 0);

		sceIoDclose(dfd);
	} else {
		return copyFile(src_path, dst_path);
	}

	return 1;
}

const char* getMsDrive() {
	switch (config.ms_location) {
		case MEMORY_STICK_LOCATION_UR0:
			return "ur0:";
		case MEMORY_STICK_LOCATION_IMC0:
			return "imc0:";
		case MEMORY_STICK_LOCATION_XMC0:
			return "xmc0:";
		case MEMORY_STICK_LOCATION_UMA0:
			return "uma0:";
		default:
			return "ux0:";
	}
}