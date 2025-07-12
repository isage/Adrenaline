#ifndef __SCTRLLIBRARY_H__
#define __SCTRLLIBRARY_H__

// COMMON

#include <pspsdk.h>
#include <psptypes.h>
#include <pspiofilemgr_kernel.h>
#include <psploadexec_kernel.h>
#include <psploadcore.h>

enum BootModes
{
	BOOT_NORMAL,
	BOOT_INFERNO,
	BOOT_MARCH33,
	BOOT_NP9660,
	BOOT_RECOVERY,
};

enum BootLoadFlags
{
	BOOTLOAD_VSH = 1,
	BOOTLOAD_GAME = 2,
	BOOTLOAD_UPDATER = 4,
	BOOTLOAD_POPS = 8,
	BOOTLOAD_UMDEMU = 64, /* for original NP9660 */
};

// Different PSP models
enum {
    PSP_1000 = 0,   // 01g
    PSP_2000 = 1,   // 02g
    PSP_3000 = 2,   // 03g
    PSP_4000 = 3,   // 04g
    PSP_GO   = 4,   // 05g
    PSP_7000 = 6,   // 07g
    PSP_9000 = 8,   // 09g
    PSP_11000 = 10, // 11g
};

typedef int (* STMOD_HANDLER)(SceModule2 *);

/**
 * Restart the vsh.
 *
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL
 *
 * @returns < 0 on some errors.
 *
*/
int sctrlKernelExitVSH(struct SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a disc.
 * It is the function used by the firmware to execute the EBOOT.BIN from a disc.
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors.
*/
int sctrlKernelLoadExecVSHDisc(const char *file, struct SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a disc.
 * It is the function used by the firmware to execute an updater from a disc.
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors.
*/
int sctrlKernelLoadExecVSHDiscUpdater(const char *file, struct SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a memory stick.
 * It is the function used by the firmware to execute an updater from a memory stick.
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors.
*/
int sctrlKernelLoadExecVSHMs1(const char *file, struct SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a memory stick.
 * It is the function used by the firmware to execute games (and homebrew :P) from a memory stick.
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors.
*/
int sctrlKernelLoadExecVSHMs2(const char *file, struct SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a memory stick.
 * It is the function used by the firmware to execute ... ?
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors.
*/
int sctrlKernelLoadExecVSHMs3(const char *file, struct SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a memory stick.
 * It is the function used by the firmware to execute psx games
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors.
*/
int sctrlKernelLoadExecVSHMs4(const char *file, struct SceKernelLoadExecVSHParam *param);


/**
 * Executes a new executable with the specified apitype
 *
 * @param apitype - The apitype
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors.
*/
int sctrlKernelLoadExecVSHWithApitype(int apitype, const char *file, struct SceKernelLoadExecVSHParam *param);

/**
 * Obtain the syscall number of a given user-exported kernel function.
 *
 * @param func_addr: absolute address of kernel function.
 *
 * @returns syscall number on success, <0 on error.
 */
int sctrlKernelQuerySystemCall(void *func_addr);

/**
 *  Calculate Random Number via KIRK
 */
unsigned int sctrlKernelRand(void);

/**
 * Sets the user level of the current thread
 *
 * @param level - The user level
 * @return the previous user level on success
 */
int sctrlKernelSetUserLevel(int level);



/**
 * Finds a driver
 *
 * @param drvname - The name of the driver (without ":" or numbers)
 *
 * @returns the driver if found, NULL otherwise
 *
 */
PspIoDrv *sctrlHENFindDriver(char *drvname);

/**
 * Finds a function.
 *
 * @param szMod - The module where to search the function
 * @param szLib - The library name
 * @param nid - The nid of the function
 *
 * @returns - The function address or 0 if not found
 *
*/
u32 sctrlHENFindFunction(const char* szMod, const char* szLib, u32 nid);
#define FindProc sctrlHENFindFunction

/**
 * Replace import function stub with a function or dummy value.
 *
 * @param mod - The module where to search the function
 * @param library - The library name
 * @param nid - The nid of the function
 * @param func - The function to replace the stub. If NULL, use dummy value
 * @param dummy - The dummy value to replace the stub
 *
 * @returns 0 if successful,
 * -1 if `mod` or `library` are NULL,
 * -2 if failed to find import by NID and fail to resolve that NID from older firmware version
 * -3 if failed to find import by NID after successful resolve to older firmware version
 *
*/
int sctrlHENHookImportByNID(SceModule2 * mod, char *library, u32 nid, void *func, int dummy);

/**
 * Gets the HEN version
 *
 * @returns - The HEN version
 *
 * HEN D / SE-C :  0x00000400
 */
int sctrlHENGetVersion();

/**
 * Checks if we are in Devhook.
 *
 * @returns 1 if we are in SE-C/HEN-D for devhook  or later, 0 if we are in normal SE-C/HEN-D or later,
 * and < 0 (a kernel error code) in any other case
*/
int	sctrlHENIsDevhook();

/**
 * Checks if we are in SE.
 *
 * @returns 1 if we are in SE-C or later, 0 if we are in HEN-D or later,
 * and < 0 (a kernel error code) in any other case
*/
int	sctrlHENIsSE();

/**
 * Checks if the system already started.
 *
 * @returns 1 if system has started, 0 otherwise.
*/
int sctrlHENIsSystemBooted();

/**
 * Sets the partition 2 and 11 memory for next loadexec.
 *
 * @param p2 - The size in MB for the user partition. Must be > 0
 * @param p11 - The size in MB for partition 8. Can be 0.
 *
 * @returns 0 on success, < 0 on error.
 * The function will fail if p2+p8 > 52 or p2 == 0
 * The function will fail with -1 if can't unlock (i.e. pops, vsh),
 * -2 if already unlocked, -3 if too late to unlock.
*/
int sctrlHENSetMemory(u32 p2, u32 p11);

/**
 * Sets the speed for the cpu and bus.
 *
 * @param cpu - The cpu speed
 * @param bus - The bus speed
*/
void sctrlHENSetSpeed(int cpu, int bus);

/**
 * Sets a function to be called just before module_start of a module is gonna be called (useful for patching purposes)
 *
 * @param handler - The function, that will receive the module structure before the module is started.
 *
 * @returns - The previous set function (NULL if none);
 * @Note: because only one handler function is handled by HEN, you should
 * call the previous function in your code.
 *
 * @Example:
 *
 * STMOD_HANDLER previous = NULL;
 *
 * int OnModuleStart(SceModule2 *mod);
 *
 * void somepointofmycode()
 * {
 *		previous = sctrlHENSetStartModuleHandler(OnModuleStart);
 * }
 *
 * int OnModuleStart(SceModule2 *mod)
 * {
 *		if (strcmp(mod->modname, "vsh_module") == 0)
 *		{
 *			// Do something with vsh module here
 *		}
 *
 *		if (!previous)
 *			return 0;
 *
 *		// Call previous handler
 *
 *		return previous(mod);
 * }
 *
 * @Note2: The above example should be compiled with the flag -fno-pic
 *			in order to avoid problems with gp register that may lead to a crash.
 *
*/
STMOD_HANDLER sctrlHENSetStartModuleHandler(STMOD_HANDLER handler);



/**
 * Flush/Cleans Instruction and Data Caches
 */
void sctrlFlushCache(void);


int sctrlGetUsbState();
int sctrlStartUsb();
int sctrlStopUsb();

#ifdef __UPDATER__
int sctrlRebootDevice();
#endif

/**
 * LZ4 decompress.
 *
 * @param dest out buffer where the decompressed data will be
 * @param src source buffer with the compressed data
 * @param size size of the decompressed data
 */
int sctrlLZ4Decompress(void* dest, const void* src, int size);

/**
 * LZO decompress.
 *
 * @param dest out buffer where the decompressed data will be
 * @param dst_size return size of the decompressed data
 * @param src source buffer with the compressed data
 * @param src_size size of the compressed data
 */
int sctrlLzoDecompress(void* dest, unsigned* dst_size, void* src, unsigned src_size);


// USER ONLY
#ifdef __USER__

/**
 * Wrapper for sceKernelDeflateDecompress.
 *
 * @param dest out buffer where the decompressed data will be
 * @param src source buffer with the compressed data
 * @param size size of the decompressed data
 *
 */
int sctrlDeflateDecompress(void* dest, void* src, int size);

/**
 * GZIP decompress.
 * Wrapper for sceKernelGzipDecompress.
 *
 * @param dest out buffer where the decompressed data will be
 * @param src source buffer with the compressed data
 * @param size size of the decompressed data
 *
 */
int sctrlGzipDecompress(void* dest, void* src, int size);

#endif // __USER__

// KERNEL ONLY
#ifdef __KERNEL__

#include <pspkernel.h>
#include <pspinit.h>

/**
 * Loads a module on next reboot. Only kernel mode.
 *
 * @param module_after - The path of the module which is loaded after the module to be loaded.
   The module passed to this function will be loaded just before that module.
 * @param buf - The buffer containing the module - Don't deallocate this one. It has to reside in kernel memory.
 * @param size - The size of the module
 * @param flags - The modes in which the module should be loaded, one of BootLoadFlags
 *
 * @Example:
 * sctrlHENLoadModuleOnReboot("/kd/usersystemlib.prx", module_buffer, module_size, BOOTLOAD_GAME | BOOTLOAD_POPS | BOOTLOAD_UMDEMU);
 *
 * This will load the module contained in module_buffer just before /kd/usersystemlib.prx in the next reboot, if the mode of next reboot is game, pops or umdemu
 *
 * @Remarks: Don't use too early modules in first param like "/kd/init.prx" or "/kd/systemctrl.prx", or your module may not load properly
 * Only one module will be loaded on reboot with this function.
 * If this function is called many times, only the last one will be considered.
 * By making a module to load itself using this function, and calling
 * sctrlHENLoadModuleOnReboot on module_start, a prx can cause itself to be resident in the modes choosen by flags.
 * If all flags are selected, the module will stay resident until a psp shutdown, or until sctrlHENLoadModuleOnReboot is not called.
*/
void sctrlHENLoadModuleOnReboot(char *module_after, void *buf, int size, int flags);

/** Changes a syscall to another function
 *
 * @param addr - the address of the original function
 * @param newaddr - the address of the new function
*/
void sctrlHENPatchSyscall(u32 addr, void *newaddr);

typedef int (* HEN_REG_HOMEBREW_LOADER_HANDLER)(const char *path, int flags, SceKernelLMOption *option);
int sctrlHENRegisterHomebrewLoader(HEN_REG_HOMEBREW_LOADER_HANDLER handler);

void lowerString(char *orig, char *ret, int strSize);
int strncasecmp(const char *s1, const char *s2, SceSize n);
int strcasecmp(const char *s1, const char *s2);
char *strncat(char *dest, const char *src, SceSize count);
SceSize strncat_s(char *dest, SceSize numberOfElements, const char *src, SceSize count);
SceSize strncpy_s(char *dest, SceSize numberOfElements, const char *src, SceSize count);

#endif // __KERNEL__


#endif // __SCTRLLIBRARY_H__