	.set noreorder

#include "pspimport.s"
#include "pspstub.s"

	IMPORT_START "SysMemForKernel",0x00010011
	IMPORT_FUNC  "SysMemForKernel",0x58148F07,sceKernelCreateHeap661
	IMPORT_FUNC  "SysMemForKernel",0x23D81675,sceKernelAllocHeapMemory661
	IMPORT_FUNC  "SysMemForKernel",0x87C2AB85,sceKernelFreeHeapMemory661
	IMPORT_FUNC  "SysMemForKernel",0x7158CE7E,sceKernelAllocPartitionMemory661
	IMPORT_FUNC  "SysMemForKernel",0xC1A26C6F,sceKernelFreePartitionMemory661
	IMPORT_FUNC  "SysMemForKernel",0xF12A62F7,sceKernelGetBlockHeadAddr661
	IMPORT_FUNC  "SysMemForKernel",0x83B5226D,sceKernelSetDdrMemoryProtection661
	IMPORT_FUNC  "SysMemForKernel",0x36C503A9,sceKernelGetSystemStatus661
	IMPORT_FUNC  "SysMemForKernel",0xEF29061C,sceKernelGetGameInfo661
