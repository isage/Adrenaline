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
#include <adrenaline_log.h>

#include "main.h"
#include "executable_patch.h"

int (* _sceKernelCheckExecFile)(void *buf, SceLoadCoreExecFileInfo *execInfo);
int (* _sceKernelProbeExecutableObject)(void *buf, SceLoadCoreExecFileInfo *execInfo);
int (* PspUncompress)(void *buf, SceLoadCoreExecFileInfo *execInfo, u32 *newSize);
int (* PartitionCheck)(u32 *param, SceLoadCoreExecFileInfo *execInfo);

__attribute__((noinline)) void AdjustExecInfo(void *buf, SceLoadCoreExecFileInfo *execInfo) {
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

int sceKernelProbeExecutableObjectPatched(void *buf, SceLoadCoreExecFileInfo *execInfo)
{
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