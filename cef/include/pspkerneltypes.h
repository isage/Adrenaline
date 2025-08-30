/*
 * PSP Software Development Kit - https://github.com/pspdev
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * pspkerneltypes.h - PSP kernel types and definitions.
 *
 * Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
 * Copyright (c) 2005 James Forshaw <tyranid@gmail.com>
 * Copyright (c) 2005 John Kelley <ps2dev@kelley.ca>
 *
 */

/* Note: Some of the structures, types, and definitions in this file were
   extrapolated from symbolic debugging information found in the Japanese
   version of Puzzle Bobble. */

#ifndef PSPKERNELTYPES_H
#define PSPKERNELTYPES_H

#include <psptypes.h>

/** UIDs are used to describe many different kernel objects. */
typedef int SceUID;

/* Misc. kernel types. */
typedef unsigned int SceSize;
typedef int SceSSize;

typedef unsigned char SceUChar;
typedef unsigned int SceUInt;

/* File I/O types. */
typedef int SceMode;
typedef SceInt64 SceOff;
typedef SceInt64 SceIores;

/**
 * API types of an executable.
 *
 * `sceKernelLoadModuleWithApitype`
 */
typedef enum SceFileExecApiType {
	SCE_APITYPE_KERNEL = 0, /* ModuleMgrForKernel */
	SCE_APITYPE_BUFFER_KERNEL = 0x02,
	SCE_APITYPE_KERNEL_BLOCK = 0x03,
	SCE_APITYPE_USER = 0x10, /* ModuleMgrForUser */
	SCE_APITYPE_DNAS = 0x13,
	SCE_APITYPE_NPDRM = 0x14,
	SCE_APITYPE_VSH = 0x20,
	SCE_APITYPE_BUFFER_VSH = 0x21,
	SCE_APITYPE_BUFFER_USBWLAN = 0x30,
	SCE_APITYPE_BUFFER_MS = 0x42,
	SCE_APITYPE_BUFFER_APP = 0x43,
	SCE_APITYPE_BUFFER_BOOT_INIT_BTCNF = 0x51,
	SCE_APITYPE_BUFFER_BOOT_INIT_CONFIG = 0x52,
	SCE_APITYPE_BUFFER_DECI = 0x70,

	/** GAME EBOOT. */
    SCE_APITYPE_GAME_EBOOT = 0x110,
    /** GAME BOOT. */
    SCE_APITYPE_GAME_BOOT = 0x111,
    /** Emulated EBOOT Memory-Stick. */
    SCE_APITYPE_EMU_EBOOT_MS = 0x112,
    /** Emulated BOOT Memory-Stick. */
    SCE_APITYPE_EMU_BOOT_MS = 0x113,
    /** Emulated EBOOT EF. */
    SCE_APITYPE_EMU_EBOOT_EF = 0x114,
    /** Emulated BOOT EF. */
    SCE_APITYPE_EMU_BOOT_EF = 0x115,
    /** NP-DRM Memory-Stick. */
    SCE_APITYPE_NPDRM_MS = 0x116, /* Distributed programs and data through the Playstation Store. */
    /** Unknown. */
    SCE_APITYPE_UNK117 = 0x117,
	/** NP-DRM EF. */
    SCE_APITYPE_NPDRM_EF = 0x118, /* NP-DRM: PlayStation Network Platform Digital Rights Management */
	/** Unknown. */
    SCE_APITYPE_UNK119 = 0x119,
	/** Executable on a disc. */
	SCE_APITYPE_UMD = 0x120,
	/** Updater executable on a disc. */
	SCE_APITYPE_UMD_UPDATER = 0x121,
	/** Disc debugger. */
    SCE_APITYPE_UMD_DEBUG = 0x122,
    /** NP-9660 game on Memory Stick. */
    SCE_APITYPE_UMD_EMU_MS1 = 0x123,
	/** NP-9660 game with update/DLC (`PBOOT.PBP`) on Memory Stick. */
    SCE_APITYPE_UMD_EMU_MS2 = 0x124,
    /** NP-9660 game on internal memory (`ef0:`). */
    SCE_APITYPE_UMD_EMU_EF1 = 0x125,
    /** NP-9660 game with update/DLC (`PBOOT.PBP`) on internal memory (`ef0:`). */
    SCE_APITYPE_UMD_EMU_EF2 = 0x126,
    /** Game-sharing executable. */
    SCE_APITYPE_USBWLAN = 0x130,
    /** Unknown. */
    SCE_APITYPE_USBWLAN_DEBUG = 0x131,
    /** Updater executable on a PBP in the Memory Stick. */
    SCE_APITYPE_MS1 = 0x140,
    /** PSP executable on a PBP in the Memory Stick. */
    SCE_APITYPE_MS2 = 0x141,
    /** Unknown. */
    SCE_APITYPE_MS3 = 0x142,
    /** Applications (i.e. Comic Reader) executable on a PBP in the Memory Stick. */
    SCE_APITYPE_MS4 = 0x143,
    /** Playstation One executable on a PBP in the Memory Stick. */
    SCE_APITYPE_MS5 = 0x144,
    /** Unknown. Licensed games? */
    SCE_APITYPE_MS6 = 0x145,
    /** Updater executable on a PBP in the internal memory (`ef0:`). */
    SCE_APITYPE_EF1 = 0x151,
    /** PSP executable on a PBP in the internal memory (`ef0:`). */
    SCE_APITYPE_EF2 = 0x152,
    /** Unknown. Probably the same as MS3 but loading form `ef0`. */
    SCE_APITYPE_EF3 = 0x153,
    /** Applications (i.e. Comic Reader) executable on a PBP in the internal memory (`ef0:`). */
    SCE_APITYPE_EF4 = 0x154,
    /** Playstation One executable on a PBP in the internal memory (`ef0:`). */
    SCE_APITYPE_EF5 = 0x155,
    /** Unknown. Probably the same as MS6 but loading form `ef0`. */
    SCE_APITYPE_EF6 = 0x156,
    /** Game/APP with update/DLC on a disc. */
    SCE_APITYPE_UMD2 = 0x160,
    /** Unknown. */
    SCE_APITYPE_UNK161 = 0x161,
    /** MerlinDRM Applications on Memory Stick */
    SCE_APITYPE_MLNAPP_MS = 0x170,
    /** MerlinDRM Applications on internal memory (`ef0:`). */
    SCE_APITYPE_MLNAPP_EF = 0x171,
    /** Exit VSH Kernel. */
    SCE_APITYPE_VSH_KERNEL = 0x200,
    /** Exit Game. */
    SCE_APITYPE_VSH1 = 0x210,
    /** Exit VSH. */
    SCE_APITYPE_VSH2 = 0x220,
    /** Kernel reboot. */
    SCE_APITYPE_KERNEL_REBOOT = 0x300,
    /** Debug. Doesn't start reboot. */
    SCE_APITYPE_DEBUG = 0x420,  /* doesn't start reboot */

// Alias

	SCE_APITYPE_DISC = SCE_APITYPE_UMD,
	SCE_APITYPE_DISC2 = SCE_APITYPE_UMD2,
	SCE_APITYPE_DISC_UPDATER = SCE_APITYPE_UMD_UPDATER,
	SCE_APITYPE_DISC_DEBUG = SCE_APITYPE_UMD_DEBUG,
	SCE_APITYPE_DISC_EMU_MS1 = SCE_APITYPE_UMD_EMU_MS1,
	SCE_APITYPE_DISC_EMU_MS2 = SCE_APITYPE_UMD_EMU_MS2,
	SCE_APITYPE_DISC_EMU_EF1 = SCE_APITYPE_UMD_EMU_EF1,
	SCE_APITYPE_DISC_EMU_EF2 = SCE_APITYPE_UMD_EMU_EF2,
	SCE_APITYPE_MS_UPDATER = SCE_APITYPE_MS1,
	SCE_APITYPE_MS_GAME = SCE_APITYPE_MS2,
	SCE_APITYPE_MS_APP = SCE_APITYPE_MS4,
	SCE_APITYPE_MS_PS1 = SCE_APITYPE_MS5,
	SCE_APITYPE_EF_UPDATER = SCE_APITYPE_EF1,
	SCE_APITYPE_EF_GAME = SCE_APITYPE_EF2,
	SCE_APITYPE_EF_APP = SCE_APITYPE_EF4,
	SCE_APITYPE_EF_PS1 = SCE_APITYPE_EF5,
	SCE_APITYPE_VSH_EXITGAME = SCE_APITYPE_VSH1, /* ExitGame */
	SCE_APITYPE_VSH_EXITVSH = SCE_APITYPE_VSH2, /* ExitVSH */
} SceFileExecApiType;

/**
 * The possible boot medium types for an executable.
 */
typedef enum SceBootMediumType {
    /** The executable was booted via Flash 0 (1, 2). */
    SCE_BOOT_FLASH     = 0,
    /** The executable was booted via a Disc medium. */
    SCE_BOOT_DISC      = 0x20,
    /** The executable was booted via a Game-sharing medium. */
    SCE_BOOT_USBWLAN   = 0x30,
    /** The executable was booted via the Memory Stick medium. */
    SCE_BOOT_MS        = 0x40,
    /** The executable was booted via an unknown medium. */
    SCE_BOOT_EF        = 0x50,
    /** The executable was booted via Flash 3.*/
    SCE_BOOT_FLASH3    = 0x80,
} SceBootMediumType;

/**
 * Application types of an executable.
 */
typedef enum SceApplicationType {
	/** The application is a VSH application (i.e. VSH modules). */
	SCE_APPTYPE_VSH		= 0x100,
	/** The application is an updater. */
	SCE_APPTYPE_UPDATER	= 0x110,
	/** The application is a PSP game. */
	SCE_APPTYPE_GAME	= 0x200,
	/** The application is a Playstation One game. */
	SCE_APPTYPE_POPS	= 0x300,
	/** The application is a PSP application (i.e. Skype). */
	SCE_APPTYPE_APP		= 0x400,
} SceApplicationType;

#endif /* PSPKERNELTYPES_H */
