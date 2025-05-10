#include <pspsysmem.h>
#include <string.h>
#include "utils.h"

void* vsh_malloc(size_t size) {
	SceUID uid = sceKernelAllocPartitionMemory(2, "", PSP_SMEM_High, size+sizeof(u32), NULL);
	int* ptr = sceKernelGetBlockHeadAddr(uid);
	if (ptr){
		ptr[0] = uid;
		return &(ptr[1]);
	}
	return NULL;
}

void vsh_free(void* ptr) {
	if (ptr){
		int* p = ptr;
		int uid = p[-1];
		sceKernelFreePartitionMemory(uid);
	}
}
