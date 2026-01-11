/*
 * PSP Software Development Kit - https://github.com/pspdev
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * psploadexec.h - Process load and exit related functions.
 *
 * Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
 * Copyright (c) 2005 James Forshaw <tyranid@gmail.com>
 * Copyright (c) 2005 John Kelley <ps2dev@kelley.ca>
 *
 */

/* Note: Some of the structures, types, and definitions in this file were
   extrapolated from symbolic debugging information found in the Japanese
   version of Puzzle Bobble. */

#ifndef __LOADEXEC_H__
#define __LOADEXEC_H__
// Protect from double definition of mixing our modified header with pspsdk header
#define PSPLOADEXEC_KERNEL_H

/** @defgroup LoadExec LoadExec Library */

#ifdef __cplusplus
extern "C" {
#endif

#include <psptypes.h>
#include <pspkerneltypes.h>

/** @addtogroup LoadExec */

/**@{*/

/** Structure to pass to loadexec */
typedef struct SceKernelLoadExecParam {
	/** Size of the structure */
	SceSize size;
	/** Size of the arg string */
	SceSize args;
	/** Pointer to the arg string */
	void * argp;
	/** Encryption key ? */
	const char * key;
} SceKernelLoadExecParam;

/** Structure for LoadExecVSH* functions */
typedef struct SceKernelLoadExecVSHParam {
/** Size of the structure in bytes */
	SceSize size;
/** Size of the arguments string */
	SceSize args;
/** Pointer to the arguments strings */
	void * argp;
/** The key, usually "game", "updater" or "vsh" */
	const char * key;
/** The size of the vshmain arguments */
	u32 vshmain_args_size;
/** vshmain arguments that will be passed to vshmain after the program has exited */
	void *vshmain_args;
/** "/kd/pspbtcnf_game.txt" or "/kd/pspbtcnf.txt" if not supplied (max. 256 chars) */
	char *configfile;
/** An unknown string (max. 256 chars) probably used in 2nd stage of loadexec */
	u32 unk4;
/** unknown flag default value = 0x10000 */
	u32 unk5;
} SceKernelLoadExecVSHParam;

typedef struct {
	u32 api_type;
	s32 args;
	// Sometimes contains filename (args is then set to 0)
	void *argp;
	SceKernelLoadExecVSHParam *vsh_param;
	void *opt4;
	char *np_drm1;
	s32 np_drm2_1;
	s32 np_drm2_2;
	void *np_drm_arg;
} RunExecParams;

#ifdef __USER__
/**
 * Register callback
 *
 * @note By installing the exit callback the home button becomes active. However if sceKernelExitGame
 * is not called in the callback it is likely that the psp will just crash.
 *
 * @par Example:
 * @code
 * int exit_callback(void) { sceKernelExitGame(); }
 *
 * cbid = sceKernelCreateCallback("ExitCallback", exit_callback, NULL);
 * sceKernelRegisterExitCallback(cbid);
 * @endcode
 *
 * @param cbid Callback id
 * @return < 0 on error
 *
 * @attention Needs to link to `psploadexec_user` stub.
 */
int sceKernelRegisterExitCallback(int cbid);

/**
 * Exit game and go back to the PSP browser.
 *
 * @note You need to be in a thread in order for this function to work
 *
 * @attention Needs to link to `psploadexec_user` stub.
 */
void sceKernelExitGame(void);

/**
 * Exit game and go back to the PSP browser with a status value.
 *
 * @param status - The status value to use
 *
 * @note You need to be in a thread in order for this function to work
 *
 * @attention Needs to link to `psploadexec_user` stub.
 */
void sceKernelExitGameWithStatus(int status);

/**
 * Execute a new game executable, limited when not running in kernel mode.
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecParam structure, or NULL.
 *
 * @return < 0 on error, probably.
 *
 * @attention Needs to link to `psploadexec_user` stub.
 */
int sceKernelLoadExec(const char *file, SceKernelLoadExecParam *param);

#endif // __USER__

#ifdef __KERNEL__

#if defined(_PSP_FW_VERSION) && _PSP_FW_VERSION <= 151
/**
 * Executes a new executable from a buffer.
 *
 * @param bufsize - Size in bytes of the buffer pointed by buf.
 * @param buf - Pointer to a buffer containing the module to execute.
 * @param param - Pointer to a ::SceKernelLoadExecParam structure, or NULL.
 *
 * @returns < 0 on some errors.
 *
 * @attention Needs to link to `psploadexec_kernel` stub.
*/
int sceKernelLoadExecBufferPlain(SceSize bufsize, void *buf, SceKernelLoadExecParam *param);
#endif // defined(_PSP_FW_VERSION) && _PSP_FW_VERSION < 151


/**
 * Restart the vsh.
 *
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL
 *
 * @returns < 0 on some errors.
 *
 * @note - when called in game mode it will have the same effect that sceKernelExitGame
 *
 * @attention Needs to link to `psploadexec_kernel` stub.
*/
int sceKernelExitVSHVSH(SceKernelLoadExecVSHParam *param);

#if defined(_PSP_FW_VERSION) && _PSP_FW_VERSION >= 200
/**
 * Restart the vsh (to be used by a kernel module)
 *
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL
 *
 * @returns < 0 on some errors.
 *
 * @note - when called in game mode it will have the same effect that sceKernelExitGame
 * @note2: available since firmware 2.00.
 *
 * @attention Needs to link to `psploadexec_kernel` stub.
*/
int sceKernelExitVSHKernel(SceKernelLoadExecVSHParam *param);
#endif // defined(_PSP_FW_VERSION) && _PSP_FW_VERSION >= 200

/**
 * Executes a new executable from a disc.
 * It is the function used by the firmware to execute the EBOOT.BIN from a disc.
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors.
 *
 * @attention Needs to link to `psploadexec_kernel` stub.
*/
int sceKernelLoadExecVSHDisc(const char *file, SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a disc.
 * It is the function used by the firmware to execute an updater from a disc.
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors.
 *
 * @attention Needs to link to `psploadexec_kernel` stub.
*/
int sceKernelLoadExecVSHDiscUpdater(const char *file, SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a memory stick.
 * It is the function used by the firmware to execute an updater from a memory stick.
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors.
 *
 * @attention Needs to link to `psploadexec_kernel` stub.
*/
int sceKernelLoadExecVSHMs1(const char *file, SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a memory stick.
 * It is the function used by the firmware to execute games (and homebrew :P) from a memory stick.
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors.
 *
 * @attention Needs to link to `psploadexec_kernel` stub.
*/
int sceKernelLoadExecVSHMs2(const char *file, SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a memory stick.
 * It is the function used by the firmware to execute ... ?
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors.
 *
 * @attention Needs to link to `psploadexec_kernel` stub.
*/
int sceKernelLoadExecVSHMs3(const char *file, SceKernelLoadExecVSHParam *param);

#if defined(_PSP_FW_VERSION) && _PSP_FW_VERSION >= 300
/***
 * Executes a new executable from a memory stick.
 * It is the function used by the firmware to execute applications (i.e. Comic Reader)
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors.
 * @note - Available since firmware 3.00
 *
 * @attention Needs to link to `psploadexec_kernel` stub.
*/
int sceKernelLoadExecVSHMs4(const char *file, SceKernelLoadExecVSHParam *param);

/***
 * Executes a new executable from a memory stick.
 * It is the function used by the firmware to execute psx games
 *
 * @param file - The file to execute.
 * @param param - Pointer to a ::SceKernelLoadExecVSHParam structure, or NULL.
 *
 * @returns < 0 on some errors.
 * @note - Available since firmware 3.00
 *
 * @attention Needs to link to `psploadexec_kernel` stub.
*/
int sceKernelLoadExecVSHMs5(const char *file, SceKernelLoadExecVSHParam *param);
#endif // defined(_PSP_FW_VERSION) && _PSP_FW_VERSION >= 300


#endif // __KERNEL__


#ifdef __cplusplus
}
#endif

/**@}*/

#endif
