/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * pspsysmem.h - Interface to the system memory manager.
 *
 * Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
 *
 * $Id: pspsysmem.h 1961 2006-07-04 04:14:56Z jim $
 */

/* Note: Some of the structures, types, and definitions in this file were
   extrapolated from symbolic debugging information found in the Japanese
   version of Puzzle Bobble. */

#ifndef PSPSYSMEM_H
#define PSPSYSMEM_H

#include <pspkerneltypes.h>

/** @defgroup SysMem System Memory Manager
  * This module contains routines to manage heaps of memory.
  */

/** @addtogroup SysMem System Memory Manager */
/*@{*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _PspSysmemPartitionInfo {
	SceSize size;
	unsigned int startaddr;
	unsigned int memsize;
	unsigned int attr;
} PspSysmemPartitionInfo;

enum PspModel {
	PSP_MODEL_STANDARD = 0,
	PSP_MODE_SLIM_AND_LITE = 1
};

/** Structure of a UID control block */
typedef struct _uidControlBlock {
	struct _uidControlBlock *parent;
	struct _uidControlBlock *nextChild;
	struct _uidControlBlock *type;   //(0x8)
	u32 UID;					//(0xC)
	char *name;					//(0x10)
	unsigned char unk;
	unsigned char size;			// Size in words
	short attribute;
	struct _uidControlBlock *nextEntry;
} __attribute__((packed)) uidControlBlock;

/** Specifies the type of allocation used for memory blocks. */
enum PspSysMemBlockTypes {
	/** Allocate from the lowest available address. */
	PSP_SMEM_Low = 0,
	/** Allocate from the highest available address. */
	PSP_SMEM_High,
	/** Allocate from the specified address. */
	PSP_SMEM_Addr
};

typedef struct SceGameInfo {
	u32 size; // 0
	u32 flags; // 4
	char umd_data_string[16]; // 8
	char expect_umd_data[16]; // 24
	char qtgp2[8]; // 40
	char qtgp3[16]; // 48
	u32 allow_replace_umd; // 64
	char game_id[16]; // 68
	u32 parental_level; // 84
	char vsh_version[8]; // 88
	u32 umd_cache_on; // 96
	u32 compiled_sdk_version; // 100
	u32 compiler_version; // 104
	u32 dnas; // 108
	u32 utility_location; // 112
	char vsh_bootfilename[64]; // 116
	char gamedata_id[16]; // 180
	char app_ver[8]; // 196
	char subscription_validity[8]; // 204 Maybe padding
	int bootable; // 212
	int opnssmp_ver; // 216
} SceGameInfo;

typedef int SceKernelSysMemAlloc_t;

/**
 * Allocate a memory block from a memory partition.
 *
 * @param partitionid - The UID of the partition to allocate from.
 * @param name - Name assigned to the new block.
 * @param type - Specifies how the block is allocated within the partition.  One of ::PspSysMemBlockTypes.
 * @param size - Size of the memory block, in bytes.
 * @param addr - If type is PSP_SMEM_Addr, then addr specifies the lowest address allocate the block from.
 *
 * @returns The UID of the new block, or if less than 0 an error.
 *
 * @attention Needs to link to `pspsysmem_user` or `pspsysmem_kernel` stub.
 */
SceUID sceKernelAllocPartitionMemory(SceUID partitionid, const char *name, int type, SceSize size, void *addr);

/**
 * Free a memory block allocated with ::sceKernelAllocPartitionMemory.
 *
 * @param blockid - UID of the block to free.
 *
 * @returns ? on success, less than 0 on error.
 *
 * @attention Needs to link to `pspsysmem_user` or `pspsysmem_kernel` stub.
 */
int sceKernelFreePartitionMemory(SceUID blockid);

/**
 * Get the address of a memory block.
 *
 * @param blockid - UID of the memory block.
 *
 * @returns The lowest address belonging to the memory block.
 *
 * @attention Needs to link to `pspsysmem_user` or `pspsysmem_kernel` stub.
 */
void * sceKernelGetBlockHeadAddr(SceUID blockid);


/**
 * Get the firmware version.
 *
 * @returns The firmware version.
 * 0x01000300 on v1.00 unit,
 * 0x01050001 on v1.50 unit,
 * 0x01050100 on v1.51 unit,
 * 0x01050200 on v1.52 unit,
 * 0x02000010 on v2.00/v2.01 unit,
 * 0x02050010 on v2.50 unit,
 * 0x02060010 on v2.60 unit,
 * 0x02070010 on v2.70 unit,
 * 0x02070110 on v2.71 unit.
 *
 * @attention Needs to link to `pspsysmem_user` or `pspsysmem_kernel` stub.
 */
int sceKernelDevkitVersion(void);

/**
 * Set the SDK version of the current application.
 * All licensed games seem to set this value in the crt0
 *
 * @param sdkversion - The sdkversion to set (e.g.: 0x02070110 in applicationc compiled for firmware 2.71)
 *
 * @returns 0
 *
 * @attention Needs to link to `pspsysmem_user` or `pspsysmem_kernel` stub.
 */
int sceKernelSetCompiledSdkVersion(int sdkversion);

/**
 * Get the SDK version of the current application, previously set with sceKernelSetCompiledSdkVersion
 *
 * @returns The sdk version
 *
 * @attention Needs to link to `pspsysmem_user` or `pspsysmem_kernel` stub.
 */
int sceKernelGetCompiledSdkVersion(void);

/**
 * Set the compiler version of the current application.
 *
 * All licensed games seem to set this value in the crt0
 *
 * @param version - The compiler version to set
 *
 * @returns 0
 *
 * @attention Needs to link to `pspsysmem_user` or `pspsysmem_kernel` stub.
 */
int sceKernelSetCompilerVersion(int version);

/**
 * Get the SDK version set with sceKernelSetCompiledSdkVersion().
 *
 * @return Version number, or 0 if unset.
 *
 * @attention Needs to link to `pspsysmem_user` or `pspsysmem_kernel` stub.
 */
int sceKernelGetCompiledSdkVersion(void);

#ifdef __USER__

/**
 * Get the total amount of free memory.
 *
 * @returns The total amount of free memory, in bytes.
 *
 * @attention Needs to link to `pspsysmem_user` stub.
 */
SceSize sceKernelTotalFreeMemSize(void);

/**
 * Get the size of the largest free memory block.
 *
 * @returns The size of the largest free memory block, in bytes.
 *
 * @attention Needs to link to `pspsysmem_user` stub.
 */
SceSize sceKernelMaxFreeMemSize(void);

/**
 * Set the version of the SDK with which the caller was compiled.
 * Version numbers are as for sceKernelDevkitVersion().
 *
 * @return 0 on success, < 0 on error.
 *
 * @attention Needs to link to `pspsysmem_user` stub.
 */
int sceKernelSetCompiledSdkVersion(int version);


#if defined(_PSP_FW_VERSION) && _PSP_FW_VERSION >= 150

/**
 * Kernel printf function.
 *
 * @param format - The format string.
 * @param ... - Arguments for the format string.
 *
 * @attention Needs to link to `pspsysmem_user` stub.
 */
void sceKernelPrintf(const char *format, ...) __attribute__((format(printf, 1, 2)));

#endif // defined(_PSP_FW_VERSION) && _PSP_FW_VERSION >= 150

#endif // __USER__


#ifdef __KERNEL__

/**
 * Get the compiler version of the current application, previously set with sceKernelSetCompilerVersion
 *
 * @returns The compiler version
 *
 * @attention Needs to link to `pspsysmem_kernel` stub.
 */
int sceKernelGetCompilerVersion(void);

/**
 * Query the partition information
 *
 * @param pid  - The partition id
 * @param info - Pointer to the ::PspSysmemPartitionInfo structure
 *
 * @returns 0 on success.
 *
 * @attention Needs to link to `pspsysmem_kernel` stub.
 */
int sceKernelQueryMemoryPartitionInfo(int pid, PspSysmemPartitionInfo *info);

/**
 * Get the total amount of free memory.
 *
 * @param pid - The partition id
 *
 * @returns The total amount of free memory, in bytes.
 *
 * @attention Needs to link to `pspsysmem_kernel` stub.
 */
SceSize sceKernelPartitionTotalFreeMemSize(int pid);

/**
 * Get the size of the largest free memory block.
 *
 * @param pid - The partition id
 *
 * @returns The size of the largest free memory block, in bytes.
 *
 * @attention Needs to link to `pspsysmem_kernel` stub.
 */
SceSize sceKernelPartitionMaxFreeMemSize(int pid);

/**
 * Set the protection of a block of ddr memory
 *
 * @param addr - Address to set protection on
 * @param size - Size of block
 * @param prot - Protection bitmask
 *
 * @return < 0 on error
 *
 * @attention Needs to link to `pspsysmem_kernel` stub.
 */
int sceKernelSetDdrMemoryProtection(void *addr, int size, int prot);

/**
 * Create a heap.
 *
 * @param partitionid - The UID of the partition where allocate the heap.
 * @param size - The size in bytes of the heap.
 * @param unk - Unknown, probably some flag or type, pass 1.
 * @param name - Name assigned to the new heap.
 *
 * @returns The UID of the new heap, or if less than 0 an error.
 *
 * @attention Needs to link to `pspsysmem_kernel` stub.
*/
SceUID sceKernelCreateHeap(SceUID partitionid, SceSize size, int unk, const char *name);

/**
 * Allocate a memory block from a heap.
 *
 * @param heapid - The UID of the heap to allocate from.
 * @param size - The number of bytes to allocate.
 *
 * @returns The address of the allocated memory block, or NULL on error.
 *
 * @attention Needs to link to `pspsysmem_kernel` stub.
*/
void *sceKernelAllocHeapMemory(SceUID heapid, SceSize size);

/**
 * Free a memory block allocated from a heap.
 *
 * @param heapid - The UID of the heap where block belongs.
 * @param block - The block of memory to free from the heap.
 *
 * @returns 0 on success, < 0 on error.
 *
 * @attention Needs to link to `pspsysmem_kernel` stub.
 */
int sceKernelFreeHeapMemory(SceUID heapid, void *block);

/**
 * Delete a heap.
 *
 * @param heapid - The UID of the heap to delete.
 *
 * @returns 0 on success, < 0 on error.
 *
 * @attention Needs to link to `pspsysmem_kernel` stub.
*/
int sceKernelDeleteHeap(SceUID heapid);

/**
 * Get the amount of free size of a heap, in bytes.
 *
 * @param heapid - The UID of the heap
 *
 * @returns the free size of the heap, in bytes. < 0 on error.
 *
 * @attention Needs to link to `pspsysmem_kernel` stub.
*/
SceSize sceKernelHeapTotalFreeSize(SceUID heapid);

/**
 * Get a UID control block
 *
 * @param uid - The UID to find
 * @param block - Pointer to hold the pointer to the block
 *
 * @return 0 on success
 *
 * @attention Needs to link to `pspsysmem_kernel` stub.
 */
int sceKernelGetUIDcontrolBlock(SceUID uid, uidControlBlock** block);

/**
 * Get a UID control block on a particular type
 *
 * @param uid - The UID to find
 * @param type - Pointer to the type UID block
 * @param block - Pointer to hold the pointer to the block
 *
 * @return 0 on success
 *
 * @attention Needs to link to `pspsysmem_kernel` stub.
 */
int sceKernelGetUIDcontrolBlockWithType(SceUID uid, uidControlBlock* type, uidControlBlock** block);

#if defined(_PSP_FW_VERSION) && _PSP_FW_VERSION >= 150

/**
 * Get the root of the UID tree (1.5+ only)
 *
 * @return Pointer to the UID tree root
 *
 * @attention Needs to link to `pspsysmem_kernel` stub.
 */
uidControlBlock* sceKernelGetUidmanCB(void);

/**
 * Gets the current status of the system.
 *
 * @returns The status of the system.
 *
 * @attention Needs to link to `pspsysmem_kernel` stub.
 */
int sceKernelGetSystemStatus();


#endif // defined(_PSP_FW_VERSION) && _PSP_FW_VERSION >= 150


/**
 * Gets the model of the PSP. (2.80+ ?)
 *
 * @returns one of `PspModel` values.
 *
 * @attention Needs to link to `pspsysmem_kernel` stub.
*/
int sceKernelGetModel(void);


/**
 * Gets the information of the game. (2.00+ ?)
 *
 * @returns Pointer to the game information on success. NULL otherwise.
 *
 * @attention Needs to link to `pspsysmem_kernel` stub.
 */
SceGameInfo *sceKernelGetGameInfo();


#if defined(_PSP_FW_VERSION) && _PSP_FW_VERSION <= 340

/**
 * Get the kernel to dump the internal memory table to Kprintf
 *
 * @attention Needs to link to `pspsysmem_kernel` stub.
 */
void sceKernelSysMemDump(void);

/**
 * Dump the list of memory blocks
 *
 * @attention Needs to link to `pspsysmem_kernel` stub.
 */
void sceKernelSysMemDumpBlock(void);

/**
 * Dump the tail blocks
 *
 * @attention Needs to link to `pspsysmem_kernel` stub.
 */
void sceKernelSysMemDumpTail(void);

#endif // defined(_PSP_FW_VERSION) && _PSP_FW_VERSION <= 340

#endif // __KERNEL__


#ifdef __cplusplus
}
#endif

/*@}*/

#endif /* PSPSYSMEM_H */
