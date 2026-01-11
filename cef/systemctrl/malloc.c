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

#include "main.h"

// SceUID heapid = -1;

static void* malloc_impl(SceUID partition, SceSize size) {
	SceUID uid = sceKernelAllocPartitionMemory(partition, "", 1, size+sizeof(SceUID), NULL);
	int* ptr = sceKernelGetBlockHeadAddr(uid);
	if (ptr){
		ptr[0] = uid;
		return &(ptr[1]);
	}
	return NULL;
}

void oe_free(void *ptr) {
	if (ptr != NULL) {
		SceUID uid = ((SceUID*)ptr)[-1];
		sceKernelFreePartitionMemory(uid);
	}
}

void user_free(void* ptr) {
	oe_free(ptr);
}

void *oe_malloc(SceSize size) {
	return malloc_impl(PSP_MEMORY_PARTITION_KERNEL, size);
}

void* user_malloc(SceSize size) {
	return malloc_impl(PSP_MEMORY_PARTITION_USER, size);
}

void* user_memalign(SceSize align, SceSize size) {
	SceUID uid = sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_USER, "", 1, size+sizeof(SceUID)+align, NULL);
	int* ptr = sceKernelGetBlockHeadAddr(uid);
	if (ptr){
		ptr = (void*)(((u32)ptr & (~(align-1))) + 64);
		ptr[-1] = uid;
		return ptr;
	}
	return NULL;
}

int mallocinit() {
	int keyconfig = sceKernelApplicationType();
	if ((keyconfig == PSP_INIT_KEYCONFIG_POPS) || (keyconfig == PSP_INIT_KEYCONFIG_GAME && sceKernelInitApitype() == PSP_INIT_APITYPE_UMD_EMU_MS1)) {
		return 0;
	}

	// heapid = sceKernelCreateHeap(PSP_MEMORY_PARTITION_KERNEL, 256 * 1024, 1, "");

	// return (heapid < 0) ? heapid : 0;
	return 0;
}