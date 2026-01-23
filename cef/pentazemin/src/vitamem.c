#include <stdio.h>
#include <string.h>
#include <pspsdk.h>
#include <pspsysmem_kernel.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <pspsysevent.h>
#include <pspiofilemgr.h>

#include <systemctrl_ark.h>
#include <cfwmacros.h>
#include <systemctrl.h>
#include <systemctrl_se.h>


static void* findGetPartition(){
	for (u32 addr = SYSMEM_TEXT; ; addr+=4){
		if (VREAD32(addr) == 0x2C85000D){
			return (void*)(addr-4);
		}
	}
	return NULL;
}

int unlockVitaMemory(u32 user_size_mib){
	// Do not allow in pops and vsh
	int apitype = sceKernelInitApitype();
	if (apitype == PSP_INIT_APITYPE_MS5 || apitype == PSP_INIT_APITYPE_EF5 || apitype >= PSP_INIT_APITYPE_VSH_KERNEL) {
		return -1;
	}

	PspSysMemPartition *(* GetPartition)(int partition) = findGetPartition();

	PspSysMemPartition *partition;
	u32 user_size = user_size_mib * 1024 * 1024; // new p2 size

	// modify p2
	partition = GetPartition(PSP_MEMORY_PARTITION_USER);
	partition->size = user_size;
	partition->data->size = (((user_size >> 8) << 9) | 0xFC);

	// modify p11
	for (int i=8; i<12; i++){
		partition = GetPartition(i);
		if (partition) {
			partition->size = 0;
			partition->address = 0x88800000 + user_size;
			partition->data->size = (((partition->size >> 8) << 9) | 0xFC);
		}
	}

	return 0;
}

int (*_sctrlHENApplyMemory)(u32) = NULL;
int memoryHandlerVita(u32 p2){
	// sanity checks
	if (p2<=24) return -1;

	// the first 16MB are stable and good enough for most use cases
	// but homebrew that require extra ram will be allowed to use (some of) the upper 16MB
	if (p2 > 52){
		p2 = 52; //(se_config->force_high_memory == 2)? 52 : 40;
	}

	// call orig function to determine if can unlock
	int res = _sctrlHENApplyMemory(p2);
	if (res < 0) {
		return res;
	}

	// unlock
	res = unlockVitaMemory(p2);

	// unlock fail? revert back to 24MB
	if (res < 0) {
		_sctrlHENApplyMemory(24);
	}

	return res;
}

void PatchMemUnlock() {
	HIJACK_FUNCTION(K_EXTRACT_IMPORT(sctrlHENApplyMemory), memoryHandlerVita, _sctrlHENApplyMemory);

	sctrlFlushCache();
}