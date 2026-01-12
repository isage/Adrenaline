/*
	Adrenaline Kernel-User Bridge (KUBridge)
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

#ifndef __KULIBRARY__
#define __KULIBRARY__

#include <pspsdk.h>
#include <pspkernel.h>
#include <pspsysmem.h>
#include <pspsysmem_kernel.h>
#include <pspctrl.h>

// interface for passing arguments to kernel functions
typedef struct KernelCallArg {
    u32 arg1;
    u32 arg2;
    u32 arg3;
    u32 arg4;
    u32 arg5;
    u32 arg6;
    u32 arg7;
    u32 arg8;
    u32 arg9;
    u32 arg10;
    u32 arg11;
    u32 arg12;
    u32 ret1;
    u32 ret2;
} KernelCallArg;

/**
 * Functions to let user mode access certain functions only available in
 * kernel mode
*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Load a module using ModuleMgrForKernel.
 *
 * @param[in] path The path to the module to load.
 * @param flags Unused. Always `0`.
 * @param[in] option A pointer to a `SceKernelLMOption` structure. It can be `NULL`.
 *
 * @return The UID of the loaded module on success, otherwise one of `PspKernelErrorCodes`.
 */
SceUID kuKernelLoadModule(const char *path, int flags, SceKernelLMOption *option);


/**
 * Load a module with a specific apitype
 *
 * @param apitype The apitype.
 * @param[in] path The path to the module to load.
 * @param flags Unused. Always `0`.
 * @param[in] option  Pointer to a `SceKernelLMOption` structure. It can be `NULL`.
 *
 * @return The UID of the loaded module on success, otherwise one of `PspKernelErrorCodes`.
 */
SceUID kuKernelLoadModuleWithApitype2(int apitype, const char *path, int flags, SceKernelLMOption *option);

/**
 * Gets the API type.
 *
 * @return The API type in which the system has booted.
*/
int kuKernelInitApitype();

/**
 * Gets the filename of the executable to be launched after all modules of the API.
 *
 * @param[out] initfilename The buffer to receive the initfilename.
 *
 * @return `0` on success.
*/
int kuKernelInitFileName(char *initfilename);

/**
 * Gets the device in which the application was launched.
 *
 * @return The device code. One of `SceBootMediumType` (or `PSPBootFrom`) values.
*/
int kuKernelBootFrom();

/**
 * Get the key configuration in which the system has booted.
 *
 * @return The key configuration code, one of `SceApplicationType` (or `PSPKeyConfig`) values
*/
int kuKernelInitKeyConfig();

/**
 * Get the user level of the current thread.
 *
 * @return The user level, `< 0` on error.
 */
int kuKernelGetUserLevel(void);

/**
 * Set the protection of a block of ddr memory.
 *
 * @param[in] addr The address to set protection on.
 * @param size The size of the block to protect.
 * @param prot The protection bitmask.
 *
 * @return `< 0` on error.
 */
int kuKernelSetDdrMemoryProtection(void *addr, int size, int prot);

/**
 * Gets the model of the PSP from user mode.
 *
 * This function is available since 3.60 M33.
 * In previous version, use the kernel function sceKernelGetModel
 *
 * @return One of `PspModel` values
*/
int kuKernelGetModel(void);

/**
 * Find module by name.
 *
 * @param[in] modname The name of the module.
 * @param[out] mod A pointer to receive the module structure.
 *
 * @return `< 0` on error.
 */
int kuKernelFindModuleByName(char *modname, SceModule *mod);

/**
 * Find module by arbitrary memory address.
 *
 * @param[in] addr The memory address somewhere inside the module
 * @param[out] mod A pointer to receive the module structure.
 *
 * @return `< 0` on error.
 */
int kuKernelFindModuleByAddress(void *addr, SceModule *mod);

/**
 * Invalidate the entire instruction cache.
 */
void kuKernelIcacheInvalidateAll(void);

/**
 * Read 4 bytes from memory (with kernel memory access).
 *
 * @param[in] addr The address to read. It must have 4 bytes alignment.
 */
u32 kuKernelPeekw(void *addr);

/**
 * Write 4 bytes to memory (with kernel memory access).
 *
 * @param[in] addr The address to write. It must have 4 bytes alignment.
 */
void kuKernelPokew(void *addr, u32 value);

/**
 * memcpy (with kernel memory access).
 *
 * @param dest The destination address.
 * @param src The source address.
 * @param num The copy bytes count.
 *
 * @return The destination address.
 */
void *kuKernelMemcpy(void *dest, const void *src, size_t num);

/**
 * Call a kernel function with kernel privilege.
 *
 * @param[in] func_addr The kernel function address.
 * @param[inout] args The kernel arguments and return values
 *
 * @return `< 0` on error.
 */
int kuKernelCall(void *func_addr, KernelCallArg *args);

/**
 * Call a kernel function with kernel privilege and extended stack.
 *
 * @param[in] func_addr The kernel function address.
 * @param[inout] args The kernel arguments and return values.
 * @param stack_size The stack size to use.
 *
 * @return `< 0` on error.
 */
int kuKernelCallExtendStack(void *func_addr, KernelCallArg *args, int stack_size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __KULIBRARY__ */

