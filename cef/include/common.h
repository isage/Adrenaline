#ifndef __COMMON_H__
#define __COMMON_H__

#include <pspsdk.h>
#include <pspkernel.h>

#include <psperror.h>

#include <kubridge.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <psperror.h>

#include <pspexception.h>
#include <pspsysevent.h>
#include <psputilsforkernel.h>
#include <pspsysmem.h>
#include <psploadexec.h>
#include <pspthreadman_kernel.h>

#include <pspusb.h>
#include <pspusbstor.h>
#include <pspumd.h>
#include <psprtc.h>
#include <pspreg.h>
#include <pspctrl.h>
#include <psppower.h>
#include <pspaudio.h>
#include <pspdisplay.h>
#include <pspdisplay_kernel.h>
#include <psputility.h>
#include <pspcrypt.h>

#include <macros.h>

// If COMMON_H_SYSCLIB_USER is defined, it includes the sysclib_user.h definitions
// And other other headers from STD C must be manually included if necessary
#ifdef COMMON_H_SYSCLIB_USER
#include <sysclib_user.h>
#include <stdarg.h>
#else
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <malloc.h>
#endif

#include "../../adrenaline_compat.h"

typedef enum IsoDiscType {
	ISO_DISC_TYPE_GAME = 0x10,
	ISO_DISC_TYPE_VIDEO = 0x20,
	ISO_DISC_TYPE_AUDIO = 0x40,
} IsoDiscType;

typedef struct {
	u32 magic;
	u32 version;
	u32 param_offset;
	u32 icon0_offset;
	u32 icon1_offset;
	u32 pic0_offset;
	u32 pic1_offset;
	u32 snd0_offset;
	u32 elf_offset;
	u32 psar_offset;
} PBPHeader;

typedef struct  __attribute__((packed)) {
	u32 signature;
	u32 version;
	u32 fields_table_offs;
	u32 values_table_offs;
	int nitems;
} SFOHeader;

typedef struct __attribute__((packed)) {
	u16 field_offs;
	u8  unk;
	u8  type; // 0x2 -> string, 0x4 -> number
	u32 unk2;
	u32 unk3;
	u16 val_offs;
	u16 unk4;
} SFODir;

typedef struct BtcnfHeader {
	u32 signature; // 0
	u32 devkit;		// 4
	u32 unknown[2];  // 8
	u32 modestart;  // 0x10
	int nmodes;  // 0x14
	u32 unknown2[2];  // 0x18
	u32 modulestart; // 0x20
	int nmodules;  // 0x24
	u32 unknown3[2]; // 0x28
	u32 modnamestart; // 0x30
	u32 modnameend; // 0x34
	u32 unknown4[2]; // 0x38
}  __attribute__((packed)) BtcnfHeader;

typedef struct ModeEntry {
	u16 maxsearch;
	u16 searchstart; //
	int mode1;
	int mode2;
	int reserved[5];
} __attribute__((packed)) ModeEntry;

typedef struct ModuleEntry {
	u32 stroffset; // 0
	int reserved; // 4
	u16 flags; // 8
	u8 loadmode; // 10
	u8 signcheck; // 11
	int reserved2; // 12
	u8  hash[0x10]; // 16
} __attribute__((packed)) ModuleEntry; // 32

typedef struct {
	char *name;
	void *buffer;
	u32 size;
} BootFile;

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
	char game_id[10];
} RebootexConfig;

int sctrlGetUsbState();
int sctrlStartUsb();
int sctrlStopUsb();

#endif