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

#ifndef __ADRENALINE_COMPAT_H__
#define __ADRENALINE_COMPAT_H__
#include "adrenaline_version.h"

#define ADRENALINE_TITLEID "PSPEMUCFW"

#define SCE_PSPEMU_FLASH0_PACKAGE_SIZE 0x920000
#define SCE_PSPEMU_EXTRA_MEMORY 0x8B000000
#define SCE_PSPEMU_FLASH0_RAMFS 0x8B000000
#define SCE_PSPEMU_SCRATCHPAD 0x8BD00000
#define SCE_PSPEMU_VRAM 0x8BE00000

#define SCE_PSPEMU_FRAMEBUFFER 0x74000000
#define SCE_PSPEMU_FRAMEBUFFER_SIZE 0x88000

#define PSP_EXTRA_RAM 0x8A000000
#define PSP_EXTRA_RAM_SIZE (32 * 1024 *1024)
#define PSP_RAM_SIZE (64 * 1024 * 1024)

#define PSP_SCREEN_WIDTH 480
#define PSP_SCREEN_HEIGHT 272
#define PSP_SCREEN_LINE 512

#define EPI_REBOOTEX_CFG_ADDR 0x88FB0000
#define EPI_REBOOTEX_MOD_ADDR 0x88FC0000
#define EPI_CONFIG_ADDR 0x08800010

typedef struct {
	SceSize size;
	char shortFileName[13];
	char __padding__[3];
	char longFileName[1024];
} SceFatMsDirent;

typedef struct {
	unsigned int max_clusters;
	unsigned int free_clusters;
	unsigned int max_sectors;
	unsigned int sector_size;
	unsigned int sector_count;
} ScePspemuIoDevInfo;

enum KermitModes {
	KERMIT_MODE_NONE, // 0x0
	KERMIT_MODE_UNK_1, // 0x1
	KERMIT_MODE_UNK_2, // 0x2
	KERMIT_MODE_MSFS, // 0x3
	KERMIT_MODE_FLASHFS, // 0x4
	KERMIT_MODE_AUDIOOUT, // 0x5
	KERMIT_MODE_ME, // 0x6
	KERMIT_MODE_LOWIO, // 0x7
	KERMIT_MODE_POCS_USBPSPCM, // 0x8
	KERMIT_MODE_PERIPHERAL, // 0x9
	KERMIT_MODE_WLAN, // 0xa
	KERMIT_MODE_AUDIOIN, // 0xb
	KERMIT_MODE_USB, // 0xc
	KERMIT_MODE_UTILITY, // 0xd
	KERMIT_MODE_EXTRA_1, // 0x0
	KERMIT_MODE_EXTRA_2, // 0x0
};

enum KermitVirtualInterrupts {
	KERMIT_VIRTUAL_INTR_NONE,
	KERMIT_VIRTUAL_INTR_AUDIO_CH1,
	KERMIT_VIRTUAL_INTR_AUDIO_CH2,
	KERMIT_VIRTUAL_INTR_AUDIO_CH3,
	KERMIT_VIRTUAL_INTR_ME_DMA_CH1,
	KERMIT_VIRTUAL_INTR_ME_DMA_CH2,
	KERMIT_VIRTUAL_INTR_ME_DMA_CH3,
	KERMIT_VIRTUAL_INTR_WLAN_CH1,
	KERMIT_VIRTUAL_INTR_WLAN_CH2,
	KERMIT_VIRTUAL_INTR_IMPOSE_CH1,
	KERMIT_VIRTUAL_INTR_POWER_CH1,
	KERMIT_VIRTUAL_INTR_CAMERA_CH1,	// <- used after settings
	KERMIT_VIRTUAL_INTR_USBGPS_CH1,
	KERMIT_VIRTUAL_INTR_USBPSPCM_CH1,
};

enum KermitArgumentModes {
  KERMIT_INPUT_MODE = 0x1,
  KERMIT_OUTPUT_MODE = 0x2,
};

typedef struct {
	uint32_t cmd; //0x0
	SceUID sema_id; //0x4
	uint64_t *response; //0x8
	uint32_t padding; //0xC
	uint64_t args[14]; // 0x10
} SceKermitRequest; //0x80

// 0xBFC00800
typedef struct {
	uint32_t cmd; //0x00
	SceKermitRequest *request; //0x04
} SceKermitCommand; //0x8

// 0xBFC00840
typedef struct {
	uint64_t result; //0x0
	SceUID sema_id; //0x8
	int32_t unk_C; //0xC
	uint64_t *response; //0x10
	uint64_t unk_1C; //0x1C
} SceKermitResponse; //0x24 or 0x30????

// 0xBFC008C0
typedef struct {
	int32_t unk_0; //0x0
	int32_t unk_4; //0x4
} SceKermitInterrupt; //0x8

enum SaveStateModes {
	SAVESTATE_MODE_NONE,
	SAVESTATE_MODE_SAVE,
	SAVESTATE_MODE_LOAD,
};

enum AdrenalinePspCommands {
	ADRENALINE_PSP_CMD_NONE,
	ADRENALINE_PSP_CMD_REINSERT_MS,
	ADRENALINE_PSP_CMD_SAVESTATE,
	ADRENALINE_PSP_CMD_LOADSTATE,
};

enum AdrenalineVitaCommands {
	ADRENALINE_VITA_CMD_NONE,
	ADRENALINE_VITA_CMD_SAVESTATE,
	ADRENALINE_VITA_CMD_LOADSTATE,
	ADRENALINE_VITA_CMD_GET_USB_STATE,
	ADRENALINE_VITA_CMD_START_USB,
	ADRENALINE_VITA_CMD_STOP_USB,
	ADRENALINE_VITA_CMD_PAUSE_POPS,
	ADRENALINE_VITA_CMD_RESUME_POPS,
	ADRENALINE_VITA_CMD_POWER_SHUTDOWN,
	ADRENALINE_VITA_CMD_POWER_REBOOT,
	ADRENALINE_VITA_CMD_PRINT,
	ADRENALINE_VITA_CMD_UPDATE,
	ADRENALINE_VITA_CMD_APP_STARTED,
	ADRENALINE_VITA_CMD_POWER_TICK,
};

enum AdrenalineVitaResponse {
	ADRENALINE_VITA_RESPONSE_NONE,
	ADRENALINE_VITA_RESPONSE_SAVED,
	ADRENALINE_VITA_RESPONSE_LOADED,
};

typedef struct SceIoMount { // size is 0x34-bytes
    uint32_t mount;
    int mount_id;
    char assign_name[0x20];
    SceUInt32 opened_entry_num;
    SceBool unk1;
    int unk2;
} SceIoMount;

typedef struct SceIoMountInfo { // size is 0x54C-bytes
	uint32_t mount;
	int mount_id;
	char assign_name[0x20];
	SceUInt32 opened_entry_num;
	int unk1[23];
	char unk2[0x400];
	int unk3;
	char unk4[0x40];
	char block_dev_name[0x40];
	char unk5[0x40];
} SceIoMountInfo;

typedef struct {
	int savestate_mode;
	int num;
	unsigned int sp;
	unsigned int ra;

	int pops_mode;
	int draw_psp_screen_in_pops;
	char title[128];
	char titleid[12];
	char filename[256];

	int psp_cmd;
	int vita_cmd;
	int psp_response;
	int vita_response;

	char printbuf[1024];
} SceAdrenaline;

#define ADRENALINE_SIZE 0x2000
#define ADRENALINE_ADDRESS 0xABCDE000

#define DRAW_NATIVE 0xABCDEF00
#define NATIVE_FRAMEBUFFER 0x0A400000

#define CONVERT_ADDRESS(addr) (0x68000000 + (addr & 0x0FFFFFFF))

#endif
