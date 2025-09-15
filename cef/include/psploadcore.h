/*
 * PSP Software Development Kit - http://www.pspdev.org
 * -----------------------------------------------------------------------
 * Licensed under the BSD license, see LICENSE in PSPSDK root for details.
 *
 * psploadcore.h - Interface to LoadCoreForKernel.
 *
 * Copyright (c) 2005 Marcus R. Brown <mrbrown@ocgnet.org>
 * Copyright (c) 2005 James Forshaw <tyranid@gmail.com>
 * Copyright (c) 2005 John Kelley <ps2dev@kelley.ca>
 *
 * $Id: psploadcore.h 1095 2005-09-27 21:02:16Z jim $
 */

#ifndef PSPLOADCORE_H
#define PSPLOADCORE_H

#include <pspkerneltypes.h>

/** @defgroup LoadCore Interface to the LoadCoreForKernel library.
 */

#ifdef __cplusplus
extern "C" {
#endif

/** @addtogroup LoadCore Interface to the LoadCoreForKernel library. */
/*@{*/

#define SCE_KERNEL_MAX_MODULE_SEGMENT (4)

#ifndef __THREADMAN_H__
typedef s32 (*SceKernelThreadEntry)(SceSize args, void *argp);
#endif
typedef s32 (*SceKernelRebootBeforeForKernel)(void *arg1, s32 arg2, s32 arg3, s32 arg4);
typedef s32 (*SceKernelRebootPhaseForKernel)(s32 arg1, void *arg2, s32 arg3, s32 arg4);

/** The SceModule structure represents a loaded module in memory. */
typedef struct SceModule {
	/** Pointer to the next registered module. Modules are connected via a linked list. */
	struct SceModule* next; // 0
	/** The attributes of a module. One or more of ::SceModuleAttribute and ::SceModulePrivilegeLevel. */
	u16 attribute; // 4
	/**
	 * The version of the module. Consists of a major and minor part. There can be several modules
	 * loaded with the same name and version.
	 */
	u8 version[2]; // 6
	/** The module's name. There can be several modules loaded with the same name. */
	char modname[27]; // 8
	/** String terminator (always '\0'). */
	char terminal; // 0x23
	/**
	 * The status of the module. Contains information whether the module has been started, stopped,
	 * is a user module, etc.
	 */
	u16 mod_state;	// 0x24
	char padding[2]; // 0x26
	/** A secondary ID for the module. */
	SceUID sec_id;	// 0x28
	/** The module's UID. */
	SceUID mod_id; // 0x2C
	/** The thread ID of a user module. */
	SceUID user_mod_thid; // 0x30
	/** The ID of the memory block belonging to the module. */
	SceUID mem_id; // 0x34
	/** The ID of the TEXT segment's memory partition. */
	u32 mpid_text;	// 0x38
	/** The ID of the DATA segment's memory partition. */
	u32 mpid_data; // 0x3C
	/** Pointer to the first resident library entry table of the module. */
	void * ent_top; // 0x40
	/** The size of all resident library entry tables of the module. */
	SceSize ent_size; // 0x44
	/** Pointer to the first stub library entry table of the module. */
	void * stub_top; // 0x48
	/** The size of all stub library entry tables of the module. */
	SceSize stub_size; // 0x4C
	/**
	 * A pointer to the (required) module's start entry function. This function is executed during
	 * the module's startup.
	 */
	SceKernelThreadEntry module_start; // 0x50
	/**
	 * A pointer to the (required) module's stop entry function. This function is executed during
	 * the module's stopping phase.
	 */
	SceKernelThreadEntry module_stop; // 0x54
	/**
	 * A pointer to a module's Bootstart entry function. This function is probably executed after
	 * a reboot.
	 */
	SceKernelThreadEntry module_bootstart; // 0x58
	/**
	 * A pointer to a module's rebootBefore entry function. This function is probably executed
	 * before a reboot.
	 */
	SceKernelRebootBeforeForKernel module_reboot_before; // 0x5C
	/**
	 * A pointer to a module's rebootPhase entry function. This function is probably executed
	 * during a reboot.
	 */
	SceKernelRebootPhaseForKernel module_reboot_phase; // 0x60
	/**
	 * The entry address of the module. It is the offset from the start of the TEXT segment to the
	 * program's entry point.
	 */
	u32 entry_addr; // 0x64
	/** Contains the offset from the start of the TEXT segment of the program's GP register value. */
	u32 gp_value; // 0x68
	/** The start address of the TEXT segment. */
	u32 text_addr; // 0x6C
	/** The size of the TEXT segment. */
	u32 text_size; // 0x70
	/** The size of the DATA segment. */
	u32 data_size;	// 0x74
	/** The size of the BSS segment. */
	u32 bss_size; // 0x78
	/** The number of segments the module consists of. */
	u8 nsegment; // 0x7C
	/** Reserved. */
	u8 padding2[3]; // 0x7D
	/** An array containing the start address of each segment. */
	u32 segment_addr[4]; // 0x80
	/** An array containing the size of each segment. */
	SceSize segment_size[4]; // 0x90
	/** An array containing the alignment information of each segment. */
	u32 segment_align[4]; // 0xA0
	/** The priority of the module start thread. */
	s32 module_start_thread_priority; // 0xB0
	/** The stack size of the module start thread. */
	SceSize module_start_thread_stacksize; // 0xB4
	/** The attributes of the module start thread. */
	SceUInt module_start_thread_attr; // 0xB8
	/** The priority of the module stop thread. */
	s32 module_stop_thread_priority; // 0xBC
	/** The stack size of the module stop thread. */
	SceSize module_stop_thread_stacksize; // 0xC0
	/** The attributes of the module stop thread. */
	SceUInt module_stop_thread_attr; // 0xC4
	/** The priority of the module reboot before thread. */
	s32 module_reboot_before_thread_priority; // 0xC8
	/** The stack size of the module reboot before thread. */
	SceSize module_reboot_before_thread_stacksize; // 0xCC
	/** The attributes of the module reboot before thread. */
	SceUInt module_reboot_before_thread_attr; // 0xD0
	/** The value of the coprocessor 0's count register when the module is created. */
	u32 count_reg_val; // 0xD4
	/** The segment checksum of the module's segments. */
	u32 segment_checksum; // 0xD8
	/** TEXT segment checksum of the module. */
	u32 text_segment_checksum; // 0xDC
	/**
	 * Whether to compute the text segment checksum before starting the module (see prologue).
	 * If non-zero, the text segment checksum will be computed after the module's resident libraries
	 * have been registered, and its stub libraries have been linked.
	 */
	u32 compute_text_segment_checksum; // 0xE0
} SceModule;

typedef struct {
	char *name;
	void *buf;
	int size;
	int unk_12;
	int attr;
	int unk_20;
	int argSize;
	int argPartId;
} SceLoadCoreBootModuleInfo;

/** Defines a library and its exported functions and variables.  Use the len
	member to determine the real size of the table (size = len * 4). */
typedef struct SceLibraryEntryTable {
	/**The library's name. */
	const char *		libname;
	/** Library version. */
	unsigned char		version[2];
	/** Library attributes. */
	unsigned short		attribute;
	/** Length of this entry table in 32-bit WORDs. */
	unsigned char		len;
	/** The number of variables exported by the library. */
	unsigned char		vstubcount;
	/** The number of functions exported by the library. */
	unsigned short		stubcount;
	/** Pointer to the entry table; an array of NIDs followed by
		pointers to functions and variables. */
	void * entrytable;
} SceLibraryEntryTable;

/** Specifies a library and a set of imports from that library.  Use the len
	member to determine the real size of the table (size = len * 4). */
typedef struct SceLibraryStubTable {
	/* The name of the library we're importing from. */
	const char *		libname;
	/** Minimum required version of the library we want to import. */
	unsigned char		version[2];
	/* Import attributes. */
	unsigned short		attribute;
	/** Length of this stub table in 32-bit WORDs. */
	unsigned char		len;
	/** The number of variables imported from the library. */
	unsigned char		vstubcount;
	/** The number of functions imported from the library. */
	unsigned short		stubcount;
	/** Pointer to an array of NIDs. */
	unsigned int *		nidtable;
	/** Pointer to the imported function stubs. */
	void * stubtable;
	/** Pointer to the imported variable stubs. */
	void * vstubtable;
} SceLibraryStubTable;

typedef struct SceLoadCoreExecFileInfo {
	/** Unknown. */
	u32 unk0;
	/** The mode attribute of the executable file. One of ::SceExecFileModeAttr. */
	u32 mode_attr; //4
	/** The API type. */
	u32 api_type; //8
	/** Unknown. */
	u32 unk12;
	/** The size of the executable, including the ~PSP header. */
	SceSize exec_size; //16
	/** The maximum size needed for the decompression. */
	SceSize max_alloc_size; //20
	/** The memory ID of the decompression buffer. */
	SceUID decompression_mem_id; //24
	/** Pointer to the compressed module data. */
	void *file_base; //28
	/** Indicates the ELF type of the executable. One of ::SceExecFileElfType. */
	u32 elf_type; //32
	/** The start address of the TEXT segment of the executable in memory. */
	void *top_addr; //36
	/**
	 * The entry address of the module. It is the offset from the start of the TEXT segment to the
	 * program's entry point.
	 */
	u32 entry_addr; //40
	/** Unknown. */
	u32 unk44;
	/**
	 * The size of the largest module segment. Should normally be "textSize", but technically can
	 * be any other segment.
	 */
	SceSize largest_seg_size; //48
	/** The size of the TEXT segment. */
	SceSize text_size; //52
	/** The size of the DATA segment. */
	SceSize data_size; //56
	/** The size of the BSS segment. */
	SceSize bss_size; //60
	/** The memory partition of the executable. */
	u32 partition_id; //64
	/**
	 * Indicates whether the executable is a kernel module or not. Set to 1 for kernel module,
	 * 0 for user module.
	 */
	u32 is_kernel_mod; //68
	/**
	 * Indicates whether the executable is decrypted or not. Set to 1 if it is successfully decrypted,
	 * 0 for encrypted.
	 */
	u32 is_decrypted; //72
	/** The offset from the start address of the TEXT segment to the SceModuleInfo section. */
	u32 module_info_offset; //76
	/** The pointer to the module's SceModuleInfo section. */
	SceModuleInfo *module_info; //80
	/** Indicates whether the module is compressed or not. Set to 1 if it is compressed, otherwise 0.*/
	u32 is_compressed; //84
	/** The module's attributes. One or more of ::SceModuleAttribute and ::SceModulePrivilegeLevel. */
	u16 mod_info_attribute; //88
	/** The attributes of the executable file. One of ::SceExecFileAttr. */
	u16 exec_attribute; //90
	/** The size of the decompressed module, including its headers. */
	SceSize dec_size; //92
	/** Indicates whether the module is decompressed or not. Set to 1 for decompressed, otherwise 0. */
	u32 is_decompressed; //96
	/**
	 * Indicates whether the module was signChecked or not. Set to 1 for signChecked, otherwise 0.
	 * A signed module has a "mangled" executable header, in other words, the "~PSP" signature can't
	 * be seen.
	 */
	u32 is_sign_checked; //100
	/** Unknown. */
	u32 unk104;
	/** The size of the GZIP compression overlap. */
	SceSize overlap_size; //108
	/** Pointer to the first resident library entry table of the module. */
	void *exports_info; //112
	/** The size of all resident library entry tables of the module. */
	SceSize exports_size; //116
	/** Pointer to the first stub library entry table of the module. */
	void *imports_info; //120
	/** The size of all stub library entry tables of the module. */
	SceSize imports_size; //124
	/** Pointer to the string table section. */
	void *strtab_offset; //128
	/** The number of segments in the executable. */
	u8 num_segments; //132
	/** Reserved. */
	u8 padding[3]; //133
	/** An array containing the start address of each segment. */
	u32 segment_addr[SCE_KERNEL_MAX_MODULE_SEGMENT]; //136
	/** An array containing the size of each segment. */
	u32 segment_size[SCE_KERNEL_MAX_MODULE_SEGMENT]; //152
	/** The ID of the ELF memory block containing the TEXT, DATA and BSS segment. */
	SceUID mem_block_id; //168
	/** An array containing the alignment information of each segment. */
	u32 segment_align[SCE_KERNEL_MAX_MODULE_SEGMENT]; //172
	/** The largest value of the segment_align array. */
	u32 max_seg_align; //188
} SceLoadCoreExecFileInfo;

/**
 * This structure represents a boot callback belonging to a module.
 */
typedef struct SceBootCallback {
	/** The boot callback function. */
	void *boot_callback_func;
	/** Global pointer value of the module. */
	u32 gp;
} SceBootCallback;

/**
 * Find a module by it's name.
 *
 * @param modname - The name of the module.
 *
 * @returns Pointer to the ::SceModule structure if found, otherwise NULL.
 */
//SceModule * sceKernelFindModuleByName(const char *modname);
SceModule *sceKernelFindModuleByName(const char *modname);

/**
 * Find a module from an address.
 *
 * @param addr - Address somewhere within the module.
 *
 * @returns Pointer to the ::SceModule structure if found, otherwise NULL.
 */
//SceModule * sceKernelFindModuleByAddress(unsigned int addr);
SceModule *sceKernelFindModuleByAddress(u32 addr);

/**
 * Find a module by it's UID.
 *
 * @param modid - The UID of the module.
 *
 * @returns Pointer to the ::SceModule structure if found, otherwise NULL.
 */
//SceModule * sceKernelFindModuleByUID(SceUID modid);
SceModule *sceKernelFindModuleByUID(SceUID modid);


/**
 * Return the count of loaded modules.
 *
 * @returns The count of loaded modules.
 */
int sceKernelModuleCount(void);

/**
 * Invalidate the CPU's instruction cache.
 */
void sceKernelIcacheClearAll(void);

/**
 * Check an executable file. This contains scanning its ELF header and ~PSP header (if it has one)
 * and filling the execInfo structure with basic information, like the ELF type, segment information,
 * the size of the executable. The file is also uncompressed, if it was compressed before.
 *
 * @param buf Pointer to the file's contents.
 * @param execInfo Pointer to the executionInfo belonging to that executable.
 *
 * @return 0 on success.
 */
int sceKernelCheckExecFile(void *buf, SceLoadCoreExecFileInfo *execInfo);

/**
 * Probe an executable file. This contains calculating the sizes for the three segments TEXT, DATA
 * and BSS, filling the execInfo structure with information about the location and sizes of the
 * resident/stub library entry tables.
 *
 * Furthermore, it is checked whether the executable has valid API type or not.
 *
 * @param buf Pointer to the file's contents.
 * @param execInfo Pointer to the executionInfo belonging to that executable.
 *
 * @return 0 on success.
 */
int sceKernelProbeExecutableObject(void *buf, SceLoadCoreExecFileInfo *execInfo);

/*@}*/

#ifdef __cplusplus
}
#endif

#endif /* PSPLOADCORE_H */
