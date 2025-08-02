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
#include <pspmodulemgr.h>

#include "main.h"

int kuKernelGetModel() {
	return 1; // Fake slim model
}

int kuKernelSetDdrMemoryProtection(void *addr, int size, int prot) {
	int k1 = pspSdkSetK1(0);
	int res = sceKernelSetDdrMemoryProtection(addr, size, prot);
	pspSdkSetK1(k1);
	return res;
}

int kuKernelGetUserLevel(void) {
	int k1 = pspSdkSetK1(0);
	int res = sceKernelGetUserLevel();
	pspSdkSetK1(k1);
	return res;
}

int kuKernelInitKeyConfig() {
	unsigned int k1 = pspSdkSetK1(0);
	int result = sceKernelInitKeyConfig();
	pspSdkSetK1(k1);
	return result;
}

int kuKernelBootFrom() {
	return sceKernelBootFrom();
}

int kuKernelInitFileName(char *filename) {
	int k1 = pspSdkSetK1(0);
	strcpy(filename, sceKernelInitFileName());
	pspSdkSetK1(k1);
	return 0;
}

int kuKernelInitApitype() {
	return sceKernelInitApitype();
}

SceUID kuKernelLoadModuleWithApitype2(int apitype, const char *path, int flags, SceKernelLMOption *option) {
 	int k1 = pspSdkSetK1(0);
	int res = sceKernelLoadModuleWithApitype2(apitype, path, flags, option);
	pspSdkSetK1(k1);
	return res;
}

SceUID kuKernelLoadModule(const char *path, int flags, SceKernelLMOption *option) {
	int k1 = pspSdkSetK1(0);
	int res = sceKernelLoadModule(path, flags, option);
	pspSdkSetK1(k1);
	return res;
}

int kuKernelFindModuleByName(char *modname, SceModule2 *mod) {
	SceModule2 *pmod;

	if(modname == NULL || mod == NULL) {
		return -1;
	}

	pmod = sceKernelFindModuleByName(modname);
	if(pmod == NULL) {
		return -2;
	}

	memcpy(mod, pmod, sizeof(*pmod));

	return 0;
}

int kuKernelFindModuleByAddress(void *addr, SceModule2 *mod) {
	SceModule2 *pmod;

	if(addr == NULL || mod == NULL) {
		return -1;
	}

	pmod = sceKernelFindModuleByAddress((u32)addr);
	if(pmod == NULL) {
		return -2;
	}

	memcpy(mod, pmod, sizeof(*pmod));

	return 0;
}

int kuKernelCall(void *func_addr, KernelCallArg *args) {
	u32 k1, level;
	u64 ret;
	u64 (*func)(u32, u32, u32, u32, u32, u32, u32, u32, u32, u32, u32, u32);

	if(func_addr == NULL || args == NULL) {
		return -1;
	}

	k1 = pspSdkSetK1(0);
	level = sctrlKernelSetUserLevel(8);
	func = func_addr;
	ret = (*func)(
		args->arg1,
		args->arg2,
		args->arg3,
		args->arg4,
		args->arg5,
		args->arg6,
		args->arg7,
		args->arg8,
		args->arg9,
		args->arg10,
		args->arg11,
		args->arg12
	);
	args->ret1 = (u32)(ret);
	args->ret2 = (u32)(ret >> 32);
	sctrlKernelSetUserLevel(level);
	pspSdkSetK1(k1);

	return 0;
}

typedef struct KernelCallArgExtendStack {
	KernelCallArg args;
	void *func_addr;
} KernelCallArgExtendStack;

static int kernel_call_stack(KernelCallArgExtendStack *args_stack) {
	u64 ret;
	KernelCallArg *args;
	int (*func)(u32, u32, u32, u32, u32, u32, u32, u32, u32, u32, u32, u32);

	args = &args_stack->args;
	func = args_stack->func_addr;
	ret = (*func)(
		args->arg1,
		args->arg2,
		args->arg3,
		args->arg4,
		args->arg5,
		args->arg6,
		args->arg7,
		args->arg8,
		args->arg9,
		args->arg10,
		args->arg11,
		args->arg12
	);
	args->ret1 = (u32)(ret);
	args->ret2 = (u32)(ret >> 32);

	return 0;
}

int kuKernelCallExtendStack(void *func_addr, KernelCallArg *args, int stack_size) {
	u32 k1, level;
	int ret;
	KernelCallArgExtendStack args_stack;

	if(func_addr == NULL || args == NULL) {
		return -1;
	}

	k1 = pspSdkSetK1(0);
	level = sctrlKernelSetUserLevel(8);
	memcpy(&args_stack.args, args, sizeof(*args));
	args_stack.func_addr = func_addr;
	ret = sceKernelExtendKernelStack(stack_size, (void*)&kernel_call_stack, &args_stack);
	sctrlKernelSetUserLevel(level);
	pspSdkSetK1(k1);

	return ret;
}

void kuKernelGetUmdFile(char *umdfile, int size) {
	strncpy(umdfile, GetUmdFile(), size);
}

// Read Dword from Kernel
u32 kuKernelPeekw(void * addr){
    return VREAD32((u32)addr);
}

// Write Dword into Kernel
void kuKernelPokew(void * addr, u32 value){
	VWRITE32((u32)addr, value);
}

void * kuKernelMemcpy(void * dest, const void * src, unsigned int num) {
    unsigned int k1 = pspSdkSetK1(0);
    void * address = memcpy(dest, src, num);
    pspSdkSetK1(k1);
    return address;
}

void kuKernelIcacheInvalidateAll(void) {
	u32 k1 = pspSdkSetK1(0);
	sctrlFlushCache();
	pspSdkSetK1(k1);
}