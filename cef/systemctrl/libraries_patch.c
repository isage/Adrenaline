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

#include <pspintrman_kernel.h>

#include <systemctrl_adrenaline.h>
#include <adrenaline_log.h>

#include "main.h"
#include "libraries_patch.h"
#include "string_clone.h"

#include "nid_table.h"

int (* aLinkLibEntries)(void *lib);
int (* search_nid_in_entrytable)(void *lib, u32 nid, int unk, int nidSearchOption);

#define N_MISSING_NID(x) (sizeof(x) / sizeof(MissingNid))
#define N_MISSING_NID_LIST (sizeof(missing_nid_list) / sizeof(MissingNidList))

MissingNid SysclibForKernel_nids[] = {
	{ 0x1AB53A58, strtok_r },
	{ 0x87F8D2DA, strtok },
	{ 0x1D83F344, atob },
	{ 0x62AE052F, strspn },
	{ 0x89B79CB1, strcspn },
	{ 0xD3D1A3B9, strncat },
	{ 0x909C228B, &setjmp_clone },
	{ 0x18FE80DB, &longjmp_clone },
};

MissingNid LoadCoreForKernel_nids[] = {
	{ 0x2952F5AC, 0 }, //sceKernelDcacheWBinvAll
	{ 0xD8779AC6, 0 }, //sceKernelIcacheClearAll
};

MissingNidList missing_nid_list[] = {
	{ "SysclibForKernel", SysclibForKernel_nids, N_MISSING_NID(SysclibForKernel_nids) },
	{ "LoadCoreForKernel", LoadCoreForKernel_nids, N_MISSING_NID(LoadCoreForKernel_nids) },
};

void *ResolveMissingNIDs(const char *libname, u32 nid) {
	for (int i = 0; i < N_MISSING_NID_LIST; i++) {
		if (strcmp(missing_nid_list[i].libname, libname) == 0) {
			for (int j = 0; j < missing_nid_list[i].n_nid; j++) {
				if (missing_nid_list[i].nid[j].nid == nid) {
					return missing_nid_list[i].nid[j].function;
				}
			}
		}
	}

	return NULL;
}

u32 ResolveOldNIDs(const char *libname, u32 nid) {
	for (int i = 0; i < N_NID_TABLE; i++) {
		if (strcmp(nid_table[i].libname, libname) == 0) {
			for (int j = 0; j < nid_table[i].n_nid; j++) {
				if (nid_table[i].nid[j].old_nid == nid) {
					return nid_table[i].nid[j].new_nid;
				}
			}
		}
	}

	return 0;
}

u32 sctrlHENFindFunction(const char *mod_name, const char *library, u32 nid) {
	SceModule *mod = sceKernelFindModuleByName(mod_name);
	if (!mod) {
		mod = sceKernelFindModuleByAddress((SceUID)mod_name);
		if (!mod)
			return 0;
	}

	u32 new_nid = ResolveOldNIDs(library, nid);
	if (new_nid != 0) {
		nid = new_nid;
	}

	int i = 0;
	while (i < mod->ent_size) {
		SceLibraryEntryTable *entry = (SceLibraryEntryTable *)(mod->ent_top + i);

		if (!library || (entry->libname && strcmp(entry->libname, library) == 0)) {
			u32 *table = entry->entrytable;
			int total = entry->stubcount + entry->vstubcount;

			int j;
			for (j = 0; j < total; j++) {
				if (table[j] == nid) {
					return table[j + total];
				}
			}
		}

		i += (entry->len * 4);
	}

	return 0;
}

u32 sctrlHENFindImport(const char *mod_name, const char *library, u32 nid) {
	SceModule *mod = sceKernelFindModuleByName(mod_name);
	if (!mod) {
		mod = sceKernelFindModuleByAddress((SceUID)mod_name);
		if (!mod)
			return 0;
	}
/*
	u32 new_nid = ResolveOldNIDs(library, nid);
	if (new_nid)
		nid = new_nid;
*/
	int i = 0;
	while (i < mod->stub_size) {
		SceLibraryStubTable *stub = (SceLibraryStubTable *)(mod->stub_top + i);

		if (stub->libname && strcmp(stub->libname, library) == 0) {
			u32 *table = (u32 *)stub->nidtable;

			int j;
			for (j = 0; j < stub->stubcount; j++) {
				if (table[j] == nid) {
					logmsg4("[DEBUG]: %s: %s — %s — 0x%08lX: FOUND\n", __func__, mod->modname, library, nid);
					return ((u32)stub->stubtable + (j * 8));
				}
			}
		}

		i += (stub->len * 4);
	}

	logmsg4("[DEBUG]: %s: %s — %s — 0x%08lX: NOT FOUND\n", __func__, mod->modname, library, nid);
	return 0;
}

int aLinkLibEntriesPatched(void *lib) {
	char *libname = (char *)((u32 *)lib)[8/4];

	// Fix Prometheus patch
	if (strcmp(libname, "Kernel_LibrarZ") == 0 || strcmp(libname, "Kernel_Librar0") == 0) {
		libname[13] = 'y';
	}

	if (strcmp(libname, "sceUtilitO") == 0) {
		libname[9] = 'y';
	}

	return aLinkLibEntries(lib);
}

int search_nid_in_entrytable_patched(void *lib, u32 nid, void *stub, int count) {
	char *libname = (char *)((u32 *)lib)[0x44/4];
	u32 stubtable = ((u32 *)stub)[0x18/4];
	u32 original_stub = ((u32 *)stub)[0x24/4];
	int is_user_mode = ((u32 *)stub)[0x34/4];
	u32 stub_addr = stubtable + (count * 8);

	u32 module_sdk_version = sctrlHENFindFunction((char *)original_stub, NULL, 0x11B97506);
	if (module_sdk_version && (FIRMWARE_TO_FW(*(u32 *)module_sdk_version) == 0x660 || FIRMWARE_TO_FW(*(u32 *)module_sdk_version) == 0x661)) {
		// Sony module
	} else {
		if (!is_user_mode) {
			// Resolve missing NIDs
			void *function = ResolveMissingNIDs(libname, nid);
			if (function) {
				REDIRECT_FUNCTION(stub_addr, function);
				return -1;
			}
		}

		// Resolve old NIDs
		u32 new_nid = ResolveOldNIDs(libname, nid);
		if (new_nid)
			nid = new_nid;
	}

	int res = search_nid_in_entrytable(lib, nid, -1, 0);

	// Not linked yet
	if (res < 0) {
		VWRITE32(stub_addr, 0x0000054C);
		VWRITE32(stub_addr + 4, 0x00000000);

		return -1;
	}

	return res;
}

SceLibraryStubTable* sctrlHENFindImportLib(SceModule* mod, const char* library) {
	if (mod == NULL || library == NULL) {
		return NULL;
	}

	int i = 0;
	while (i < mod->stub_size) {
		SceLibraryStubTable *stub = (SceLibraryStubTable *)(mod->stub_top + i);

		if (stub->libname != NULL && strcmp(stub->libname, library) == 0) {
			return stub;
		}

		i += (stub->len * 4);
	}

	return NULL;
}

u32 sctrlFindImportByNID(SceModule * mod, const char * library, u32 nid) {
	SceLibraryStubTable * imp = sctrlHENFindImportLib(mod, library);

	if(imp != NULL) {
		int i = 0; for(; i < imp->stubcount; i++) {
			// Matching Function NID
			if(imp->nidtable[i] == nid) {
				// Return Function Stub Address
				return (u32)(imp->stubtable + 8 * i);
			}
		}
	}

	return 0;
}

u32 sctrlHENFindImportInMod(SceModule * mod, const char *library, u32 nid) {
	// Invalid Arguments
	if (mod == NULL || library == NULL) {
		return 0;
	}

	int i = 0;
	while (i < mod->stub_size) {
		SceLibraryStubTable *stub = (SceLibraryStubTable *)(mod->stub_top + i);

		if (stub->libname && strcmp(stub->libname, library) == 0) {
			u32 *table = (u32 *)stub->nidtable;

			int j;
			for (j = 0; j < stub->stubcount; j++) {
				if (table[j] == nid) {
					logmsg4("[DEBUG]: %s: %s — %s — 0x%08lX: FOUND\n", __func__, mod->modname, library, nid);
					return ((u32)stub->stubtable + (j * 8));
				}
			}
		}

		i += (stub->len * 4);
	}

	logmsg4("[DEBUG]: %s: %s — %s — 0x%08lX: NOT FOUND\n", __func__, mod->modname, library, nid);
	return 0;
}

u32 sctrlHENFindFunctionInMod(SceModule * mod, const char *library, u32 nid) {
	// Invalid Arguments
	if(mod == NULL || library == NULL) {
		return 0;
	}

	u32 new_nid = ResolveOldNIDs(library, nid);
	if (new_nid != 0) {
		nid = new_nid;
	}

	int i = 0;
	while (i < mod->ent_size) {
		SceLibraryEntryTable *entry = (SceLibraryEntryTable *)(mod->ent_top + i);

		if (!library || (entry->libname && strcmp(entry->libname, library) == 0)) {
			u32 *table = entry->entrytable;
			int total = entry->stubcount + entry->vstubcount;

			int j;
			for (j = 0; j < total; j++) {
				if (table[j] == nid) {
					logmsg4("[DEBUG]: %s: %s — %s — 0x%08lX: FOUND\n", __func__, mod->modname, library, nid);
					return table[j + total];
				}
			}
		}

		i += (entry->len * 4);
	}

	logmsg4("[DEBUG]: %s: %s — %s — 0x%08lX: NOT FOUND\n", __func__, mod->modname, library, nid);
	return 0;
}

// Replace Import Function Stub
int sctrlHookImportByNID(SceModule * mod, const char * library, u32 nid, void *func) {
	// Invalid Arguments
	if(mod == NULL || library == NULL) {
		return -1;
	}

	u32 stub = sctrlHENFindImportInMod(mod, library, nid);

	if (stub == 0) {
		logmsg2("[WARN]: %s: %s — %s — 0x%08lX: Failed to find import\n", __func__, mod->modname, library, nid);

		u32 new_nid = ResolveOldNIDs(library, nid);

		// resolver fail
		if (new_nid == 0) {
			logmsg("[ERROR]: %s: %s — %s — 0x%08lX: Failed to resolve NID\n", __func__, mod->modname, library, nid);
			return -2;
		} else {
			logmsg4("[DEBUG]: %s: %s — %s — 0x%08X: NID resolved: 0x%08lX -> 0x%08lX\n", __func__, mod->modname, library, nid, nid, new_nid);
		}
		stub = sctrlHENFindImportInMod(mod, library, new_nid);

		// Stub not found again...
		if (stub == 0) {
			logmsg("[ERROR]: %s: %s — %s — 0x%08lX: Failed to find import with resolved nid\n", __func__, mod->modname, library, nid);
			return -3;
		} else {
			logmsg3("[INFO]: %s: %s — %s — 0x%08lX: Import found\n", __func__, mod->modname, library, nid);
		}
	} else {
		logmsg3("[INFO]: %s: %s — %s — 0x%08lX: Import found\n", __func__, mod->modname, library, nid);
	}

	// Function as 32-Bit unsigned integer
	u32 func_int = (u32)func;

	// Dummy Return
	if (func_int <= 0xFFFF) {
		// Create Dummy Return
		MAKE_DUMMY_FUNCTION(stub, func_int);
		logmsg4("[DEBUG]: %s: %s — %s — 0x%08X: Made a dummy function -> 0x%08X\n", __func__, mod->modname, library, nid, func_int);
	} else { // Normal Hook
		// Syscall Hook
		if ((stub & 0x80000000) == 0 && (func_int & 0x80000000) != 0) {
			// Query Syscall Number
			int syscall = sceKernelQuerySystemCall(func);

			// Not properly exported in exports.exp
			if(syscall < 0) {
				logmsg("[ERROR]: %s: %s — %s — 0x%08lX: Syscall not found\n", __func__, mod->modname, library, nid);
				return -3;
			}

			// Create Syscall Hook
			MAKE_SYSCALL_FUNCTION(stub, syscall);
			logmsg4("[DEBUG]: %s: %s — %s — 0x%08X: Made a syscall hook\n", __func__, mod->modname, library, nid);
		} else {
			// Create Direct Jump Hook
			REDIRECT_FUNCTION(stub, func);
			logmsg4("[DEBUG]: %s: %s — %s — 0x%08X: Made a direct jump hook\n", __func__, mod->modname, library, nid);
		}
	}

	// Invalidate Cache
	sceKernelDcacheWritebackInvalidateRange((void *)stub, 8);
	sceKernelIcacheInvalidateRange((void *)stub, 8);

	logmsg("[INFO] %s: %s — %s — 0x%08lX: Hook done\n", __func__, mod->modname, library, nid);
	return 0;
}


int sctrlHENIsSystemBooted() {
	int res = sceKernelGetSystemStatus();

	return (res == 0x20000) ? 1 : 0;
}

int sctrlPatchModule(char *modname, u32 inst, u32 offset) {
	int ret = 0;

	u32 k1 = pspSdkSetK1(0);
	SceModule* mod = sceKernelFindModuleByName(modname);

	if(mod != NULL) {
		MAKE_INSTRUCTION(mod->text_addr + offset, inst);
		sctrlFlushCache();
	} else {
		ret = -1;
	}

	pspSdkSetK1(k1);

	logmsg4("%s: modname=%s, inst=0x%08X, offset=0x%08X -> 0x%08X", __func__, modname, inst, offset);
	return ret;
}

u32 sctrlModuleTextAddr(char *modname) {
	u32 text_addr = 0;
	u32 k1 = pspSdkSetK1(0);

	SceModule* mod = sceKernelFindModuleByName(modname);

	if (mod != NULL) {
		text_addr = mod->text_addr;
	}

	pspSdkSetK1(k1);
	return text_addr;
}

u32 sctrlKernelResolveNid(const char *libname, u32 nid) {
	return (u32)ResolveMissingNIDs(libname, nid);
}

int sctrlKernelSetNidResolver(char* libname, u32 enabled) {
	return SCE_ENOSYS;
}

u32 sctrlHENMakeSyscallStub(void *function) {
	SceUID block_id = sceKernelAllocPartitionMemory(PSP_MEMORY_PARTITION_USER, "", PSP_SMEM_High, 2 * sizeof(u32), NULL);
	u32 stub = (u32)sceKernelGetBlockHeadAddr(block_id);
	s32 syscall_num = sceKernelQuerySystemCall(function);
	if (stub == 0) {
		logmsg("[ERROR]: %s: No memory to create stub\n", __func__);
		return 0;
	}
	if (syscall_num < 0) {
		logmsg("[ERROR]: %s: Function not properly exported for syscalls\n", __func__);
		return 0;
	}

	MAKE_SYSCALL_FUNCTION(stub, syscall_num);
	return stub;
}

u32 sctrlHENFindFunctionOnSystem(const char *libname, u32 nid, int user_mods_only) {
	SceUID mod_list[128] = {0};
	u32 mod_count = 0;
	int res = sceKernelGetModuleIdListForKernel(mod_list, 128, &mod_count, user_mods_only);

	if (res != 0) {
		return 0;
	}

	if (mod_count > 128) {
		logmsg("[WARN]: %s: System has more than 128 modules, result will be incomplete\n", __func__);
	}

	for (int i = 0; i < MIN(mod_count, 128); i++) {
		SceUID mod_id = mod_list[i];
		SceModule* mod = sceKernelFindModuleByUID(mod_id);
		u32 fn = sctrlHENFindFunctionInMod(mod, libname, nid);

		if (fn != 0) {
			logmsg4("[DEBUG]: %s: Found %s — 0x%08lx on %s module\n", __func__, libname, nid, mod->modname);
			return fn;
		}
	}

	// Not Found
	logmsg4("[DEBUG]: %s: %s — 0x%08lx not found\n", __func__, libname, nid);
	return 0;
}

void sctrlHENHijackFunction(FunctionPatchData* patch_data, void* func_addr, void* patch_func, void** orig_func) {

	void* ptr = patch_data;

	int is_kernel_patch = IS_KERNEL_ADDR(func_addr);
	if (is_kernel_patch){
		patch_func = (void*)KERNELIFY(patch_func);
		ptr = (void*)KERNELIFY(ptr);
	}

	// do hijack
	int intc = pspSdkDisableInterrupts();
	{
		u32 _pb_ = (u32)patch_data;
		u32 a = (u32)func_addr;
		u32 f = (u32)patch_func;

		MAKE_INSTRUCTION(_pb_, VREAD32(a));
		MAKE_INSTRUCTION(_pb_ + 4, VREAD32(a + 4));
		MAKE_NOP(_pb_ + 8);
		MAKE_NOP(_pb_ + 16);
		MAKE_JUMP_PATCH(_pb_ + 12, a + 8);
		REDIRECT_FUNCTION(a, f);
	}
	pspSdkEnableInterrupts(intc);

	*orig_func = ptr;

	sctrlFlushCache();

	logmsg3("[INFO]: %s: Hijacked function 0x%p -> 0x%p\n", __func__, func_addr, patch_func);
}