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

#include <psp2kern/ctrl.h>
#include <psp2kern/io/fcntl.h>
#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/threadmgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/kernel/sysroot.h>
#include <psp2kern/kernel/cpu.h>
#include <psp2kern/kernel/debug.h>
#include <psp2common/display.h>

#include <stdio.h>
#include <string.h>

#include <taihen.h>

#include "utils.h"

#include "../adrenaline_compat.h"

int ksceKernelSysrootGetShellPid();
int module_get_export_func(SceUID pid, const char *modname, uint32_t libnid, uint32_t funcnid, uintptr_t *func);

static tai_hook_ref_t ksceKernelAllocMemBlockRef;
static tai_hook_ref_t ksceKernelFreeMemBlockRef;
static tai_hook_ref_t ksceKernelUnmapMemBlockRef;
static tai_hook_ref_t SceGrabForDriver_E9C25A28_ref;
static tai_hook_ref_t sceCompatSecSetSSRAMAclRef;
static tai_hook_ref_t ksceSblAimgrIsDEXRef;
static tai_hook_ref_t ksceKernelStartPreloadedModulesRef;
static tai_hook_ref_t ksceDisplaySetFrameBufInternalRef;

static int hooks[9];
static int n_hooks = 0;

static SceUID extra_1_blockid = -1, extra_2_blockid = -1;
static uint32_t module_nid;

static uint8_t blanking = 0;
static uint32_t blackline;
static SceUID blackline_uid = 0;
static uint32_t volume = 0;

static SceUID ksceKernelAllocMemBlockPatched(const char *name, SceKernelMemBlockType type, int size, SceKernelAllocMemBlockKernelOpt *optp) {
	SceUID blockid = TAI_CONTINUE(SceUID, ksceKernelAllocMemBlockRef, name, type, size, optp);

	uint32_t addr;
	ksceKernelGetMemBlockBase(blockid, (void *)&addr);

	if (addr == 0x23000000) {
		extra_1_blockid = blockid;
	} else if (addr == 0x24000000) {
		extra_2_blockid = blockid;
	}

	return blockid;
}

static int ksceKernelFreeMemBlockPatched(SceUID uid) {
	if (uid == extra_1_blockid) {
		return 0;
	}

	int res = TAI_CONTINUE(int, ksceKernelFreeMemBlockRef, uid);

	if (uid == extra_2_blockid) {
		ksceKernelFreeMemBlock(extra_1_blockid);
		extra_1_blockid = -1;
		extra_2_blockid = -1;
	}

	return res;
}

static int ksceKernelUnmapMemBlockPatched(SceUID uid) {
	return 0;
}

static int SceGrabForDriver_E9C25A28_patched(int unk, uint32_t paddr) {
	if (unk == 2 && paddr == 0x21000001) {
		paddr = 0x22000001;
	}

	return TAI_CONTINUE(int, SceGrabForDriver_E9C25A28_ref, unk, paddr);
}

static int sceCompatSecSetSSRAMAclPatched() {
	uint32_t a;

	a = 0;
	ksceKernelMemcpyUserToKernel(&a, (void*)0x70FC0000, sizeof(uint32_t));

	if (a != 0) {
		// jal 0x88FC0000
		a = 0x0E3F0000;
		if (module_nid == 0x8F2D0378) { // 3.60 retail
			ksceKernelMemcpyKernelToUser((void*)0x70602D58, &a, sizeof(uint32_t));
			ksceKernelDcacheCleanRangeForL1WBWA((void *)0x70602D58, sizeof(uint32_t));

		} else if (module_nid == 0x07937779 || module_nid == 0x71BF9CC5
				|| module_nid == 0x7C185186 || module_nid == 0x52DFE3A7
				|| module_nid == 0xE0E3AA51 || module_nid == 0x2F5A9526
				|| module_nid == 0x8BD7640C || module_nid == 0x6E2F2888
				|| module_nid == 0x0ED0A986) { // 3.65-3.74 retail

			ksceKernelMemcpyKernelToUser((void*)0x70602D70, &a, sizeof(uint32_t));
			ksceKernelDcacheCleanRangeForL1WBWA((void *)0x70602D70, sizeof(uint32_t));
		}
	}

	return TAI_CONTINUE(int, sceCompatSecSetSSRAMAclRef);
}

static int ksceSblAimgrIsDEXPatched() {
	TAI_CONTINUE(int, ksceSblAimgrIsDEXRef);
	return 1;
}

static int ksceKernelStartPreloadedModulesPatched(SceUID pid) {
	int res = TAI_CONTINUE(int, ksceKernelStartPreloadedModulesRef, pid);

	char titleid[32];
	ksceKernelSysrootGetProcessTitleId(pid, titleid, sizeof(titleid));

	if (strcmp(titleid, "main") == 0) {
		ksceKernelLoadStartModuleForPid(pid, "ux0:app/" ADRENALINE_TITLEID "/sce_module/adrenaline_vsh.suprx", 0, NULL, 0, NULL, NULL);
	} else if (strcmp(titleid, ADRENALINE_TITLEID) == 0) {
		ksceKernelLoadStartModuleForPid(pid, "ux0:app/" ADRENALINE_TITLEID "/sce_module/adrenaline_user.suprx", 0, NULL, 0, NULL, NULL);
	}

	return res;
}

int ksceDisplaySetFrameBufInternalPatched(int head, int index, const SceDisplayFrameBuf *pParam, int sync) {
	if (!head || !pParam) {
		goto DISPLAY_HOOK_RET;
	}

	if (!blanking) {
		goto DISPLAY_HOOK_RET;
	}

	for( int i = 0; i < pParam->height; i++) {
		int off = i * pParam->pitch * 4;
		ksceKernelMemcpyKernelToUser(pParam->base + off, (const void*)blackline, pParam->pitch * 4);
	}

DISPLAY_HOOK_RET:
	return TAI_CONTINUE(int, ksceDisplaySetFrameBufInternalRef, head, index, pParam, sync);
}

int kuCtrlPeekBufferPositive(int port, SceCtrlData *pad_data, int count) {
	uint32_t state;
	ENTER_SYSCALL(state);

	SceCtrlData pad;
	uint32_t off;

	// set cpu offset to zero
	asm volatile ("mrc p15, 0, %0, c13, c0, 4" : "=r" (off));
	asm volatile ("mcr p15, 0, %0, c13, c0, 4" :: "r" (0));

	int res = ksceCtrlPeekBufferPositive(port, &pad, count);

	// restore cpu offset
	asm volatile ("mcr p15, 0, %0, c13, c0, 4" :: "r" (off));

	ksceKernelMemcpyKernelToUser((void*)pad_data, &pad, sizeof(SceCtrlData));

	EXIT_SYSCALL(state);
	return res;
}

int vfsGetMntList(SceIoMount *mounts, SceUInt32 total_num_mounts, SceUInt32 *result_num_mounts);
int kuVfsGetMntList(SceIoMount *mounts, SceUInt32 total_num_mounts, SceUInt32 *result_num_mounts) {
	uint32_t state;
	ENTER_SYSCALL(state);

	SceIoMount p_mount[24];
	SceUInt32 num_mounts = 0;

	int res = vfsGetMntList(p_mount, 24, &num_mounts);

	*result_num_mounts = num_mounts;

	ksceKernelMemcpyKernelToUser((void*)mounts, p_mount, sizeof(SceIoMount)*num_mounts);

	EXIT_SYSCALL(state);
	return res;
}

int vfsGetMntInfo(uint32_t mount, SceIoMountInfo *info);
int kuVfsGetMntInfo(uint32_t mount, SceIoMountInfo *info) {
	uint32_t state;
	ENTER_SYSCALL(state);

	SceIoMountInfo k_info= {0};
	int res = vfsGetMntInfo(mount, &k_info);

	ksceKernelMemcpyKernelToUser((void*)info, &k_info, sizeof(SceIoMountInfo));

	EXIT_SYSCALL(state);
	return res;
}

int adrStartBlanking(int vol) {
	uint32_t state;
	ENTER_SYSCALL(state);

	blackline_uid = ksceKernelAllocMemBlock("blackline", SCE_KERNEL_MEMBLOCK_TYPE_KERNEL_RW, 1024*4, NULL);
	ksceKernelGetMemBlockBase(blackline_uid, (void *)&blackline);

	blanking = 1;
	volume = vol;

	EXIT_SYSCALL(state);
	return 0;
}

uint32_t adrStopBlanking() {
	uint32_t state;
	ENTER_SYSCALL(state);

	blanking = 0;

	if (blackline_uid >= 0) {
		ksceKernelFreeMemBlock(blackline_uid);
	}

	int vol = volume;
	volume = 0;

	EXIT_SYSCALL(state);
	return vol;
}

void _start() __attribute__ ((weak, alias("module_start")));
int module_start(SceSize args, void *argp) {
	int res;

	// Tai module info
	tai_module_info_t info;
	info.size = sizeof(tai_module_info_t);
	res = taiGetModuleInfoForKernel(KERNEL_PID, "SceCompat", &info);
	if (res < 0) {
		return res;
	}

	module_nid = info.module_nid;

	// SceCompat
	switch (info.module_nid) {
		case 0x8F2D0378: // 3.60 retail
			hooks[n_hooks++] = taiHookFunctionOffsetForKernel(KERNEL_PID, &sceCompatSecSetSSRAMAclRef, info.modid, 0, 0x2AA4, 1, sceCompatSecSetSSRAMAclPatched);
			break;

		case 0x07937779: // 3.65 retail
		case 0x71BF9CC5: // 3.67 retail
		case 0x7C185186: // 3.68 retail
		case 0x52DFE3A7: // 3.69 retail
		case 0xE0E3AA51: // 3.70 retail
		case 0x2F5A9526: // 3.71 retail
		case 0x8BD7640C: // 3.72 retail
		case 0x6E2F2888: // 3.73 retail
		case 0x0ED0A986: // 3.74 retail
			hooks[n_hooks++] = taiHookFunctionOffsetForKernel(KERNEL_PID, &sceCompatSecSetSSRAMAclRef, info.modid, 0, 0x2AE8, 1, sceCompatSecSetSSRAMAclPatched);
			break;
	}

	// SceSysmemForDriver
	hooks[n_hooks++] = taiHookFunctionImportForKernel(KERNEL_PID, &ksceKernelAllocMemBlockRef, "SceCompat", 0x6F25E18A, 0xC94850C9, ksceKernelAllocMemBlockPatched);
	hooks[n_hooks++] = taiHookFunctionImportForKernel(KERNEL_PID, &ksceKernelFreeMemBlockRef, "SceCompat", 0x6F25E18A, 0x009E1C61, ksceKernelFreeMemBlockPatched);
	hooks[n_hooks++] = taiHookFunctionImportForKernel(KERNEL_PID, &ksceKernelUnmapMemBlockRef, "SceCompat", 0x6F25E18A, 0xFFCD9B60, ksceKernelUnmapMemBlockPatched);

	// SceGrabForDriver
	hooks[n_hooks++] = taiHookFunctionImportForKernel(KERNEL_PID, &SceGrabForDriver_E9C25A28_ref, "SceCompat", 0x81C54BED, 0xE9C25A28, SceGrabForDriver_E9C25A28_patched);

	// SceSblAIMgrForDriver
	hooks[n_hooks++] = taiHookFunctionImportForKernel(KERNEL_PID, &ksceSblAimgrIsDEXRef, "SceCompat", 0xFD00C69A, 0xF4B98F66, ksceSblAimgrIsDEXPatched);

	// SceKernelModulemgr
	hooks[n_hooks] = taiHookFunctionExportForKernel(KERNEL_PID, &ksceKernelStartPreloadedModulesRef, "SceKernelModulemgr", 0xC445FA63, 0x432DCC7A, ksceKernelStartPreloadedModulesPatched);
	if (hooks[n_hooks] < 0)
		hooks[n_hooks] = taiHookFunctionExportForKernel(KERNEL_PID, &ksceKernelStartPreloadedModulesRef, "SceKernelModulemgr", 0x92C9FFC2, 0x998C7AE9, ksceKernelStartPreloadedModulesPatched);
	n_hooks++;

	hooks[n_hooks++] = taiHookFunctionExportForKernel(KERNEL_PID, &ksceDisplaySetFrameBufInternalRef, "SceDisplay", 0x9FED47AC, 0x16466675, ksceDisplaySetFrameBufInternalPatched);

	SceUID shell_pid = ksceKernelSysrootGetShellPid();
	ksceKernelLoadStartModuleForPid(shell_pid, "ux0:app/" ADRENALINE_TITLEID "/sce_module/adrenaline_vsh.suprx", 0, NULL, 0, NULL, NULL);

	return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize args, void *argp) {
	taiHookReleaseForKernel(hooks[--n_hooks], ksceDisplaySetFrameBufInternalRef);
	taiHookReleaseForKernel(hooks[--n_hooks], ksceKernelStartPreloadedModulesRef);
	taiHookReleaseForKernel(hooks[--n_hooks], ksceSblAimgrIsDEXRef);
	taiHookReleaseForKernel(hooks[--n_hooks], SceGrabForDriver_E9C25A28_ref);
	taiHookReleaseForKernel(hooks[--n_hooks], ksceKernelUnmapMemBlockRef);
	taiHookReleaseForKernel(hooks[--n_hooks], ksceKernelFreeMemBlockRef);
	taiHookReleaseForKernel(hooks[--n_hooks], ksceKernelAllocMemBlockRef);
	taiHookReleaseForKernel(hooks[--n_hooks], sceCompatSecSetSSRAMAclRef);

	return SCE_KERNEL_STOP_SUCCESS;
}
