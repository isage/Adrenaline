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
#include "libraries_patch.h"
#include "string_clone.h"

#include "nid_table.h"

int (* aLinkLibEntries)(void *lib);
int (* search_nid_in_entrytable)(void *lib, u32 nid, int unk, int nidSearchOption);

#define N_MISSING_NID(x) (sizeof(x) / sizeof(MissingNid))
#define N_MISSING_NID_LIST (sizeof(missing_nid_list) / sizeof(MissingNidList))

MissingNid SysclibForKernel_nids[] = {
	{ 0x1AB53A58, strtok_r_clone },
	{ 0x87F8D2DA, strtok_clone },
	{ 0x1D83F344, atob_clone },
	{ 0x62AE052F, strspn_clone },
	{ 0x89B79CB1, strcspn_clone },
	{ 0xD3D1A3B9, strncat_clone },
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

u32 sctrlHENFindFunction(const char *szMod, const char *szLib, u32 nid) {
	SceModule2 *mod = sceKernelFindModuleByName661(szMod);
	if (!mod) {
		mod = sceKernelFindModuleByAddress661((SceUID)szMod);
		if (!mod)
			return 0;
	}

	u32 new_nid = ResolveOldNIDs(szLib, nid);
	if (new_nid)
		nid = new_nid;

	int i = 0;
	while (i < mod->ent_size) {
		SceLibraryEntryTable *entry = (SceLibraryEntryTable *)(mod->ent_top + i);

        if (!szLib || (entry->libname && strcmp(entry->libname, szLib) == 0)) {
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

u32 sctrlHENFindImport(const char *szMod, const char *szLib, u32 nid) {
	SceModule2 *mod = sceKernelFindModuleByName661(szMod);
	if (!mod) {
		mod = sceKernelFindModuleByAddress661((SceUID)szMod);
		if (!mod)
			return 0;
	}
/*
	u32 new_nid = ResolveOldNIDs(szLib, nid);
	if (new_nid)
		nid = new_nid;
*/
	int i = 0;
	while (i < mod->stub_size) {
		SceLibraryStubTable *stub = (SceLibraryStubTable *)(mod->stub_top + i);

		if (stub->libname && strcmp(stub->libname, szLib) == 0) {
			u32 *table = (u32 *)stub->nidtable;

			int j;
			for (j = 0; j < stub->stubcount; j++) {
				if (table[j] == nid) {
					return ((u32)stub->stubtable + (j * 8));
				}
			}
		}

		i += (stub->len * 4);
	}

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

	u32 module_sdk_version = FindProc((char *)original_stub, NULL, 0x11B97506);
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

u32 sctrlHENFindFunctionInMod(SceModule2 * mod, const char *szLib, u32 nid) {
	int i = 0;
	while (i < mod->stub_size) {
		SceLibraryStubTable *stub = (SceLibraryStubTable *)(mod->stub_top + i);

		if (stub->libname && strcmp(stub->libname, szLib) == 0) {
			u32 *table = (u32 *)stub->nidtable;

			int j;
			for (j = 0; j < stub->stubcount; j++) {
				if (table[j] == nid) {
					return ((u32)stub->stubtable + (j * 8));
				}
			}
		}

		i += (stub->len * 4);
	}

	return 0;
}

// Replace Import Function Stub
int sctrlHENHookImportByNID(SceModule2 * pMod, char * library, u32 nid, void *func, int dummy) {
	// Invalid Arguments
	if(pMod == NULL || library == NULL) {
		return -1;
	}

	u32 stub = sctrlHENFindFunctionInMod(pMod, library, nid);

	if (stub == 0) {

		u32 old_nid = ResolveOldNIDs(library, nid);

		// resolver fail
		if (old_nid == 0) {
			return -2;
		}

		stub = sctrlHENFindFunctionInMod(pMod, library, old_nid);

		// Stub not found again...
		if (stub == 0) {
			return -3;
		}
	}

	// Function as 32-Bit Unsigned Integer
	unsigned int func_int = (unsigned int)func;

	// Dummy Return
	if (func == NULL) {
		// Create Dummy Return
		MAKE_DUMMY_FUNCTION(stub, dummy);
	} else if (func_int <= 0xFFFF) {
		// Create Dummy Return
		MAKE_DUMMY_FUNCTION(stub, func_int);
	} else { // Normal Hook
		// Syscall Hook
		if ((stub & 0x80000000) == 0 && (func_int & 0x80000000) != 0) {
			if (dummy) {
				void *ptr = NULL;
				asm("cfc0 %0, $12\n" : "=r"(ptr));

				// Syscall table not found.
				if (NULL == ptr) {
					return -4;
				}

				u32 *syscall_table = (u32 *)(ptr + 0x10);
				int found = 0;
				for (int i = 0; i < 0x1000; i++) {
					if ((syscall_table[i] & 0x0FFFFFFF) == (stub & 0x0FFFFFFF)) {
						syscall_table[i] = func_int;
						found = 1;
					}
				}

				// Syscall not found in the table
				if (!found) {
					return -5;
				}
			} else {
				// Query Syscall Number
				int syscall = sceKernelQuerySystemCall661(func);

				// Not properly exported in exports.exp
				if(syscall < 0) return -3;

				// Create Syscall Hook
				MAKE_SYSCALL_FUNCTION(stub, syscall);
			}
		} else {
			// Create Direct Jump Hook
			REDIRECT_FUNCTION(stub, func);
		}
	}

	// Invalidate Cache
	sceKernelDcacheWritebackInvalidateRange((void *)stub, 8);
	sceKernelIcacheInvalidateRange((void *)stub, 8);

	return 0;
}

int sctrlHENIsSystemBooted() {
	int res = sceKernelGetSystemStatus661();

    return (res == 0x20000) ? 1 : 0;
}