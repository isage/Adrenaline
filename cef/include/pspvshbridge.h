/*
 * File for sceVshBridge functions
 * Note: vshbridge lets vsh threads to call to some functions that
 * are kernel-exports only.
*/

#ifndef __VSHBRIDGE__
#define __VSHBRIDGE__

#include "psploadexec.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Executes a new executable from a buffer.
 *
 * @param bufsize The Size in bytes of the buffer pointed by buf.
 * @param buf A pointer to a buffer containing the module to execute.
 * @param param A pointer to a `SceKernelLoadExecParam` structure, or `NULL`.
 *
 * @return `< 0` on some errors.
*/
int vshKernelLoadExecBufferPlain(SceSize bufsize, void *buf, struct SceKernelLoadExecParam *param);

/**
 * Restart the vsh.
 *
 * @param unk Unknown, I haven't checked it. Set it to `NULL`.
 *
 * @return `< 0` on some errors.
 *
 * @note When called in game mode it will have the same effect that `sceKernelExitGame`.
 *
*/
int vshKernelExitVSHVSH(void *unk);

/**
 * Executes a new executable from a disc.
 *
 * It is the function used by the firmware to execute the `EBOOT.BIN` from a disc.
 *
 * @param file The file to execute.
 * @param param A pointer to a `SceKernelLoadExecVSHParam` structure, or `NULL`.
 *
 * @return `< 0` on some errors.
*/
int vshKernelLoadVSHDisc(const char *file, SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a disc.
 *
 * It is the function used by the firmware to execute an updater from a disc.
 *
 * @param file The file to execute.
 * @param param A pointer to a `SceKernelLoadExecVSHParam` structure, or `NULL`.
 *
 * @return `< 0` on some errors.
*/
int vshKernelLoadExecVSHDiscUpdater(const char *file, SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a memory stick.
 *
 * It is the function used by the firmware to execute an updater from a memory stick.
 *
 * @param file The file to execute.
 * @param param A pointer to a `SceKernelLoadExecVSHParam` structure, or `NULL`.
 *
 * @return < 0 on some errors.
*/
int vshKernelLoadExecVSHMs1(const char *file, SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a memory stick.
 *
 * It is the function used by the firmware to execute games (and homebrew :P) from a memory stick.
 *
 * @param file The file to execute.
 * @param param A pointer to a `SceKernelLoadExecVSHParam` structure, or `NULL`.
 *
 * @return `< 0` on some errors.
*/
int vshKernelLoadExecVSHMs2(const char *file, SceKernelLoadExecVSHParam *param);

/**
 * Executes a new executable from a memory stick.
 *
 * It is the function used by the firmware to execute ... ?
 *
 * @param file The file to execute.
 * @param param A pointer to a `SceKernelLoadExecVSHParam` structure, or `NULL`.
 *
 * @return `< 0` on some errors.
*/
int vshKernelLoadExecVSHMs3(const char *file, SceKernelLoadExecVSHParam *param);


/**
 * Performs a logical format in a flash partition.
 *
 * @param argc The number of parameters.
 * @param argv The parameters.
 *
 * @return `< 0` on error.
 */
int vshLflashFatfmtStartFatfmt(int argc, char *argv[]);

/**
 * Load a module.
 *
 * @param path The path to the module to load.
 * @param flags Unused, always `0`.
 * @param option  A pointer to a `SceKernelLMOption` structure. It can be `NULL`.
 *
 * @return The UID of the loaded module on success, otherwise one of `PspKernelErrorCodes`.
 */
SceUID vshKernelLoadModuleVSH(const char *path, int flags, SceKernelLMOption *option);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __VSHBRIDGE__ */
