/*
	Adrenaline
	Copyright (C) 2016-2018, TheFloW

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

#include <common.h>
#include <pspelf.h>

#include "main.h"
#include "init_patch.h"
#include "libraries_patch.h"

int (* _sceKernelCheckExecFile)(void *buf, SceLoadCoreExecFileInfo *execInfo);
int (* _sceKernelProbeExecutableObject)(void *buf, SceLoadCoreExecFileInfo *execInfo);
int (* PspUncompress)(void *buf, SceLoadCoreExecFileInfo *execInfo, u32 *newSize);
int (* PartitionCheck)(u32 *param, SceLoadCoreExecFileInfo *execInfo);
int (* PrologueModule)(void *modmgr_param, SceModule *mod) = NULL;

////////////////////////////////////////////////////////////////////////////////
// HELPERS
////////////////////////////////////////////////////////////////////////////////

__attribute__((noinline))
static void AdjustExecInfo(void *buf, SceLoadCoreExecFileInfo *execInfo) {
	SceModuleInfo *modInfo = (SceModuleInfo *)((u32)buf + execInfo->module_info_offset);

	if ((u32)modInfo >= 0x88400000 && (u32)modInfo <= 0x88800000) {
		return;
	}
	// don't null attribute for proper encrypted apps (senseme, etc)
	if (execInfo->mod_info_attribute == 0 && modInfo->modattribute != 0)
		execInfo->mod_info_attribute = modInfo->modattribute;

	execInfo->is_kernel_mod = (execInfo->mod_info_attribute & 0x1000) ? 1 : 0;
}

__attribute__((noinline)) void AdjustExecInfoProbe(void *buf, SceLoadCoreExecFileInfo *execInfo) {
	SceModuleInfo *modInfo = (SceModuleInfo *)((u32)buf + execInfo->module_info_offset);

	if ((u32)modInfo >= 0x88400000 && (u32)modInfo <= 0x88800000) {
		return;
	}

	execInfo->mod_info_attribute = modInfo->modattribute;
	execInfo->is_kernel_mod = (execInfo->mod_info_attribute & 0x1000) ? 1 : 0;
}

////////////////////////////////////////////////////////////////////////////////
// PATCHED IMPLEMENTATIONS
////////////////////////////////////////////////////////////////////////////////

int sceKernelCheckExecFilePatched(void *buf, SceLoadCoreExecFileInfo *execInfo) {
	Elf32_Ehdr *header = (Elf32_Ehdr *)buf;

	int res = _sceKernelCheckExecFile(buf, execInfo);

	// Plain ELF
	if (header->e_magic == ELF_MAGIC) {
		execInfo->is_decrypted = 1;

		// Static ELF
		if (execInfo->elf_type == -1 && execInfo->module_info_offset == 0 && header->e_type == 2) {
			execInfo->elf_type = 3;
		}

		AdjustExecInfo(buf, execInfo);
	}

	return res;
}

int sceKernelProbeExecutableObjectPatched(void *buf, SceLoadCoreExecFileInfo *execInfo) {
	Elf32_Ehdr *header = (Elf32_Ehdr *)buf;

	// save mod_info_attribute before we null it
	u32 attr = execInfo->mod_info_attribute;

	// Plain ELF
	if (header->e_magic == ELF_MAGIC) {
		if (execInfo->is_decrypted) {
			// Static ELF
			if (header->e_type == 2) {
				execInfo->api_type = SCE_APITYPE_UMD;

				// Find module_info_offset
				if (execInfo->module_info_offset == 0) {
					Elf32_Shdr *section = (Elf32_Shdr *)((u32)header + header->e_shoff);
					char *strtable = (char *)((u32)header + section[header->e_shstrndx].sh_offset);

					for (int i = 0; i < header->e_shnum; i++) {
						if (strcmp(strtable + section[i].sh_name, ".rodata.sceModuleInfo") == 0) {
							execInfo->module_info_offset = section[i].sh_offset;
							break;
						}
					}
				}
			}

			AdjustExecInfoProbe(buf, execInfo);
		}
	}
	
	// NOTE: probe won't pass mod_info_attribute with privelege level for unencrypted elf
	int ret = _sceKernelProbeExecutableObject(buf, execInfo);

	// homebrew: force user privelege level after probe
	if (header->e_magic == ELF_MAGIC && attr == 0x457F && execInfo->mod_info_attribute == 0)
		execInfo->mod_info_attribute = 0x200;

	return ret;
}

int PspUncompressPatched(void *buf, SceLoadCoreExecFileInfo *execInfo, u32 *newSize) {
	if (*(u16 *)((u32)buf + 0x150) == 0x8B1F) {
		execInfo->is_decrypted = 1;
		sceKernelGzipDecompress(execInfo->top_addr, execInfo->dec_size, (void *)((u32)buf + 0x150), 0);
		return 0;
	}

	return PspUncompress(buf, execInfo, newSize);
}

int PartitionCheckPatched(u32 *param, SceLoadCoreExecFileInfo *execInfo) {
	SceUID fd = param[0x18/4];

	// Get position
	int pos = sceIoLseek32(fd, 0, PSP_SEEK_CUR);
	sceIoLseek32(fd, 0, PSP_SEEK_SET);

	// Read ELF header
	Elf32_Ehdr header;
	if (sceIoRead(fd, &header, sizeof(Elf32_Ehdr)) == sizeof(Elf32_Ehdr)) {
		u32 elf_offset = 0;

		// PBP magic
		if (header.e_magic == PBP_MAGIC) {
			PBPHeader *pbp_header = (PBPHeader *)&header;
			u32 psar_offset = pbp_header->psar_offset;
			elf_offset = pbp_header->elf_offset;

			// Read ELF header
			sceIoLseek32(fd, elf_offset, PSP_SEEK_SET);
			sceIoRead(fd, &header, sizeof(Elf32_Ehdr));

			// Allow psar's in decrypted pbp's
			if (header.e_type != 2) {
				execInfo->exec_size = psar_offset - elf_offset;
			}
		}

		// ELF magic
		if (header.e_magic == ELF_MAGIC) {
			// Go to SceModuleInfo offset
			sceIoLseek32(fd, elf_offset + execInfo->module_info_offset, PSP_SEEK_SET);

			// Adjust execInfo
			sceIoRead(fd, &execInfo->mod_info_attribute, sizeof(u16));
			execInfo->is_kernel_mod = (execInfo->mod_info_attribute & 0x1000) ? 1 : 0;
		}
	}

	sceIoLseek32(fd, pos, PSP_SEEK_SET);
	return PartitionCheck(param, execInfo);
}

int PrologueModulePatched(void *modmgr_param, SceModule *mod) {
	int res = PrologueModule(modmgr_param, mod);

	if (res >= 0 && module_handler)
		module_handler(mod);

	return res;
}

////////////////////////////////////////////////////////////////////////////////
// MODULE PATCHERS
////////////////////////////////////////////////////////////////////////////////

void PatchLoadCore() {
	SceModule *mod = sceKernelFindModuleByName("sceLoaderCore");
	u32 text_addr = mod->text_addr;

	HIJACK_FUNCTION(K_EXTRACT_IMPORT(&sceKernelCheckExecFile), sceKernelCheckExecFilePatched, _sceKernelCheckExecFile);
	HIJACK_FUNCTION(K_EXTRACT_IMPORT(&sceKernelProbeExecutableObject), sceKernelProbeExecutableObjectPatched, _sceKernelProbeExecutableObject);

	for (int i = 0; i < mod->text_size; i += 4) {
		u32 addr = text_addr + i;
		u32 data = VREAD32(addr);

		// Allow custom modules
		if (data == 0x1440FF55) {
			PspUncompress = (void *)K_EXTRACT_CALL(addr - 8);
			MAKE_CALL(addr - 8, PspUncompressPatched);
			continue;
		}

		// Patch relocation check in switch statement (7 -> 0)
		if (data == 0x00A22021) {
			u32 high = (((u32)VREAD16(addr - 0xC)) << 16);
			u32 low = ((u32)VREAD16(addr - 0x4));

			if (low & 0x8000) high -= 0x10000;

			u32 *RelocationTable = (u32 *)(high | low);

			RelocationTable[7] = RelocationTable[0];

			continue;
		}

		// Allow kernel modules to have syscall imports
		if (data == 0x30894000) {
			VWRITE32(addr, 0x3C090000);
			continue;
		}

		// Allow lower devkit version
		if (data == 0x14A0FFCB) {
			VWRITE16(addr + 2, 0x1000);
			continue;
		}

		// Allow higher devkit version
		if (data == 0x14C0FFDF) {
			MAKE_NOP(addr);
			continue;
		}

		// Patch to resolve NIDs
		if (data == 0x8D450000) {
			MAKE_INSTRUCTION(addr + 4, 0x02203021); // move $a2, $s1
			search_nid_in_entrytable = (void *)K_EXTRACT_CALL(addr + 8);
			MAKE_CALL(addr + 8, search_nid_in_entrytable_patched);
			MAKE_INSTRUCTION(addr + 0xC, 0x02403821); // move $a3, $s2
			continue;
		}

		if (data == 0xADA00004) {
			// Patch to resolve NIDs
			MAKE_NOP(addr);
			MAKE_NOP(addr + 8);

			// Patch to undo prometheus patches
			HIJACK_FUNCTION(addr + 0xC, aLinkLibEntriesPatched, aLinkLibEntries);

			continue;
		}

		// Patch call to init module_bootstart
		if (data == 0x02E0F809) {
			MAKE_CALL(addr, PatchInit);
			MAKE_INSTRUCTION(addr + 4, 0x02E02021); // move $a0, $s7
			continue;
		}

		// Restore original call
		if (data == 0xAE2D0048) {
			MAKE_CALL(addr + 8, FindProc("sceMemlmd", "memlmd", 0xEF73E85B));
			continue;
		}

		if (data == 0x40068000 && VREAD32(addr + 4) == 0x7CC51180) {
			LoadCoreForKernel_nids[0].function = (void *)addr;
			continue;
		}

		if (data == 0x40068000 && VREAD32(addr + 4) == 0x7CC51240) {
			LoadCoreForKernel_nids[1].function = (void *)addr;
			continue;
		}
	}
}

void PatchModuleMgr() {
	SceModule *mod = sceKernelFindModuleByName("sceModuleManager");
	u32 text_addr = mod->text_addr;

	for (int i = 0; i < mod->text_size; i += 4) {
		u32 addr = text_addr + i;
		u32 data = VREAD32(addr);

		if (data == 0xA4A60024) {
			// Patch to allow a full coverage of loaded modules
			PrologueModule = (void *)K_EXTRACT_CALL(addr - 4);
			MAKE_CALL(addr - 4, PrologueModulePatched);
			continue;
		}

		if (data == 0x27BDFFE0 && VREAD32(addr + 4) == 0xAFB10014) {
			HIJACK_FUNCTION(addr, PartitionCheckPatched, PartitionCheck);
			continue;
		}
	}

	// Dummy patch for LEDA
	MAKE_JUMP(sctrlHENFindImport(mod->modname, "ThreadManForKernel", 0x446D8DE6), sceKernelCreateThread);
}