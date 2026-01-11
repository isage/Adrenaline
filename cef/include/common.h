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

#include <rebootexconfig.h>
#include <cfwmacros.h>

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

#include <extratypes.h>

enum InfernoCacheConf {
	CACHE_CONFIG_LRU,
    CACHE_CONFIG_RR,
	CACHE_CONFIG_OFF,
};

enum IsoCacheNumberConf {
	ISO_CACHE_NUM_AUTO,
	ISO_CACHE_NUM_001,
	ISO_CACHE_NUM_002,
	ISO_CACHE_NUM_004,
	ISO_CACHE_NUM_008,
	ISO_CACHE_NUM_016,
	ISO_CACHE_NUM_032,
	ISO_CACHE_NUM_064,
	ISO_CACHE_NUM_128,
};

enum IsoCacheSizeConf {
	ISO_CACHE_SIZE_AUTO,
	ISO_CACHE_SIZE_01KB,
	ISO_CACHE_SIZE_02KB,
	ISO_CACHE_SIZE_04KB,
	ISO_CACHE_SIZE_08KB,
	ISO_CACHE_SIZE_16KB,
	ISO_CACHE_SIZE_32KB,
	ISO_CACHE_SIZE_64KB,
};

enum ForceHighMemory {
	HIGHMEM_OPT_OFF,
	HIGHMEM_OPT_STABLE,
	HIGHMEM_OPT_MAX,
};

enum RecoveryColor {
	RECOVERY_COLOR_BLUE,
	RECOVERY_COLOR_GREEN,
	RECOVERY_COLOR_RED,
	RECOVERY_COLOR_GRAY,
	RECOVERY_COLOR_PINK,
	RECOVERY_COLOR_PURPLE,
	RECOVERY_COLOR_CYAN,
	RECOVERY_COLOR_ORANGE,
	RECOVERY_COLOR_YELLOW,
	RECOVERY_COLOR_B_WHITE,
	RECOVERY_COLOR_B_RED,
	RECOVERY_COLOR_B_GREEN,
	RECOVERY_COLOR_B_BLUE,
};

enum ExtendedColors {
	EXTENDED_COLOR_DISABLED,
	/** PSP 2000 Extended Color */
	EXTENDED_COLOR_02G,
	/** PSP 3000 Extended Color */
	EXTENDED_COLOR_03G,
};

enum HidePicsOpt {
	PICS_OPT_DISABLED,
	PICS_OPT_BOTH,
	PICS_OPT_PIC0_ONLY,
	PICS_OPT_PIC1_ONLY,
};

int sctrlGetUsbState();
int sctrlStartUsb();
int sctrlStopUsb();

#endif