/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * pspmodulemgr.h - Prototypes to manage modules.
 *
 * Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
 * Copyright (c) 2005 James Forshaw <tyranid@gmail.com>
 * Copyright (c) 2005 John Kelley <ps2dev@kelley.ca>
 *
 * $Id: pspmodulemgr.h 1148 2005-10-12 19:08:27Z tyranid $
 */

/* Note: Some of the structures, types, and definitions in this file were
   extrapolated from symbolic debugging information found in the Japanese
   version of Puzzle Bobble. */

#ifndef __MODLOAD_H__
#define __MODLOAD_H__

#include <pspkerneltypes.h>

/** @defgroup ModuleMgr Module Manager Library
  * This module contains the imports for the kernel's module management routines.
  */

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup ModuleMgr Module Manager Library */
/*@{*/

#define PSP_MEMORY_PARTITION_KERNEL 1
#define PSP_MEMORY_PARTITION_USER 2

typedef struct SceKernelLMOption {
	SceSize 		size;
	SceUID 			mpidtext;
	SceUID 			mpiddata;
	unsigned int 	flags;
	char 			position;
	char 			access;
	char 			creserved[2];
} SceKernelLMOption;

typedef struct SceKernelSMOption {
	SceSize 		size;
	SceUID 			mpidstack;
	SceSize 		stacksize;
	int 			priority;
	unsigned int 	attribute;
} SceKernelSMOption;


/**
  * Load a module.
  * @note This function restricts where it can load from (such as from flash0)
  * unless you call it in kernel mode. It also must be called from a thread.
  *
  * @param path - The path to the module to load.
  * @param flags - Unused, always 0 .
  * @param option  - Pointer to a mod_param_t structure. Can be NULL.
  *
  * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
  *
  * @attention Needs to link to `pspmodulemgr_user` or `pspmodulemgr_kernel` stub.
  */
SceUID sceKernelLoadModule(const char *path, int flags, SceKernelLMOption *option);

/**
 * Load a module from the given file UID.
 *
 * @param fid - The module's file UID.
 * @param flags - Unused, always 0.
 * @param option - Pointer to an optional ::SceKernelLMOption structure.
 *
 * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
 *
 * @attention Needs to link to `pspmodulemgr_user` or `pspmodulemgr_kernel` stub.
 */
SceUID sceKernelLoadModuleByID(SceUID fid, int flags, SceKernelLMOption *option);

/**
 * Load a module from a buffer using the USB/WLAN API.
 *
 * Can only be called from kernel mode, or from a thread that has attributes of 0xa0000000.
 *
 * @param bufsize - Size (in bytes) of the buffer pointed to by buf.
 * @param buf - Pointer to a buffer containing the module to load.  The buffer must reside at an
 *              address that is a multiple to 64 bytes.
 * @param flags - Unused, always 0.
 * @param option - Pointer to an optional ::SceKernelLMOption structure.
 *
 * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
 *
 * @attention Needs to link to `pspmodulemgr_user` or `pspmodulemgr_kernel` stub.
 */
SceUID sceKernelLoadModuleBufferUsbWlan(SceSize bufsize, void *buf, int flags, SceKernelLMOption *option);

/**
  * Start a loaded module.
  *
  * @param modid - The ID of the module returned from LoadModule.
  * @param argsize - Length of the args.
  * @param argp - A pointer to the arguments to the module.
  * @param status - Returns the status of the start.
  * @param option - Pointer to an optional ::SceKernelSMOption structure.
  *
  * @return ??? on success, otherwise one of ::PspKernelErrorCodes.
  *
  * @attention Needs to link to `pspmodulemgr_user` or `pspmodulemgr_kernel` stub.
  */
int sceKernelStartModule(SceUID modid, SceSize argsize, void *argp, int *status, SceKernelSMOption *option);

/**
 * Stop a running module.
 *
 * @param modid - The UID of the module to stop.
 * @param argsize - The length of the arguments pointed to by argp.
 * @param argp - Pointer to arguments to pass to the module's module_stop() routine.
 * @param status - Return value of the module's module_stop() routine.
 * @param option - Pointer to an optional ::SceKernelSMOption structure.
 *
 * @returns ??? on success, otherwise one of ::PspKernelErrorCodes.
 *
 * @attention Needs to link to `pspmodulemgr_user` or `pspmodulemgr_kernel` stub.
 */
int sceKernelStopModule(SceUID modid, SceSize argsize, void *argp, int *status, SceKernelSMOption *option);

/**
 * Unload a stopped module.
 *
 * @param modid - The UID of the module to unload.
 *
 * @returns ??? on success, otherwise one of ::PspKernelErrorCodes.
 *
 * @attention Needs to link to `pspmodulemgr_user` or `pspmodulemgr_kernel` stub.
 */
int sceKernelUnloadModule(SceUID modid);

/**
 * Stop and unload the current module.
 *
 * @param unknown - Unknown (I've seen 1 passed).
 * @param argsize - Size (in bytes) of the arguments that will be passed to module_stop().
 * @param argp - Pointer to arguments that will be passed to module_stop().
 *
 * @return ??? on success, otherwise one of ::PspKernelErrorCodes.
 *
 * @attention Needs to link to `pspmodulemgr_user` or `pspmodulemgr_kernel` stub.
 */
int sceKernelSelfStopUnloadModule(int unknown, SceSize argsize, void *argp);


typedef struct SceKernelModuleInfo {
	SceSize 		size;
	char 			nsegment;
	char 			reserved[3];
	int 			segmentaddr[4];
	int 			segmentsize[4];
	unsigned int 	entry_addr;
	unsigned int 	gp_value;
	unsigned int 	text_addr;
	unsigned int 	text_size;
	unsigned int 	data_size;
	unsigned int 	bss_size;
	/* The following is only available in the v1.5 firmware and above,
	   but as sceKernelQueryModuleInfo is broken in v1.0 is doesn't matter ;) */
	unsigned short  attribute;
	unsigned char   version[2];
	char            name[28];
} SceKernelModuleInfo;

/**
  * Query the information about a loaded module from its UID.
  * @note This fails on v1.0 firmware (and even it worked has a limited structure)
  * so if you want to be compatible with both 1.5 and 1.0 (and you are running in
  * kernel mode) then call this function first then ::pspSdkQueryModuleInfoV1
  * if it fails, or make separate v1 and v1.5+ builds.
  *
  * @param modid - The UID of the loaded module.
  * @param info - Pointer to a ::SceKernelModuleInfo structure.
  *
  * @return 0 on success, otherwise one of ::PspKernelErrorCodes.
  *
  * @attention Needs to link to `pspmodulemgr_user` or `pspmodulemgr_kernel` stub.
  */
int sceKernelQueryModuleInfo(SceUID modid, SceKernelModuleInfo *info);

/**
  * Get a list of module IDs. NOTE: This is only available on 1.5 firmware
  * and above. For V1 use ::pspSdkGetModuleIdList.
  *
  * @param readbuf - Buffer to store the module list.
  * @param readbufsize - Number of elements in the readbuffer.
  * @param idcount - Returns the number of module ids
  *
  * @return >= 0 on success
  *
  * @attention Needs to link to `pspmodulemgr_user` or `pspmodulemgr_kernel` stub.
  */
int sceKernelGetModuleIdList(SceUID *readbuf, int readbufsize, int *idcount);


#ifdef __USER__

/**
  * Load a module from MS.
  * @note This function restricts what it can load, e.g. it wont load plain executables.
  *
  * @param path - The path to the module to load.
  * @param flags - Unused, set to 0.
  * @param option  - Pointer to a mod_param_t structure. Can be NULL.
  *
  * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
  *
  * @attention Needs to link to `pspmodulemgr_user` stub.
  */
SceUID sceKernelLoadModuleMs(const char *path, int flags, SceKernelLMOption *option);

/**
  * Load a module protected by DRM...
  *
  * @param path - The path to the module to load.
  * @param flags - Unused, always 0 .
  * @param option  - Pointer to a mod_param_t structure. Can be NULL.
  *
  * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
  *
  * @attention Needs to link to `pspmodulemgr_user` stub.
  */
SceUID sceKernelLoadModuleDNAS(const char *path, int flags, SceKernelLMOption *option);

/**
 * Stop and unload the current module with the specified exit status code
 *
 * @param exitcode - The exitcode for the module
 * @param argsize - Size (in bytes) of the arguments that will be passed to module_stop().
 * @param argp - Poitner to arguments that will be passed to module_stop().
 * @param status - Return value from module_stop().
 * @param option - Pointer to an optional ::SceKernelSMOption structure.
 *
 * @returns ??? on success, otherwise one of ::PspKernelErrorCodes.
 *
 * @attention Needs to link to `pspmodulemgr_user` stub.
 */
int	sceKernelStopUnloadSelfModule(int exitcode, SceSize argsize, void *argp, int *status, SceKernelSMOption *option);

#endif // __USER__

#ifdef __KERNEL__

/**
 * Stop and unload the current module.
 *
 * @param argsize - Size (in bytes) of the arguments that will be passed to module_stop().
 * @param argp - Poitner to arguments that will be passed to module_stop().
 * @param status - Return value from module_stop().
 * @param option - Pointer to an optional ::SceKernelSMOption structure.
 *
 * @returns ??? on success, otherwise one of ::PspKernelErrorCodes.
 *
 * @attention Needs to link to `pspmodulemgr_kernel` stub.
 */
int sceKernelStopUnloadSelfModule(SceSize argsize, void *argp, int *status, SceKernelSMOption *option);

#if defined(_PSP_FW_VERSION) && _PSP_FW_VERSION < 103

/**
  * Gets the current module list.
  *
  * @param readbufsize - The size of the read buffer.
  * @param readbuf     - Pointer to a buffer to store the IDs
  *
  * @return < 0 on error.
  *
  * @attention Needs to link to `pspmodulemgr_kernel` stub.
  */
int sceKernelGetModuleList(int readbufsize, SceUID *readbuf);

/**
  * Get the number of loaded modules.
  *
  * @return The number of loaded modules.
  *
  * @attention Needs to link to `pspmodulemgr_kernel` stub.
  */
int sceKernelModuleCount(void);

#endif // defined(_PSP_FW_VERSION) && _PSP_FW_VERSION < 103

/**
 * Load a module from a buffer.
 *
 * @param buf - Pointer to a buffer containing the module to load.  The buffer must reside at an
 *              address that is a multiple to 64 bytes.
 * @param bufsize - Size (in bytes) of the buffer pointed to by buf.
 * @param flags - Unused, always 0.
 * @param option - Pointer to an optional ::SceKernelLMOption structure.
 *
 * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
 *
 * @attention Needs to link to `pspmodulemgr_kernel` stub.
 */
SceUID sceKernelLoadModuleBufferForKernel(void *buf, SceSize bufsize, int flags, SceKernelLMOption *option);

/**
 * Alias for `sceKernelLoadModuleBufferForKernel`
 *
 * @attention Needs to link to `pspmodulemgr_kernel` stub.
 */
SceUID sceKernelLoadModuleBuffer(void *buf, SceSize bufsize, int flags, SceKernelLMOption *option);

/**
 * Load a module from a buffer with the USB/WLAN apitype (0x130).
 *
 * @param bufsize - Size (in bytes) of the buffer pointed to by buf.
 * @param buf - Pointer to a buffer containing the module to load.  The buffer must reside at an
 *              address that is a multiple to 64 bytes.
 * @param flags - Unused, always 0.
 * @param option - Pointer to an optional ::SceKernelLMOption structure.
 *
 * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
 *
 * @attention Needs to link to `pspmodulemgr_kernel` stub.
 */
SceUID sceKernelLoadModuleBufferForLoadExecBufferVSHUsbWlan(SceSize bufsize, void *buf, int flags, SceKernelLMOption *option);

/**
 * Load a module from a buffer with the USB/WLAN Debug apitype (0x131).
 *
 * @param bufsize - Size (in bytes) of the buffer pointed to by buf.
 * @param buf - Pointer to a buffer containing the module to load.  The buffer must reside at an
 *              address that is a multiple to 64 bytes.
 * @param flags - Unused, always 0.
 * @param option - Pointer to an optional ::SceKernelLMOption structure.
 *
 * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
 *
 * @attention Needs to link to `pspmodulemgr_kernel` stub.
 */
SceUID sceKernelLoadModuleBufferForLoadExecBufferVSHUsbWlanDebug(SceSize bufsize, void *buf, int flags, SceKernelLMOption *option);

/**
 * Alias for `sceKernelLoadModuleBufferForLoadExecBufferVSHUsbWlanDebug`.
 *
 * @attention Needs to link to `pspmodulemgr_kernel` stub.
 */
SceUID sceKernelLoadModuleBufferUsbWlanDebug(SceSize bufsize, void *buf, int flags, SceKernelLMOption *option);

/**
 * Load a module from a buffer with the Exit VSH Kernel apitype (0x200).
 *
 * @param buf - Pointer to a buffer containing the module to load.  The buffer must reside at an
 *              address that is a multiple to 64 bytes.
 * @param flags - Unused, always 0.
 * @param option - Pointer to an optional ::SceKernelLMOption structure.
 * @param unk - Unknown
 *
 * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
 *
 * @attention Needs to link to `pspmodulemgr_kernel` stub.
 */
SceUID sceKernelLoadModuleBufferForExitVSHKernel(void *buf, int flags, SceKernelLMOption *option, int unk);

/**
 * Load a module from a buffer with Exit Game apitype (0x210).
 *
 * @param buf - Pointer to a buffer containing the module to load.  The buffer must reside at an
 *              address that is a multiple to 64 bytes.
 * @param flags - Unused, always 0.
 * @param option - Pointer to an optional ::SceKernelLMOption structure.
 * @param unk - Unknown
 *
 * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
 *
 * @attention Needs to link to `pspmodulemgr_kernel` stub.
 */
SceUID sceKernelLoadModuleBufferForExitGame(void *buf, int flags, SceKernelLMOption *option, int unk);

/**
 * Load a module from a buffer with Exit VSH apitype (0x220).
 *
 * @param buf - Pointer to a buffer containing the module to load.  The buffer must reside at an
 *              address that is a multiple to 64 bytes.
 * @param flags - Unused, always 0.
 * @param option - Pointer to an optional ::SceKernelLMOption structure.
 * @param unk - Unknown
 *
 * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
 *
 * @attention Needs to link to `pspmodulemgr_kernel` stub.
 */
SceUID sceKernelLoadModuleBufferForExitVSHVSH(void *buf, int flags, SceKernelLMOption *option, int unk);

/**
 * Load a module from a buffer with Reboot Kernel apitype (0x300)
 *
 * @param buf - Pointer to a buffer containing the module to load.  The buffer must reside at an
 *              address that is a multiple to 64 bytes.
 * @param flags - Unused, always 0.
 * @param option - Pointer to an optional ::SceKernelLMOption structure.
 * @param unk - Unknown
 *
 * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
 *
 * @attention Needs to link to `pspmodulemgr_kernel` stub.
 */
SceUID sceKernelLoadModuleBufferForRebootKernel(void *buf, int flags, SceKernelLMOption *option, int unk);

/**
 * Load a module from a buffer with the VSH apitype (0x021).
 *
 * @param bufsize - Size (in bytes) of the buffer pointed to by buf.
 * @param buf - Pointer to a buffer containing the module to load.  The buffer must reside at an
 *              address that is a multiple to 64 bytes.
 * @param flags - Unused, always 0.
 * @param option - Pointer to an optional ::SceKernelLMOption structure.
 *
 * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
 *
 * @attention Needs to link to `pspmodulemgr_kernel` stub.
 */
SceUID sceKernelLoadModuleBufferVSH(void *buf, int flags, SceKernelLMOption *option);

/**
 * Load a module from a buffer with the Boot Init BTCNF apitype (0x051).
 *
 * @param bufsize - Size (in bytes) of the buffer pointed to by buf.
 * @param buf - Pointer to a buffer containing the module to load.  The buffer must reside at an
 *              address that is a multiple to 64 bytes.
 * @param flags - Unused, always 0.
 * @param option - Pointer to an optional ::SceKernelLMOption structure.
 *
 * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
 *
 * @attention Needs to link to `pspmodulemgr_kernel` stub.
 */
SceUID sceKernelLoadModuleBufferBootInitBtcnf(int bufsize, void *buf, int flags, SceKernelLMOption *option);

/**
 * Load a module from a buffer with the Boot Init Config apitype (0x052).
 *
 * @param bufsize - Size (in bytes) of the buffer pointed to by buf.
 * @param buf - Pointer to a buffer containing the module to load.  The buffer must reside at an
 *              address that is a multiple to 64 bytes.
 * @param flags - Unused, always 0.
 * @param option - Pointer to an optional ::SceKernelLMOption structure.
 *
 * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
 *
 * @attention Needs to link to `pspmodulemgr_kernel` stub.
 */
SceUID sceKernelLoadModuleBufferBootInitConfig(int bufsize, void *buf, int flags, SceKernelLMOption *option);

/**
  * Load a module with the Deci apitype (0x070).
  *
  * @param path - The path to the module to load.
  * @param flags - Unused, always 0 .
  * @param option  - Pointer to a mod_param_t structure. Can be NULL.
  *
  * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
  *
  * @attention Needs to link to `pspmodulemgr_kernel` stub.
  */
SceUID sceKernelLoadModuleDeci(const char *path, int flags, SceKernelLMOption *option);

/**
  * Load a module with the VSH apitype (0x020).
  *
  * @param path - The path to the module to load.
  * @param flags - Unused, always 0 .
  * @param option  - Pointer to a mod_param_t structure. Can be NULL.
  *
  * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
  *
  * @attention Needs to link to `pspmodulemgr_kernel` stub.
  */
SceUID sceKernelLoadModuleVSH(const char *path, int flags, SceKernelLMOption *option);

/**
  * Load the executable of a disc (EBOOT.BIN) (0x120)
  *
  * @param path - The path to the module to load.
  * @param flags - Unused, always 0 .
  * @param option  - Pointer to a mod_param_t structure. Can be NULL.
  *
  * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
  *
  * @attention Needs to link to `pspmodulemgr_kernel` stub.
  */
SceUID sceKernelLoadModuleForLoadExecVSHDisc(const char *path, int flags, SceKernelLMOption *option);

/**
 * Alias for `sceKernelLoadModuleForLoadExecVSHDisc`.
 *
 * @attention Needs to link to `pspmodulemgr_kernel` stub.
 */
SceUID sceKernelLoadModuleDisc(const char *path, int flags, SceKernelLMOption *option);

/**
  * Load the updater executable of a disc (UPDATE/EBOOT.BIN) (0x121)
  *
  * @param path - The path to the module to load.
  * @param flags - Unused, always 0 .
  * @param option  - Pointer to a mod_param_t structure. Can be NULL.
  *
  * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
  *
  * @attention Needs to link to `pspmodulemgr_kernel` stub.
  */
SceUID sceKernelLoadModuleForLoadExecVSHDiscUpdater(const char *path, int flags, SceKernelLMOption *option);

/**
 * Alias for `sceKernelLoadModuleForLoadExecVSHDiscUpdater`
 *
 * @attention Needs to link to `pspmodulemgr_kernel` stub.
 */
SceUID sceKernelLoadModuleDiscUpdater(const char *path, int flags, SceKernelLMOption *option);

/**
  * Load the executable of a disc for debug in a toolkit (EBOOT.BIN) (0x122)
  *
  * @param path - The path to the module to load.
  * @param flags - Unused, always 0 .
  * @param option  - Pointer to a mod_param_t structure. Can be NULL.
  *
  * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
  *
  * @attention Needs to link to `pspmodulemgr_kernel` stub.
  */
SceUID sceKernelLoadModuleForLoadExecVSHDiscDebug(const char *path, int flags, SceKernelLMOption *option);

/**
 * Alias for `sceKernelLoadModuleForLoadExecVSHDiscDebug`
 *
 * @attention Needs to link to `pspmodulemgr_kernel` stub.
 */
SceUID sceKernelLoadModuleDiscDebug(const char *path, int flags, SceKernelLMOption *option);

/**
  * Load the executable of a disc image (0x123)
  *
  * @param path - The path to the module to load.
  * @param flags - Unused, always 0 .
  * @param option  - Pointer to a mod_param_t structure. Can be NULL.
  *
  * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
  *
  * @attention Needs to link to `pspmodulemgr_kernel` stub.
  */
SceUID sceKernelLoadModuleForLoadExecVSHDiscEmu(const char *path, int flags, SceKernelLMOption *option);

/**
 * Alias for `sceKernelLoadModuleForLoadExecVSHDiscEmu`
 *
 * @attention Needs to link to `pspmodulemgr_kernel` stub.
 */
SceUID sceKernelLoadModuleDiscEmu(const char *path, int flags, SceKernelLMOption *option);

/**
  * Load a PBP (used for updater pbp's) (0x140)
  *
  * @param path - The path to the module to load.
  * @param flags - Unused, always 0 .
  * @param option  - Pointer to a mod_param_t structure. Can be NULL.
  *
  * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
  * @attention Needs to link to `pspmodulemgr_kernel` stub.
  */
SceUID sceKernelLoadModuleForLoadExecVSHMs1(const char *path, int flags, SceKernelLMOption *option);

/**
 * Alias for `sceKernelLoadModuleForLoadExecVSHMs1`
 *
 * @attention Needs to link to `pspmodulemgr_kernel` stub.
 */
SceUID sceKernelLoadModuleMs1(const char *path, int flags, SceKernelLMOption *option);

/**
  * Load a PBP (used for non updater pbp's) (0x141)
  *
  * @param path - The path to the module to load.
  * @param flags - Unused, always 0 .
  * @param option  - Pointer to a mod_param_t structure. Can be NULL.
  *
  * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
  * @attention Needs to link to `pspmodulemgr_kernel` stub.
  */
SceUID sceKernelLoadModuleForLoadExecVSHMs2(int apitype, const char *path, int flags, SceKernelLMOption *option);

/**
 * Alias for `sceKernelLoadModuleForLoadExecVSHMs2`
 *
 * @attention Needs to link to `pspmodulemgr_kernel` stub.
 */
SceUID sceKernelLoadModuleMs2(int apitype, const char *path, int flags, SceKernelLMOption *option);

/**
  * Load a PBP (used in Ms3) (0x142)
  *
  * @param path - The path to the module to load.
  * @param flags - Unused, always 0 .
  * @param option  - Pointer to a mod_param_t structure. Can be NULL.
  *
  * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
  *
  * @attention Needs to link to `pspmodulemgr_kernel` stub.
  */
SceUID sceKernelLoadModuleForLoadExecVSHMs3(const char *path, int flags, SceKernelLMOption *option);

/**
 * Alias for `sceKernelLoadModuleForLoadExecVSHMs3`
 *
 * @attention Needs to link to `pspmodulemgr_kernel` stub.
 */
SceUID sceKernelLoadModuleMs3(const char *path, int flags, SceKernelLMOption *option);

/**
  * Load a PBP (used in Ms4 function to APPs (i.e. Comic Reader)) (0x143)
  *
  * @param path - The path to the module to load.
  * @param flags - Unused, always 0 .
  * @param option  - Pointer to a mod_param_t structure. Can be NULL.
  *
  * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
  *
  * @attention Needs to link to `pspmodulemgr_kernel` stub.
  */
SceUID sceKernelLoadModuleForLoadExecVSHMs4(const char *path, int flags, SceKernelLMOption *option);

/**
 * Alias for `sceKernelLoadModuleForLoadExecVSHMs4`
 *
 * @attention Needs to link to `pspmodulemgr_kernel` stub.
 */
SceUID sceKernelLoadModuleMs4(const char *path, int flags, SceKernelLMOption *option);

/**
  * Load a PBP (used in Ms5 function to load psx games) (0x144)
  *
  * @param path - The path to the module to load.
  * @param flags - Unused, always 0 .
  * @param option  - Pointer to a mod_param_t structure. Can be NULL.
  *
  * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
  *
  * @attention Needs to link to `pspmodulemgr_kernel` stub.
  */
SceUID sceKernelLoadModuleForLoadExecVSHMs5(const char *path, int flags, SceKernelLMOption *option);

/**
 * Alias for `sceKernelLoadModuleForLoadExecVSHMs5`
 *
 * @attention Needs to link to `pspmodulemgr_kernel` stub.
 */
SceUID sceKernelLoadModuleMs5(const char *path, int flags, SceKernelLMOption *option);

/**
  * Load a PBP (used in Ms6 function to load ???) (0x145)
  *
  * @param path - The path to the module to load.
  * @param flags - Unused, always 0 .
  * @param option  - Pointer to a mod_param_t structure. Can be NULL.
  *
  * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
  *
  * @attention Needs to link to `pspmodulemgr_kernel` stub.
  */
SceUID sceKernelLoadModuleForLoadExecVSHMs6(const char *path, int flags, SceKernelLMOption *option);

/**
 * Alias for `sceKernelLoadModuleForLoadExecVSHMs6`
 *
 * @attention Needs to link to `pspmodulemgr_kernel` stub.
 */
SceUID sceKernelLoadModuleMs6(const char *path, int flags, SceKernelLMOption *option);

/**
  * Load a module with a specific apitype
  *
  * @param apitype - The apitype. One of `SceFileExecApiType`
  * @param path - The path to the module to load.
  * @param flags - Unused, always 0 .
  * @param option  - Pointer to a mod_param_t structure. Can be NULL.
  *
  * @returns The UID of the loaded module on success, otherwise one of ::PspKernelErrorCodes.
  *
  * @attention Needs to link to `pspmodulemgr_kernel` stub.
  */
SceUID sceKernelLoadModuleForLoadExecForUser(int apitype, const char *path, int flags, SceKernelLMOption *option);

/**
 * Alias for `sceKernelLoadModuleForLoadExecForUser`
 *
 * @attention Needs to link to `pspmodulemgr_kernel` stub.
 */
SceUID sceKernelLoadModuleWithApitype2(int apitype, const char *path, int flags, SceKernelLMOption *option);

/**
 * Stop and unload the current module with the specified exit status code
 *
 * @param exitcode - The exitcode for the module
 * @param argsize - Size (in bytes) of the arguments that will be passed to module_stop().
 * @param argp - Poitner to arguments that will be passed to module_stop().
 * @param status - Return value from module_stop().
 * @param option - Pointer to an optional ::SceKernelSMOption structure.
 *
 * @returns ??? on success, otherwise one of ::PspKernelErrorCodes.
 *
 * @attention Needs to link to `pspmodulemgr_kernel` stub.
 */
int	sceKernelStopUnloadSelfModuleWithStatus(int exitcode, SceSize argsize, void *argp, int *status, SceKernelSMOption *option);

#endif // __KERNEL__

/*@}*/

#ifdef __cplusplus
}
#endif

#endif
