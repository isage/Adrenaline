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

#ifndef __SCTRLLIBRARY_SE_H__
#define __SCTRLLIBRARY_SE_H__

#include <psptypes.h>

/**
 * These functions are only available in SE-C and later,
 * and they are not in HEN
*/

enum FakeRegionOptions {
	FAKE_REGION_DISABLED = 0,
	FAKE_REGION_JAPAN = 1,
	FAKE_REGION_AMERICA = 2,
	FAKE_REGION_EUROPE = 3,
	FAKE_REGION_KOREA = 4,
	FAKE_REGION_UNK = 5,
	FAKE_REGION_UNK2 = 6,
	FAKE_REGION_AUSTRALIA = 7,
	FAKE_REGION_HONGKONG = 8,
	FAKE_REGION_TAIWAN = 9,
	FAKE_REGION_RUSSIA = 10,
	FAKE_REGION_CHINA = 11,
	FAKE_REGION_DEBUG_TYPE_I = 12,
	FAKE_REGION_DEBUG_TYPE_II = 13,
};

enum SEUmdModes {
	MODE_INFERNO,
	MODE_MARCH33,
	MODE_NP9660,
};

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

enum CpuBusSpeed {
	CLOCK_SPEED_DISABLED,
	CLOCK_SPEED_20_10,
	CLOCK_SPEED_75_37,
	CLOCK_SPEED_100_50,
	CLOCK_SPEED_133_66,
	CLOCK_SPEED_222_111,
	CLOCK_SPEED_266_133,
	CLOCK_SPEED_300_150,
	CLOCK_SPEED_333_166,
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

#define ADRENALINE_CFG_MAGIC_1 0x192EFC3C
#define ADRENALINE_CFG_MAGIC_2 0x17BEB6AA

typedef struct {
	int magic[2];
	/** 0 - Disabled, 1 - Enabled */
	u8 hide_corrupt;
	/** 0 - Disabled, 1 - Enabled */
	u8	skip_logo;
	/** 0 - Disabled, 1 - Enabled */
	u8 startup_program;
	/** One of `SEUmdModes` */
	u8 umd_mode;
	/** One of `CpuBusSpeed` */
	u8	vsh_cpu_speed;
	/** One of `CpuBusSpeed` */
	u8	app_cpu_speed;
	/** One of `FakeRegionOptions` */
	u8 fake_region;
	/** 0 - Disabled, 1 - Enabled */
	u8 skip_game_boot_logo;
	/** 0 - Disabled, 1 - Enabled */
	u8 hide_mac_addr;
	/** 0 - Disabled, 1 - Enabled */
	u8 hide_dlcs;
	/** One of `HidePicsOpt` */
	u8 hide_pic0pic1;
	/** One of `ExtendedColors` */
	u8 extended_colors;
	/** 0 - Disabled, 1 - Enabled */
	u8 use_sony_psposk;
	/** 0 - Use, 1 - Do not use */
	u8 no_nodrm_engine;
	/** 0 - Use, 1 - Do not use */
	u8 no_xmb_plugins;
	/** 0 - Use, 1 - Do not use */
	u8 no_game_plugins;
	/** 0 - Use, 1 - Do not use */
	u8 no_pops_plugins;
	/** One of `ForceHighMemory` */
	u8 force_high_memory;
	/** 0 - Disabled, 1 - Enabled */
	u8 execute_boot_bin;
	/** One of `RecoveryColor` */
	u8 recovery_color;
	/** 0 - load xmbctrl, 1 - not load xmbctrl */
	u8 no_xmbctrl;
	/** Inferno cache type. One of `InfernoCacheConf` */
	u8 iso_cache;
	/** Inferno cache partition 2 or 11 (automatic) */
	u8 iso_cache_partition;
	/** Inferno cache size (in KB) for each cache item. One of `IsoCacheSizeConf` */
	u8 iso_cache_size;
	/** Number of inferno cache items. One of `IsoCacheNumberConf` */
	u8 iso_cache_num;
	/** Simulate UMD seek time. Zero - Off, `>0` - seek time factor, i.e. value that will be multiplied on amount of bytes to be read */
	u8 umd_seek;
	/** Simulate UMD seek time. Zero - Off, `>0` - seek time factor, i.e. value that will be multiplied on amount of bytes to be read */
    u8 umd_speed;
	/** Cache `ms0:`. 0 - Use cache, 1 - do not use cache. */
    u8 no_ms_cache;
	/** Use `ge_2.prx` instead of `ge.prx`. 0 - Off, 1 - On. */
	u8 use_ge2;
	/** Use `kermit_me_wrapper_2.prx` instead of `kermit_me_wrapper.prx`. 0- Off, 1 = On. */
	u8 use_me2;
	/** Hide CFW files from games. 0 - Hide, 1 - Do not hide */
	u8 no_hide_cfw_files;
} AdrenalineConfig;

typedef AdrenalineConfig SEConfig;

#define IS_EPI_CONFIG(config) ((((AdrenalineConfig*)config)->magic[0] == ADRENALINE_CFG_MAGIC_1) && (((AdrenalineConfig*)config)->magic[1] == ADRENALINE_CFG_MAGIC_2))

/**
 * Gets the SE/OE version
 *
 * @returns the SE version
 *
 * 3.03 OE-A: 0x00000500
*/
int sctrlSEGetVersion();

/**
 * Gets the SE configuration.
 * Avoid using this function, it may corrupt your program.
 * Use sctrlSEGetCongiEx function instead.
 *
 * @param config - pointer to a AdrenalineConfig structure that receives the SE configuration
 * @returns 0 on success
*/
int sctrlSEGetConfig(AdrenalineConfig *config);

/**
 * Gets the SE configuration
 *
 * @param config - pointer to a AdrenalineConfig structure that receives the SE configuration
 * @param size - The size of the structure
 * @returns 0 on success
*/
int sctrlSEGetConfigEx(AdrenalineConfig *config, int size);

/**
 * Sets the SE configuration.
 * This function can corrupt the configuration in flash, use
 * sctrlSESetConfigEx instead.
 *
 * @param config - pointer to a AdrenalineConfig structure that has the SE configuration to set
 * @returns 0 on success
*/
int sctrlSESetConfig(AdrenalineConfig *config);

/**
 * Sets the SE configuration.
 *
 * @param config - pointer to a AdrenalineConfig structure that has the SE configuration to set
 * @param size - the size of the structure
 * @returns 0 on success
*/
int sctrlSESetConfigEx(AdrenalineConfig *config, int size);

/**
 * Initiates the emulation of a disc from an ISO9660/CSO file.
 *
 * @param file - The path of the
 * @param noumd - Wether use noumd or not
 * @param isofs - Wether use the custom SE isofs driver or not
 *
 * @returns 0 on success
 *
 * @note 1. When setting noumd to 1, isofs should also be set to 1,
 * otherwise the umd would be still required.
 *
 * @note 2. The function doesn't check if the file is valid or even if it exists
 * and it may return success on those cases
 *
 * @note 3. This function is not available in SE for devhook
 *
 * @example
 * AdrenalineConfig config;
 *
 * sctrlSEGetConfig(&config);
 *
 * if (config.usenoumd) {
 *		sctrlSEMountUmdFromFile("ms0:/ISO/mydisc.iso", 1, 1);
 * } else {
 *		sctrlSEMountUmdFromFile("ms0:/ISO/mydisc.iso", 0, config.useisofsonumdinserted);
 * }
*/
int sctrlSEMountUmdFromFile(char *file, int noumd, int isofs);

/**
 * Umounts an iso.
 *
 * @returns 0 on success
*/
int sctrlSEUmountUmd(void);

/**
 * Forces the umd disc out state
 *
 * @param out - non-zero for disc out, 0 otherwise
 *
*/
void sctrlSESetDiscOut(int out);

/**
 * Sets the disctype.
 *
 * @param type - the disctype (0x10=game, 0x20=video, 0x40=audio) or one of `IsoDiscType`
*/
void sctrlSESetDiscType(int type);

/**
 * Get the disctype.
 *
 * @returns The ISO disctype - One of `IsoDiscType`
*/
int sctrlSEGetDiscType(void);

/**
 * Sets the boot config file for next reboot
 *
 * @param index - The index identifying the file (0 -> normal bootconf, 1 -> inferno driver bootconf, 2 -> march33 driver bootconf, 3 -> np9660 bootcnf, 4 -> recovery bootconf)
*/
void sctrlSESetBootConfFileIndex(int index);

/**
 * Gets the boot config file for next reboot
 *
 * @returns The index identifying the file (0 -> normal bootconf, 1 -> inferno driver bootconf, 2 -> march33 driver bootconf, 3 -> np9660 bootcnf, 4 -> recovery bootconf)
*/
int sctrlSEGetBootConfFileIndex(void);

#ifdef __KERNEL__

/**
 * Gets the current umd file (kernel only)
 *
 * @returns The current umd file
*/
char *sctrlSEGetUmdFile();
char *GetUmdFile();

/**
 * Gets the current umd file
 *
 * @param input If not NULL, a umd filename to set
 *
 * @returns The current umd file
*/
char *sctrlSEGetUmdFileEx(char *input);

/**
 * Sets the current umd file (kernel only)
 *
 * @param file - The umd file
*/
void sctrlSESetUmdFile(const char *file);
void SetUmdFile(const char *file);

/**
 * Sets the current umd file and copy the previous to `input`
 *
 * @param file The umd file
 * @param input The output buffer for the previously set umd file. Or NULL to be ignored
*/
void sctrlSESetUmdFileEx(const char *file, char *input);

/**
 * Immediately sets the SE configuration in memory without saving to flash.
 * This function can corrupt the configuration in memory, use
 * sctrlSEApplyConfigEX instead.
 *
 * @param config - pointer to a SEConfig structure that has the SE configuration to set
*/
void sctrlSEApplyConfig(AdrenalineConfig *conf);

/**
 * Immediately sets the SE configuration in memory without saving to flash.
 *
 * @param config - pointer to a SEConfig structure that has the SE configuration to set
 * @returns 0 on success, and -1 on error
*/
int sctrlSEApplyConfigEX(AdrenalineConfig *conf, int size);

/**
 * Sets the speed for the cpu and bus.
 *
 * @param cpu - The cpu speed
 * @param bus - The bus speed
*/
void SetSpeed(int cpu, int bus);

/**
 * Deallocate memory allocated by oe_malloc.
 *
 * @param ptr - pointer to the allocated memory.
 */
void oe_free(void *ptr);
/**
 * Helper function to allocate memory on P1 (kernel memory).
 *
 * @param size - amount of bytes to allocate.
 *
 * @returns pointer to allocated buffer, or NULL on error.
 */
void *oe_malloc(SceSize size);
/**
 * Deallocate memory allocated by user_malloc.
 *
 * @param ptr - pointer to the allocated memory.
 */
void user_free(void *ptr);
/**
 * Helper function to allocate memory on P2 (user memory).
 *
 * @param size - amount of bytes to allocate.
 *
 * @returns pointer to allocated buffer, or NULL on error.
 */
void *user_malloc(SceSize size);
/**
 * Helper function to allocate aligned memory on P2 (user memory).
 *
 * @param align - The alignment
 * @param size - amount of bytes to allocate.
 *
 * @returns pointer to allocated buffer, or NULL on error.
 */
void *user_memalign(SceSize align, SceSize size);
int mallocinit();

#endif // __KERNEL__

#endif
