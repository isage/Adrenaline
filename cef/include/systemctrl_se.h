#ifndef __SCTRLLIBRARY_SE_H__
#define __SCTRLLIBRARY_SE_H__

/**
 * These functions are only available in SE-C and later,
 * and they are not in HEN
*/

enum
{
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

enum SEUmdModes
{
	MODE_INFERNO,
	MODE_MARCH33,
	MODE_NP9660,
};

enum InfernoCachePolicy
{
    CACHE_POLICY_LRU = 0,
    CACHE_POLICY_RR = 1,
};

#define ADRENALINE_CFG_MAGIC_1 0x31483943
#define ADRENALINE_CFG_MAGIC_2 0x334F4E33

typedef struct {
	int magic[2];
	int hide_corrupt;
	int	skip_logo;
	int startup_program;
	int umd_mode;
	int	vsh_cpu_speed;
	int	app_cpu_speed;
	int fake_region;
	int skip_game_boot_logo;
	int hide_mac_addr;
	int hide_dlcs;
	int hide_pic0pic1;
	int use_extended_colors;
	int use_sony_psposk;
	int no_nodrm_engine;
	int no_xmb_plugins;
	int no_game_plugins;
	int no_pops_plugins;
	int force_high_memory;
	int execute_boot_bin;
	int recovery_color;
	int enable_xmbctrl;
} AdrenalineConfig;

typedef AdrenalineConfig SEConfig;

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
