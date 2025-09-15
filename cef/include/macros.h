/*
	Adrenaline Helper Macros
	Copyright (C) 2016-2018, TheFloW
	Copyright (C) 2024-2025, isage
	Copyright (C) 2025, GrayJack

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

#ifndef __ADRN_MACROS_H__
#define __ADRN_MACROS_H__

#include <psptypes.h>

#define PBP_MAGIC 0x50425000
#define ELF_MAGIC 0x464C457F
#define PSP_MAGIC 0x5053507E
#define BTCNF_MAGIC 0x0F803001
#define SFO_MAGIC 0x46535000

#define FW_661 0x06060110
#define FW_660 0x06060010
#define FW_639 0x06030910
#define FW_635 0x06030510
#define FW_620 0x06020010
#define FW_610 0x06010010
#define FW_600 0x06000010
#define FW_551 0x05050110
#define FW_550 0x05050010
#define FW_503 0x05000310
#define FW_501 0x05000110
#define FW_500 0x05000010
#define FW_401 0x04000110
#define FW_396 0x03090610
#define FW_395 0x03090510
#define FW_393 0x03090310
#define FW_390 0x03090010
#define FW_380 0x03080010
#define FW_372 0x03070210
#define FW_371 0x03070110
#define FW_352 0x03050210
#define FW_351 0x03050110
#define FW_350 0x03050010
#define FW_340 0x03040010
#define FW_330 0x03030010
#define FW_311 0x03010110
#define FW_310 0x03010010
#define FW_303 0x03000310
#define FW_302 0x03000210

#define FW_TO_FIRMWARE(f) ((((f >> 8) & 0xF) << 24) | (((f >> 4) & 0xF) << 16) | ((f & 0xF) << 8) | 0x10)
#define FIRMWARE_TO_FW(f) ((((f >> 24) & 0xF) << 8) | (((f >> 16) & 0xF) << 4) | ((f >> 8) & 0xF))
#define ALIGN(x, align) (((x) + ((align) - 1)) & ~((align) - 1))
#define PTR_ALIGN_64(p) ((void*) ALIGN((u32)p, 64))
#define NELEMS(a) (sizeof(a) / sizeof(a[0]))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

// Volatile read a u8 value from the address `addr`
#define VREAD8(addr) _lb(addr)
// Volatile read a u16 value from the address `addr`
#define VREAD16(addr) _lh(addr)
// Volatile read a u32 value from the address `addr`
#define VREAD32(addr) _lw(addr)
// Volatile read a u64 value from the address `addr`
#define VREAD64(addr) _ld(addr)

// Volatile write to `addr` the byte value of `val`
#define VWRITE8(addr, val) _sb(val, addr);
// Volatile write to `addr` the u16 value of `val`
#define VWRITE16(addr, val) _sh(val, addr);
// Volatile write to `addr` the u32 value of `val`
#define VWRITE32(addr, val) _sw(val, addr);
// Volatile write to `addr` the u64 value of `val`
#define VWRITE64(addr, val) _sd(val, addr);


// nop instruction value
#define NOP 0

// Jump to `ra` register (`jr ra`) instruction value
#define JR_RA 0x03E00008

// Move from `zr` zero register to `v0` register (`move v0, zr`) instruction value
#define MOVE_V0_ZR 0x00001021

// Creates a load immediate (`n`) to v0 register instruction value
#define LI_V0(n) ((0x2402 << 16) | ((n) & 0xFFFF))

// Is the instruction value `i` a `jal` instruction?
#define IS_JAL(i) ((((u32)i) & 0xFC000000) == 0x0C000000)

// Kernelify Address
#define KERNELIFY(f) (0x80000000 | ((u32)(f)))

// Create jump to `f` instruction value (j addr)
#define JUMP(f) (0x08000000 | (((u32)(f) >> 2) & 0x03ffffff))

// Create a jump and link to `f` instruction value (jal addr)
//
// This is the instruction you want to create a usual function call instruction
// If you want a direct jump to an addr, look for `JUMP` macro
#define JAL(f) (0x0C000000 | (((u32)(f) >> 2) & 0x03ffffff))

// Creates the target value to use in a jump instruction
//
// For kernel range, use in combination with `KERNELIFY`.
// It works with `j` and `jal` instructions
#define JUMP_TARGET(f) (((u32)(f) & 0x03ffffff) << 2)

// Create a syscall number for a given `n`
#define SYSCALL(n) ((n << 6)|0x0000000C)

// Insert a instruction optcode `i` at the address `a`
#define MAKE_INSTRUCTION(a, i) _sw(i, a);

// Insert a no operation (`nop`) instruction at the address `a`
#define MAKE_NOP(a) _sw(NOP, a);

// Insert a direct jump to `f` at the address `a`
#define MAKE_JUMP(a, f) _sw(JUMP(f), a);

// Insert a function call (`jal`) to `f` at the address `a`
#define MAKE_CALL(a, f) _sw(JAL(f), a);

// Insert a syscall call of value `n` at the address `a`
#define MAKE_SYSCALL_FUNCTION(a, n) \
{ \
	u32 _func_ = a; \
	_sw(JR_RA, _func_); \
	_sw(SYSCALL(n), _func_ + 4); \
}

// Insert a syscall call of syscall exported function `f` at the address `a`
#define REDIRECT_SYSCALL(a, f) \
{ \
    _sw(JR_RA, a); \
    _sw(SYSCALL(sceKernelQuerySystemCall(f)), a + 4); \
}

// Insert a direct jump to `f` at the address `a`, followed by a NOP.
#define REDIRECT_FUNCTION(a, f) \
{ \
	u32 _func_ = a; \
	_sw(JUMP(f), _func_); \
	_sw(NOP, _func_ + 4); \
}

// Insert a dummy function call at the address `a` with return value `r`
//
// This uses `move` with `$zr` on `r==0`.
#define MAKE_DUMMY_FUNCTION_ZR(a, r) \
{ \
	u32 _func_ = a; \
	if (r == 0) { \
		_sw(JR_RA, _func_); \
		_sw(MOVE_V0_ZR, _func_ + 4); \
	} else { \
		_sw(JR_RA, _func_); \
		_sw(LI_V0(r), _func_ + 4); \
	} \
}

// Insert a dummy function call at the address `a` with return value `r`
#define MAKE_DUMMY_FUNCTION(f, ret) \
{ \
	u32 _func_ = f; \
	_sw(JR_RA, _func_); \
    _sw(LI_V0(ret), _func_ + 4); \
}

#define MAKE_JUMP_PATCH(a, f) _sw(0x08000000 | (((u32)(f) & 0x0FFFFFFC) >> 2), a);

// Hijack the function at the address `a`, replacing it with the function `f`,
// and setting a trampoline pointer to the original function at `ptr`.
//
//by Davee
#define HIJACK_FUNCTION(a, f, ptr) \
{ \
	static u32 _pb_[5]; \
	_sw(_lw((u32)(a)), (u32)_pb_); \
	_sw(_lw((u32)(a) + 4), (u32)_pb_ + 4);\
	_sw(NOP, (u32)_pb_ + 8);\
	_sw(NOP, (u32)_pb_ + 16);\
	MAKE_JUMP_PATCH((u32)_pb_ + 12, (u32)(a) + 8); \
	MAKE_JUMP((u32)a, f) \
	_sw(0, (u32)(a) + 4); \
	ptr = (void *)_pb_; \
}

//by Bubbletune
#define U_EXTRACT_IMPORT(x) ((((u32)_lw((u32)x)) & ~0x08000000) << 2)
#define K_EXTRACT_IMPORT(x) (((((u32)_lw((u32)x)) & ~0x08000000) << 2) | 0x80000000)
#define U_EXTRACT_CALL(x) ((((u32)_lw((u32)x)) & ~0x0C000000) << 2)
#define K_EXTRACT_CALL(x) (((((u32)_lw((u32)x)) & ~0x0C000000) << 2) | 0x80000000)

#define K_HIJACK_CALL(a, f, ptr) \
{ \
	ptr = (void *)K_EXTRACT_CALL(a); \
	MAKE_CALL(a, f); \
}


#endif // __ADRN_MACROS_H__