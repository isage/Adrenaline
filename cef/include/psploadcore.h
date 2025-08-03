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

/** Describes a module.  This structure could change in future firmware revisions. */
typedef struct SceModule {
	struct SceModule	*next;
	unsigned short		attribute;
	unsigned char		version[2];
	char				modname[27];
	char				terminal;
	unsigned int		unknown1;
	unsigned int		unknown2;
	SceUID				modid;
	unsigned int		unknown3[4];
	void *				ent_top;
	unsigned int		ent_size;
	void *				stub_top;
	unsigned int		stub_size;
	unsigned int		unknown4[4];
	unsigned int		entry_addr;
	unsigned int		gp_value;
	unsigned int		text_addr;
	unsigned int		text_size;
	unsigned int		data_size;
	unsigned int		bss_size;
	unsigned int		nsegment;
	unsigned int		segmentaddr[4];
	unsigned int		segmentsize[4];
} SceModule;

// For 1.50+

typedef struct SceModule2
{
	struct SceModule	*next; // 0
	u16					attribute; // 4
	u8					version[2]; // 6
	char				modname[27]; // 8
	char				terminal; // 0x23
	char				mod_state;	// 0x24
    char				unk1;    // 0x25
	char				unk2[2]; // 0x26
	u32					unk3;	// 0x28
	SceUID				modid; // 0x2C
	u32					unk4; // 0x30
	SceUID				mem_id; // 0x34
	u32					mpid_text;	// 0x38
	u32					mpid_data; // 0x3C
	void *				ent_top; // 0x40
	unsigned int		ent_size; // 0x44
	void *				stub_top; // 0x48
	u32					stub_size; // 0x4C
	u32					entry_addr_; // 0x50
	u32					unk5[4]; // 0x54
	u32					entry_addr; // 0x64
	u32					gp_value; // 0x68
	u32					text_addr; // 0x6C
	u32					text_size; // 0x70
	u32					data_size;	// 0x74
	u32					bss_size; // 0x78
	u32					nsegment; // 0x7C
	u32					segmentaddr[4]; // 0x80
	u32					segmentsize[4]; // 0x90
} SceModule2;

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
	void *				entrytable;
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
	void *				stubtable;
	/** Pointer to the imported variable stubs. */
	void *				vstubtable;
} SceLibraryStubTable;

typedef struct SceLoadCoreExecFileInfo {
    /** Unknown. */
    u32 unk0;
    /** The mode attribute of the executable file. One of ::SceExecFileModeAttr. */
    u32 modeAttribute; //4
    /** The API type. */
    u32 apiType; //8
    /** Unknown. */
    u32 unk12;
    /** The size of the executable, including the ~PSP header. */
    SceSize execSize; //16
    /** The maximum size needed for the decompression. */
    SceSize maxAllocSize; //20
    /** The memory ID of the decompression buffer. */
    SceUID decompressionMemId; //24
    /** Pointer to the compressed module data. */
    void *fileBase; //28
    /** Indicates the ELF type of the executable. One of ::SceExecFileElfType. */
    u32 elfType; //32
    /** The start address of the TEXT segment of the executable in memory. */
    void *topAddr; //36
    /**
     * The entry address of the module. It is the offset from the start of the TEXT segment to the
     * program's entry point.
     */
    u32 entryAddr; //40
    /** Unknown. */
    u32 unk44;
    /**
     * The size of the largest module segment. Should normally be "textSize", but technically can
     * be any other segment.
     */
    SceSize largestSegSize; //48
    /** The size of the TEXT segment. */
    SceSize textSize; //52
    /** The size of the DATA segment. */
    SceSize dataSize; //56
    /** The size of the BSS segment. */
    SceSize bssSize; //60
    /** The memory partition of the executable. */
    u32 partitionId; //64
    /**
     * Indicates whether the executable is a kernel module or not. Set to 1 for kernel module,
     * 0 for user module.
     */
    u32 isKernelMod; //68
    /**
     * Indicates whether the executable is decrypted or not. Set to 1 if it is successfully decrypted,
     * 0 for encrypted.
     */
    u32 isDecrypted; //72
    /** The offset from the start address of the TEXT segment to the SceModuleInfo section. */
    u32 moduleInfoOffset; //76
    /** The pointer to the module's SceModuleInfo section. */
    SceModuleInfo *moduleInfo; //80
    /** Indicates whether the module is compressed or not. Set to 1 if it is compressed, otherwise 0.*/
    u32 isCompressed; //84
    /** The module's attributes. One or more of ::SceModuleAttribute and ::SceModulePrivilegeLevel. */
    u16 modInfoAttribute; //88
    /** The attributes of the executable file. One of ::SceExecFileAttr. */
    u16 execAttribute; //90
    /** The size of the decompressed module, including its headers. */
    SceSize decSize; //92
    /** Indicates whether the module is decompressed or not. Set to 1 for decompressed, otherwise 0. */
    u32 isDecompressed; //96
    /**
     * Indicates whether the module was signChecked or not. Set to 1 for signChecked, otherwise 0.
     * A signed module has a "mangled" executable header, in other words, the "~PSP" signature can't
     * be seen.
     */
    u32 isSignChecked; //100
    /** Unknown. */
    u32 unk104;
    /** The size of the GZIP compression overlap. */
    SceSize overlapSize; //108
    /** Pointer to the first resident library entry table of the module. */
    void *exportsInfo; //112
    /** The size of all resident library entry tables of the module. */
    SceSize exportsSize; //116
    /** Pointer to the first stub library entry table of the module. */
    void *importsInfo; //120
    /** The size of all stub library entry tables of the module. */
    SceSize importsSize; //124
    /** Pointer to the string table section. */
    void *strtabOffset; //128
    /** The number of segments in the executable. */
    u8 numSegments; //132
    /** Reserved. */
    u8 padding[3]; //133
    /** An array containing the start address of each segment. */
    u32 segmentAddr[SCE_KERNEL_MAX_MODULE_SEGMENT]; //136
    /** An array containing the size of each segment. */
    u32 segmentSize[SCE_KERNEL_MAX_MODULE_SEGMENT]; //152
    /** The ID of the ELF memory block containing the TEXT, DATA and BSS segment. */
    SceUID memBlockId; //168
    /** An array containing the alignment information of each segment. */
    u32 segmentAlign[SCE_KERNEL_MAX_MODULE_SEGMENT]; //172
    /** The largest value of the segmentAlign array. */
    u32 maxSegAlign; //188
} SceLoadCoreExecFileInfo;

/**
 * This structure represents a boot callback belonging to a module.
 */
typedef struct SceBootCallback {
    /** The boot callback function. */
    void *bootCBFunc;
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
SceModule2 *sceKernelFindModuleByName(const char *modname);

/**
 * Find a module from an address.
 *
 * @param addr - Address somewhere within the module.
 *
 * @returns Pointer to the ::SceModule structure if found, otherwise NULL.
 */
//SceModule * sceKernelFindModuleByAddress(unsigned int addr);
SceModule2 *sceKernelFindModuleByAddress(u32 addr);

/**
 * Find a module by it's UID.
 *
 * @param modid - The UID of the module.
 *
 * @returns Pointer to the ::SceModule structure if found, otherwise NULL.
 */
//SceModule * sceKernelFindModuleByUID(SceUID modid);
SceModule2 *sceKernelFindModuleByUID(SceUID modid);


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
