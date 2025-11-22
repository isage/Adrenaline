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

#ifndef __SCTRLLIBRARY_H__
#define __SCTRLLIBRARY_H__

// COMMON

#include <pspsdk.h>
#include <psptypes.h>
#include <pspiofilemgr_kernel.h>
#include <psploadexec.h>
#include <psploadexec_kernel.h>
#include <psploadcore.h>

enum BootModes {
	BOOT_NORMAL,
	BOOT_INFERNO,
	BOOT_MARCH33,
	BOOT_NP9660,
	BOOT_RECOVERY,
	BOOT_VSHUMD,
	BOOT_UPDATERUMD,
};

enum BootLoadFlags {
	BOOTLOAD_VSH = 1,
	BOOTLOAD_GAME = 2,
	BOOTLOAD_UPDATER = 4,
	BOOTLOAD_POPS = 8,
	BOOTLOAD_UMDEMU = 64, /* for original NP9660 */
};

// Different PSP models
enum PspModels {
    PSP_1000 = 0,   // 01g
    PSP_2000 = 1,   // 02g
    PSP_3000 = 2,   // 03g
    PSP_4000 = 3,   // 04g
    PSP_GO   = 4,   // 05g
    PSP_7000 = 6,   // 07g
    PSP_9000 = 8,   // 09g
    PSP_11000 = 10, // 11g
};

typedef int (* STMOD_HANDLER)(SceModule *);

/**
 * Restart the vsh.
 *
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL
 *
 * @returns < 0 on some errors.
 *
*/
int sctrlKernelExitVSH(SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a disc.
 * It is the function used by the firmware to execute the EBOOT.BIN from a disc.
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors.
*/
int sctrlKernelLoadExecVSHDisc(const char *file, SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a disc.
 * It is the function used by the firmware to execute an updater from a disc.
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors.
*/
int sctrlKernelLoadExecVSHDiscUpdater(const char *file, SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a memory stick.
 * It is the function used by the firmware to execute an updater from a memory stick.
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors.
*/
int sctrlKernelLoadExecVSHMs1(const char *file, SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a memory stick.
 * It is the function used by the firmware to execute games (and homebrew :P) from a memory stick.
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors.
*/
int sctrlKernelLoadExecVSHMs2(const char *file, SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a memory stick.
 * It is the function used by the firmware to execute ... ?
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors.
*/
int sctrlKernelLoadExecVSHMs3(const char *file, SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a memory stick.
 * It is the function used by the firmware to execute application (e.g. Comic Reader) from a memory stick.
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors.
*/
int sctrlKernelLoadExecVSHMs4(const char *file, SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a memory stick.
 * It is the function used by the firmware to execute PS1 games from a memory stick..
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors.
*/
int sctrlKernelLoadExecVSHMs5(const char *file, SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from the internal memory.
 * It is the function used by the firmware to execute an updater from the internal memory.
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors.
*/
int sctrlKernelLoadExecVSHEf1(const char *file, SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from the internal memory.
 * It is the function used by the firmware to execute games (and homebrew :P) from the internal memory.
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors.
*/
int sctrlKernelLoadExecVSHEf2(const char *file, SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from the internal memory.
 * It is the function used by the firmware to execute ... ?
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors.
*/
int sctrlKernelLoadExecVSHEf3(const char *file, SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a internal memory.
 * It is the function used by the firmware to execute application (e.g. Comic Reader) from the internal memory
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors.
*/
int sctrlKernelLoadExecVSHEf4(const char *file, SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a internal memory.
 * It is the function used by the firmware to execute PS1 games from the internal memory.
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors.
*/
int sctrlKernelLoadExecVSHEf5(const char *file, SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable with the specified apitype
 *
 * @param apitype - The apitype
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors.
*/
int sctrlKernelLoadExecVSHWithApitype(int apitype, const char *file, SceKernelLoadExecVSHParam *param);

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
 *
 * @returns A random number
 */
u32 sctrlKernelRand(void);

/**
 * Sets the user level of the current thread
 *
 * @param level - The user level
 *
 * @return the previous user level on success
 */
int sctrlKernelSetUserLevel(int level);

/**
 * Sets the api type
 *
 * @param apitype - The apitype to set. One of `SceFileExecApiType` (or `PSPInitApitype`)
 *
 * @returns the previous apitype
 *
 * @note - this will modify also the value of sceKernelBootFrom, since the value of
 * bootfrom is calculated from the apitype
 */
int sctrlKernelSetInitApitype(int apitype);

/**
 * Sets the filename of the launched executable.
 *
 * @param filename - The filename to set
 *
 * @returns 0 on success
 */
int sctrlKernelSetInitFileName(char * filename);
#define sctrlKernelSetUMDEmuFile sctrlKernelSetInitFileName;

/**
 * Sets the init key config
 *
 * @param key - The key code. One of `SceApplicationType` (or `PSPKeyConfig`)
 *
 * @returns the previous Application type
 */
int sctrlKernelSetInitKeyConfig(int key);

/**
 * Sets the devkit version
 *
 * @param version - The devkit version to set
 *
 * @return the previous devkit version
 */
int sctrlKernelSetDevkitVersion(int version);

/**
 * Obtain boot device
 *
 * @returns The boot device.
 */
int sctrlKernelBootFrom(void);

/**
 * Resolves the NID of a library if missing.
 *
 * @param libname The name of the library
 * @param nid The NID to resolve
 *
 * @returns The function pointer to the resolved function, `0` otherwise
 *
 * @note Compat with ARK CFW
 */
u32 sctrlKernelResolveNid(const char *libname, u32 nid);

/**
 * Enable/disable NID Resolver on particular library
 *
 * @param libname the name of the library to be enabled/disabled
 * @param enabled 0 - disabled, != 0 - enabled
 *
 * @Example:
 * sctrlKernelSetNidResolver("sceImpose_driver", 0); // disable sceImpose_driver resolving
 *
 * @return previous value if set, < 0 on error
 *
 * @note Compat with ARK CFW. On Adrenaline returns `SCE_ENOSYS`
 */
int sctrlKernelSetNidResolver(char* libname, u32 enabled);

/**
 * Finds a driver
 *
 * @param drvname - The name of the driver (without ":" or numbers)
 *
 * @returns the driver if found, NULL otherwise
 *
 */
PspIoDrv *sctrlHENFindDriver(const char *drvname);

/**
 *  Find a import library stub table.
 *
 * @param mod - The module where to search the function
 * @param library - The library name
 *
 * @returns The reference to the stub table ot NULL if not found.
 */
SceLibraryStubTable* sctrlHENFindImportLib(SceModule* mod, const char* library);
#define sctrlFindImportLib sctrlHENFindImportLib

/**
 * Finds an exported function.
 *
 * @param mod_name - The module where to search the function
 * @param library - The library name
 * @param nid - The nid of the function
 *
 * @returns - The function address or 0 if not found
 *
*/
u32 sctrlHENFindFunction(const char* mod_name, const char* library, u32 nid);
#define FindProc sctrlHENFindFunction

/**
 * Finds an exported function in a ::SceModule.
 *
 * Good to optimize when you already have a ::SceModule module and wants to
 * find a function in that module.
 *
 * @param mod - The module where to search the function
 * @param library - The library name
 * @param nid - The nid of the function
 *
 * @returns - The function address or 0 if not found
 *
*/
u32 sctrlHENFindFunctionInMod(SceModule * mod, const char *library, u32 nid);
#define FindProcInMod sctrlHENFindFunctionInMod

/**
 * Finds an exported function in the system that matches the specified library name and NID.
 *
 * @param library The library name
 * @param nid The nid of the function
 * @param user_mods_only Set to 1 to only receive UIDs from user mode modules. Set to 0 to receive UIDs from kernel loaded modules.
 *
 * @returns - The function address or 0 if not found
 */
u32 sctrlHENFindFunctionOnSystem(const char *libname, u32 nid, int user_mods_only);

/**
 * Finds a import function stub in a ::SceModule.
 *
 * Good to optimize when you already have a ::SceModule module and wants to
 * find a stub in that module.
 *
 * @param mod - The module where to search the import
 * @param library - The library name
 * @param nid - The nid of the function
 *
 * @returns - The function address or 0 if not found
 *
*/
u32 sctrlHENFindImportInMod(SceModule * mod, const char *library, u32 nid);

/**
 * Replace import function stub in a module with a function or dummy value.
 *
 * This function autodetects whether Syscalls are used or not. If a syscall is
 * used, the function creates a new syscall and redirects the stub to the `func`
 * syscall; manually exporting in exports.exp is still required for Syscalls to
 * work.
 *
 * @param mod - The module where to search the function
 * @param library - The library name
 * @param nid - The nid of the function
 * @param func - The function to replace the stub. If NULL, use dummy value
 * @param dummy - If `func` is NULL, it is the dummy value to replace the stub.
 *
 * @returns
 * - 0 if successful,
 * -1 if `mod` or `library` are NULL,
 * -2 if failed to find import by NID and fail to resolve that NID from older firmware version
 * -3 if failed to find import by NID after successful resolve to older firmware version
 * -4 if failed to find syscall table
*/
int sctrlHENHookImportByNID(SceModule * mod, const char *library, u32 nid, void *func, int dummy);

/**
 * Replace import function stub in a module with a function or dummy value.
 *
 * This function autodetects whether Syscalls are used or not, and if used, it
 * behaves differently depending on the value of `replace_syscall`. If true, the
 * function replaces syscall address to the address to the `func` in the syscall
 * table. I false, the function creates a new syscall and redirects the stub to
 * the `func` syscall; in this case, manually exporting in exports.exp is still
 * required for Syscalls to work.
 *
 * @note ARK CFW compatibility, similar to `sctrlHENHookImportByNID`
 *
 * @param mod - The module where to search the function
 * @param library - The library name
 * @param nid - The nid of the function
 * @param func - The function to replace the stub. If value is below 16bit max
 * value, this is interpreted as a dummy value
 *
 * @returns
 * - 0 if successful,
 * -1 if `mod` or `library` are NULL,
 * -2 if failed to find import by NID and fail to resolve that NID from older firmware version
 * -3 if failed to find import by NID after successful resolve to older firmware version
 *
 * @note Compat with ARK CFW
 */
int sctrlHookImportByNID(SceModule * mod, const char * library, u32 nid, void * func);

/**
 * Replace exported function in a module with a function or dummy value.
 *
 * This function autodetects whether Syscalls are used or not, and if used, it
 * behaves differently depending on the value of `replace_syscall`. If true, the
 * function replaces syscall address to the address to the `func` in the syscall
 * table. I false, the function creates a new syscall and redirects the stub to
 * the `func` syscall; in this case, manually exporting in exports.exp is still
 * required for Syscalls to work.
 *
 * @param mod - The module where to search the function
 * @param library - The library name
 * @param nid - The nid of the function
 * @param func - The function to replace the stub. If NULL, use dummy value
 * @param dummy/replace_syscall - If `func` is NULL, it is the dummy value to replace the stub.
 * 	If `func` is not NULL, it is interpreted as a boolean on how to handle in
 * the case of a syscall hook; `0` means new-syscall and redirect the stub, `>0`
 * means replace the syscall in the syscall table (that means that all all usages
 * of the replaced syscall will call `func`).
 *
 * @returns
 * - 0 if successful,
 * -1 if `mod` or `library` are NULL,
 * -2 if failed to find import by NID and fail to resolve that NID from older firmware version
 * -3 if failed to find import by NID after successful resolve to older firmware version
 * -4 if failed to find syscall table
 * -5 if failed to find syscall in the syscall table
*/
int sctrlHENHookFunctionByNID(SceModule * mod, const char * library, u32 nid, void *func, int dummy);

/**
 * Gets the HEN version
 *
 * @returns - The HEN version
 *
 * HEN D / SE-C :  0x00000400
 * M33 : 0x00000700 | 0x00000800
 * TN / Adrenaline : 0x00001000
 * PRO : 0x00001003
 */
int sctrlHENGetVersion();

/**
 * Gets the HEN minor version
 *
 * @returns - The HEN minor version
 *
 * @note In practice, for Adrenaline, returns value for the release version of the CFW.
 */
int sctrlHENGetMinorVersion();

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
 * Checks if the system is a dev Toolkit.
 *
 * @returns 1 if system is a dev Toolkit, 0 otherwise.
 *
 * @note Compat with ARK CFW
*/
int sctrlHENIsToolKit();

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
 * Unlocks extra memory on partition 2 (user RAM).
 *
 * @param p2 - The size in MB for the user partition. The actual value is
 * ignored but must be > 24.
 *
 * @returns 0 on success, -1 if can't unlock (i.e. pops, vsh, function to unlock
 * not found), -2 if already unlocked, -3 if too late to unlock.
 */
int sctrlHENApplyMemory(u32 p2);

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
 *
 * @note Because only one handler function is handled by HEN, you should
 * call the previous function in your code.
 *
 * @example
 * STMOD_HANDLER previous = NULL;
 *
 * int OnModuleStart(SceModule *mod);
 *
 * void somepointofmycode()
 * {
 *		previous = sctrlHENSetStartModuleHandler(OnModuleStart);
 * }
 *
 * int OnModuleStart(SceModule *mod)
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
 * @note The above example should be compiled with the flag -fno-pic
 *			in order to avoid problems with gp register that may lead to a crash.
 *
*/
STMOD_HANDLER sctrlHENSetStartModuleHandler(STMOD_HANDLER handler);


/**
 * Read parameter from an SFO file or an EBOOT.PBP file.
 *
 * @param sfo_path The SFO file path, or NULL for EBOOT.PBP file
 * @param param_name The SFO parameter name
 * @param param_type The SFO parameter type
 * @param param_length The SFO parameter length
 * @param param_buf The buffer to write the the found SFO parameter
 *
 * @returns Returns `0` if parameter was found, `<0` on error
 */
int sctrlGetSfoPARAM(const char* sfo_path, const char* param_name, u16* param_type, u32* param_length, void* param_buf);

/**
 * Get SFO param from currently running game/app.
 *
 * @param param_name The SFO parameter name
 * @param param_type The SFO parameter type
 * @param param_length The SFO parameter length
 * @param param_buf The buffer to write the the found SFO parameter
 *
 * @returns Returns `0` if parameter was found, `<0` on error
 */
int sctrlGetInitPARAM(const char* param_name, u16* param_type, u32* param_length, void* param_buf);

/**
 * Patch module by offset
 *
 * @param modname - module name
 * @param inst  - instruction
 * @param offset - module patch offset
 *
 * @return < 0 on error
 *
 * @note Compat with PRO, ARK
 */
int sctrlPatchModule(char *modname, u32 inst, u32 offset);

/**
 * Get module text address
 *
 * @param modname - module name
 *
 * @return text address, or 0 if not found
 *
 * @note Compat with PRO, ARK
 */
u32 sctrlModuleTextAddr(char *modname);

/**
 * Flush/Cleans Instruction and Data Caches
 */
void sctrlFlushCache(void);

/**
 * Finds the UID of the thread with a given `name`.
 *
 * @param name The name of the thread
 *
 * @returns The UID of the thread on success, `< 0` on error or not found.
 */
SceUID sctrlGetThreadUIDByName(const char* name);


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

/** Use `sctrlLZ4Decompress` instead */
int LZ4_decompress_fast(const char* source, char* dest, int outputSize);

/** Use `sctrlLzoDecompress` instead */
int lzo1x_decompress(void* source, unsigned src_len, void* dest, unsigned* dst_len, void*);

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

/**
 * @note Compat with ARK
 */
u32 sctrlHENFakeDevkitVersion();

#endif // __USER__

// KERNEL ONLY
#ifdef __KERNEL__

#include <pspkernel.h>
#include <pspinit.h>

#include <rebootexconfig.h>

/**
 * Loads a module on next reboot. Only kernel mode.
 *
 * @param module_after - The path of the module which is loaded after the module to be loaded.
   The module passed to this function will be loaded just before that module.
 * @param buf - The buffer containing the module - Don't deallocate this one. It has to reside in kernel memory.
 * @param size - The size of the module
 * @param flags - The modes in which the module should be loaded, one of BootLoadFlags
 *
 * @example:
 * sctrlHENLoadModuleOnReboot("/kd/usersystemlib.prx", module_buffer, module_size, BOOTLOAD_GAME | BOOTLOAD_POPS | BOOTLOAD_UMDEMU);
 *
 * This will load the module contained in module_buffer just before /kd/usersystemlib.prx in the next reboot, if the mode of next reboot is game, pops or umdemu
 *
 * @note Don't use too early modules in first param like "/kd/init.prx" or "/kd/systemctrl.prx", or your module may not load properly
 * Only one module will be loaded on reboot with this function.
 * If this function is called many times, only the last one will be considered.
 * By making a module to load itself using this function, and calling
 * sctrlHENLoadModuleOnReboot on module_start, a prx can cause itself to be resident in the modes choosen by flags.
 * If all flags are selected, the module will stay resident until a psp shutdown, or until sctrlHENLoadModuleOnReboot is not called.
*/
void sctrlHENLoadModuleOnReboot(char *module_after, void *buf, int size, int flags);

/**
 * Changes a syscall to another function.
 *
 * This is achieved by modifying the syscall table to point to `newaddr` when
 * calling the original syscall. This means that every system call to the
 * original function (in any module) will execute the new function instead.
 *
 * @param addr - the address of the original function
 * @param newaddr - the address of the new function
 *
*/
void sctrlHENPatchSyscall(u32 addr, void *newaddr);

/**
 * Creates a syscall stub for the given `function` in user-memory.
 *
 * Creating a syscall stub this way allows a kernel program to inject syscall
 * calls in a user module without overwriting a existing syscall stub in the
 * module being modified.
 *
 * @note The given `function` must be exported as syscall for it to work.
 *
 * @param function The function pointer to the function to create the syscall stub
 *
 * @returns The syscall stub of the given function (> 0). Or zero if the function fails to create the stub.
 *
 * @attention Every call to this function allocates 8 bytes of memory in the
 * user RAM, which is also the available memory for the running application. So,
 * avoid excessive use of this function.
 */
u32 sctrlHENMakeSyscallStub(void *function);

typedef int (* HEN_REG_HOMEBREW_LOADER_HANDLER)(const char *path, int flags, SceKernelLMOption *option);
int sctrlHENRegisterHomebrewLoader(HEN_REG_HOMEBREW_LOADER_HANDLER handler);

/**
 * Get the rebootex configuration.
 *
 * @param config - A pointer to where the config will be copied, or NULL
 *
 * @returns A pointer to the global rebootex configuration.
 *
 * @note Compat with ARK
 */
RebootexConfig* sctrlHENGetRebootexConfig(RebootexConfig* config);

/**
 * Set custom start module handler
 *
 * It can be used to replace a system module
 *
 * @param func function pointer to register as custom start module handler
 *
 * @note - `func` returns -1 to ignore the module and load the original module. Or new modid if replace is done.
 *
 * @note - Compat with PRO and ARK
 */
void sctrlSetCustomStartModule(int (*func)(int modid, SceSize argsize, void *argp, int *modstatus, SceKernelSMOption *opt));

/**
 * Set custom start module handler
 *
 * It can be used to replace a system module
 *
 * @param func function pointer to register as custom start module handler
 *
 * @returns The previously set module handler, if any.
 *
 * @note - `func` returns -1 to ignore the module and load the original module. Or new modid if replace is done.
 *
 * @note - Compat with ME and ARK
 */
void* sctrlSetStartModuleExtra(int (*func)(int modid, SceSize argsize, void *argp, int *modstatus, SceKernelSMOption *opt));

/**
 * Get `sceInit` module text address
 *
 * @note - Only useful before `sceInit` exits
 *
 * @return text address, or 0 if not found
 *
 * @note - Compat with PRO and ARK
 */
u32 sctrlGetInitTextAddr(void);


void lowerString(char *orig, char *ret, int strSize);
int strncasecmp(const char *s1, const char *s2, SceSize n);
int strcasecmp(const char *s1, const char *s2);
char *strncat(char *dest, const char *src, SceSize count);
SceSize strncat_s(char *dest, SceSize numberOfElements, const char *src, SceSize count);
SceSize strncpy_s(char *dest, SceSize numberOfElements, const char *src, SceSize count);

#endif // __KERNEL__

#endif // __SCTRLLIBRARY_H__