#ifndef _EPI_EXTRA_TYPES_H
#define _EPI_EXTRA_TYPES_H

#include <psptypes.h>

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

typedef struct {
	u16 label_offset;
	u16 type;
	u32 size;
	u32 size_padded;
	u32 data_offset;
} SFOtable;

typedef struct {
	u32 signature;
	u32 version;
	u32 fields_table_offs;
	u32 values_table_offs;
	int nitems;
	SFOtable sfotable[];
} SFO;

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

#endif // _EPI_EXTRA_TYPES_H