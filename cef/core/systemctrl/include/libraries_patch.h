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

#ifndef __LIBRARIES_PATCH_H__
#define __LIBRARIES_PATCH_H__

/**
 * This structure represents a function stub belonging to same privilege-level
 * linked libraries, i.e. a kernel resident library linked with a kernel stub
 * library.
 */
typedef struct {
  /** The call to the imported function via a MIPS ASM Jump instruction. */
  u32 call;
  /** The delay slot belonging to the call, typically a NOP instruction. */
  u32 delay_slot;
} DirectCall;

/**
 * This structure represents a function stub belonging to different
 * privilege-level linked libraries, i.e. a kernel resident library linked with
 * a user stub library.
 */
typedef struct {
  /** The return instruction from the stub. Typically a JR $ra command. */
  u32 return_addr;
  /** The system call exception used to call the imported function. */
  u32 syscall;
} Syscall;

/**
 * This structure represents an imported function stub.
 */
typedef union {
  /** User/User or Kernel/Kernel function stub. */
  DirectCall dc;
  /** Kernel/User function stub. */
  Syscall sc;
} SceStub;

/**
 * This structure represents an imported variable stub.
 */
typedef struct {
  u32 *addr;
  /** The NID identifying the imported variable. */
  u32 nid;
} SceVariableStub;

/**
 * This structure is used to record the functions a resident library provides to
 * other modules. This entry table is used to register a resident library to the
 * system. A module can register multiple libraries and multiple libraries with
 * the same name can be in use simultaneously .
 */
typedef struct {
  /** The name of the library. */
  const char *libName; // 0
  /**
   * The version of the library. It consists of a 'major' and 'minor' field. If
   * you want to register another version of an already registered resident
   * library, make sure that the new library has a higher version than all its
   * currently registered versions.
   */
  u8 version[2]; // 4
  /** The library's attributes. One or more of ::SceLibAttr. */
  s16 attribute; // 6
  /**
   * The length of this entry table in 32-Bit words. Set this to
   * "LIBRARY_ENTRY_TABLE_NEW_LEN". Use this member when you want to iterate
   * through a list of entry tables (size = len * 4).
   */
  u8 len; // 8
  /** The number of exported variables by the resident library. */
  u8 var_stub_count; // 9
  /** The number of exported functions by the resident library. */
  u16 func_stub_count; // 10
  /**
   * Pointer to an array of NIDs, followed by an array of function- and variable
   * pointers. Each function-/variable pointer must have a NID value. These
   * arrays are used to correctly perform linking between a resident library and
   * its corresponding stub libraries.
   */
  u32 *entry_table; // 12
  /** Unknown. */
  u16 unk16; // 16
  /** Unknown. */
  u8 unk18; // 18
  /** Unknown. */
  u8 unk19; // 19
} SceResidentLibraryEntryTable;

/**
 * This structure represents the imports, provided by a resident library, that a
 * given module is using. A module can have multiple stub libraries.
 */
typedef struct {
  /** The name of the library. */
  const char *libName; // 0
  /**
   * The version of the library. It consists of a 'major' and 'minor' field. The
   * version of a stub library shouldn't be higher than the version(s) of the
   * corresponding resident library/libraries. Linking won't be performed in
   * such a case.
   */
  u8 version[2]; // 4
  /** The library's attributes. Can be set to either SCE_LIB_NO_SPECIAL_ATTR or
   * SCE_LIB_WEAK_IMPORT. */
  u16 attribute; // 6
  /**
   * The length of this entry table in 32-Bit words. Set this to either
   * "STUB_LIBRARY_ENTRY_TABLE_OLD_LEN" or "STUB_LIBRARY_ENTRY_TABLE_NEW_LEN".
   * Use this member when  you want to iterate through a list of entry tables
   * (size = len * 4).
   */
  u8 len; // 8
  /** The number of imported variables by the stub library. */
  u8 var_stub_count; // 9
  /** The number of imported functions by the stub library. */
  u16 func_stub_count; // 10
  /** Pointer to an array of NIDs containing the NIDs of the imported functions
   * and variables. */
  u32 *nid_table; // 12
  /** Pointer to an array of imported function stubs. */
  SceStub *func_stub_table; // 16
  /** Pointer to an array of imported variable stubs. */
  SceVariableStub *var_stub_table; // 20
  /** Unknown. */
  u16 unk24; // 24
} SceStubLibraryEntryTable;

typedef struct SceStubLibrary {
  /** Unknown. */
  u32 unk0; // 0
  /**
   * A linked list of stub libraries belonging to the same group, i.e. the same
   * resident library.
   */
  struct SceStubLibrary *next; // 4
  /** The name of the library. */
  const char *lib_name; // 8
  /**
   * The version of the library. This member is set by the corresponding stub
   * library entry table.
   */
  u8 version[2]; // 12
  /**
   * The library's attributes. This member is set by the corresponding stub
   * library entry table.
   */
  u16 attribute; // 14
  /** The length of the corresponding stub library entry table in 32-Bit words.
   */
  u8 stub_entry_table_len; // 16
  /**
   * The number of imported variables by the stub library. This member is set by
   * the corresponding stub library entry table.
   */
  u8 var_stub_count; // 17
  /**
   * The number of imported functions by the stub library. This member is set by
   * the corresponding stub library entry table.
   */
  u16 func_stub_count; // 18
  /** Pointer to an array of NIDs identifying the imported
   * functions/variables.*/
  u32 *nid_table; // 20
  /** Pointer to the imported function stubs. */
  SceStub *func_stub_table; // 24
  /** Pointer to the imported variable stubs. */
  SceVariableStub *var_stub_table; // 28
  /** Unknown. */
  u16 unk32; // 32
  /** Pointer to the corresponding stub library entry table. */
  SceStubLibraryEntryTable *lib_stub_table; // 36
  /**
   * The current status of a stub library (control block) in memory. One of
   * ::SceStubLibraryStatus.
   */
  u32 status; // 40
  /** Indicates whether the stub library lives in User land or Kernel land. */
  u32 is_user_lib; // 44
  /** The name of the library. */
  char *lib_name2; // 48
  /** Indicates whether the library's name is located in the heap or not. */
  u32 is_lib_name_in_heap; // 52
} SceStubLibrary;

typedef struct {
	u32 nid;
	void *function;
} MissingNid;

typedef struct {
	char *libname;
	MissingNid *nid;
	int n_nid;
} MissingNidList;

extern MissingNid LoadCoreForKernel_nids[];

extern int setjmp_clone();
extern void longjmp_clone();

extern int (* aLinkLibEntries)(void *lib);
extern int (* search_nid_in_entrytable)(void *lib, u32 nid, int unk, int nidSearchOption);

int aLinkLibEntriesPatched(SceStubLibrary *lib);
int search_nid_in_entrytable_patched(void *lib, u32 nid, void *stub, int count);

#endif