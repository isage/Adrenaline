#include <string.h>
#include <stdio.h>
#include <pspsdk.h>
#include <pspsysmem_kernel.h>
#include <psputilsforkernel.h>
#include <pspinit.h>

#include <cfwmacros.h>
#include <bootloadex.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <systemctrl_adrenaline.h>

#include "adrenaline.h"
#include "externs.h"
#include <systemctrl_adrenaline.h>

// Sony flash0 files
static BootFileList* g_boot_files = (BootFileList*)BOOT_FILE_LIST_ADDR;
static int g_cur_file = 14;

////////////////////////////////////////////////////////////////////////////////
// PATCHED IMPLEMENTATIONS
////////////////////////////////////////////////////////////////////////////////

SceUID sceKernelLoadModuleBufferBootInitBtcnfPatched(SceLoadCoreBootModuleInfo *info, void *buf, int flags, SceKernelLMOption *option) {

	char path[64];
	char* filename = (info->name[0])? info->name : (char*)&(g_boot_files->bootfile[g_cur_file]);

	PSPKeyConfig app_type = sceKernelApplicationType();

	if (g_config.osk_type == OSK_TYPE_PSP) {
		if (strcmp(filename, "/kd/kermit_utility.prx") == 0) {
			filename = "/kd/utility.prx";
		}
	}

	if (g_config.ge_type == GE_TYPE_2 && app_type != PSP_INIT_KEYCONFIG_VSH) {
		if (strcmp(filename, "/kd/ge.prx") == 0) {
			filename = "/kd/ge_2.prx";

			char* path = "flash0:/kd/ge_2.prx";
			SceUID mod = sceKernelLoadModule(path, 0, NULL);
			if (mod >= 0) {
				return mod;
			}
		}
	}

	if (g_config.me_type == ME_TYPE_2 && app_type != PSP_INIT_KEYCONFIG_VSH) {
		if (strcmp(filename, "/kd/kermit_me_wrapper.prx") == 0) {
			filename = "/kd/kermit_me_wrapper_2.prx";

			char* path = "flash0:/kd/kermit_me_wrapper_2.prx";
			SceUID mod = sceKernelLoadModule(path, 0, NULL);
			if (mod >= 0) {
				return mod;
			}
		}
	}

	sprintf(path, "ms0:/__ADRENALINE__/flash0%s", filename); //not use flash0 cause of cxmb

	g_cur_file++;

	SceUID mod = sceKernelLoadModule(path, 0, NULL);
	if (mod >= 0)
		return mod;

	return sceKernelLoadModuleBufferBootInitBtcnf(info->size, buf, flags, option);
}

static SceUID (* LoadModuleBufferAnchorInBtcnf)(void *buf, int a1);
SceUID LoadModuleBufferAnchorInBtcnfPatched(void *buf, SceLoadCoreBootModuleInfo *info) {

	char path[64];
	char* filename = (info->name[0])? info->name : (char*)&(g_boot_files->bootfile[g_cur_file]);

	sprintf(path, "ms0:/__ADRENALINE__/flash0%s", filename);

	g_cur_file++;

	SceUID mod = sceKernelLoadModule(path, 0, NULL);
	if (mod >= 0) {
		return mod;
	}

	return LoadModuleBufferAnchorInBtcnf(buf, (info->attr >> 8) & 1);
}

static int (*PrevPatchInit)(int (* module_bootstart)(SceSize, void *), void *argp) = NULL;
int AdrenalinePatchInit(int (* module_bootstart)(SceSize, void *), void *argp) {
	u32 init_addr = ((u32)module_bootstart) - 0x1A54;

	// Ignore StopInit
	MAKE_NOP(init_addr + 0x18EC);

	// Redirect load functions to load from MS
	LoadModuleBufferAnchorInBtcnf = (void *)init_addr + 0x1038;
	MAKE_CALL(init_addr + 0x17E4, LoadModuleBufferAnchorInBtcnfPatched);
	MAKE_INSTRUCTION(init_addr + 0x17E8, 0x02402821); // move $a1, $s2

	MAKE_INSTRUCTION(init_addr + 0x1868, 0x02402021); // move $a0, $s2
	MAKE_CALL(init_addr + 0x1878, sceKernelLoadModuleBufferBootInitBtcnfPatched);

	sctrlFlushCache();

	return PrevPatchInit(module_bootstart, argp);
}

////////////////////////////////////////////////////////////////////////////////
// MODULE PATCHERS
////////////////////////////////////////////////////////////////////////////////

// Patch Loader Core Module
void PatchLoaderCore(void) {
	// Find Module
	SceModule* mod = sceKernelFindModuleByName("sceLoaderCore");

	for (int i = 0; i < g_boot_files->nfiles; i++) {
		if (strcmp((char*)&(g_boot_files->bootfile[i]), "/kd/init.prx") == 0){
			g_cur_file = i+1;
			break;
		}
	}

	// Fetch Text Address
	u32 start_addr = mod->text_addr;
	u32 topaddr = mod->text_addr+mod->text_size;

	for (u32 addr = start_addr; addr<topaddr; addr+=4){
		u32 data = VREAD32(addr);
		if (data == 0x02E02021){
			PrevPatchInit = (void*)K_EXTRACT_CALL(addr-4);
			MAKE_INSTRUCTION(addr-4, JAL(AdrenalinePatchInit));
			break;
		}
	}

	sctrlFlushCache();
}
