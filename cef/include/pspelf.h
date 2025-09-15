#ifndef _PSPELF_H_
#define _PSPELF_H_

#include <psptypes.h>

typedef struct {
	u32 e_magic;
	u8 e_class;
	u8 e_data;
	u8 e_idver;
	u8 e_pad[9];
	u16 e_type;
	u16 e_machine;
	u32 e_version;
	u32 e_entry;
	u32 e_phoff;
	u32 e_shoff;
	u32 e_flags;
	u16 e_ehsize;
	u16 e_phentsize;
	u16 e_phnum;
	u16 e_shentsize;
	u16 e_shnum;
	u16 e_shstrndx;
} __attribute__((packed)) Elf32_Ehdr;

typedef struct {
	u32 sh_name;
	u32 sh_type;
	u32 sh_flags;
	u32 sh_addr;
	u32 sh_offset;
	u32 sh_size;
	u32 sh_link;
	u32 sh_info;
	u32 sh_addralign;
	u32 sh_entsize;
} __attribute__((packed)) Elf32_Shdr;

#endif