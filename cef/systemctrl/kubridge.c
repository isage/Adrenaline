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

int kuKernelGetModel() {
	return 1; // Fake slim model
}

int kuKernelSetDdrMemoryProtection(void *addr, int size, int prot) {
	int k1 = pspSdkSetK1(0);
	int res = sceKernelSetDdrMemoryProtection661(addr, size, prot);
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
	return sceKernelInitKeyConfig();
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
	int res = sceKernelLoadModuleWithApitype2661(apitype, path, flags, option);
	pspSdkSetK1(k1);
	return res;
}

SceUID kuKernelLoadModule(const char *path, int flags, SceKernelLMOption *option) {
	int k1 = pspSdkSetK1(0);
	int res = sceKernelLoadModule661(path, flags, option);
	pspSdkSetK1(k1);
	return res;
}

int kuKernelFindModuleByName(char *modname, SceModule2 *mod) {
    SceModule2 *pmod;

    if(modname == NULL || mod == NULL) {
        return -1;
    }

    pmod = (SceModule2*) sceKernelFindModuleByName(modname);
    if(pmod == NULL) {
        return -2;
    }

    memcpy(mod, pmod, sizeof(*pmod));

    return 0;
}

int kuKernelCall(void *func_addr, struct KernelCallArg *args) {
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

struct KernelCallArgExtendStack {
    struct KernelCallArg args;
    void *func_addr;
};

static int kernel_call_stack(struct KernelCallArgExtendStack *args_stack) {
    u64 ret;
    struct KernelCallArg *args;
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

int kuKernelCallExtendStack(void *func_addr, struct KernelCallArg *args, int stack_size) {
    u32 k1, level;
    int ret;
    struct KernelCallArgExtendStack args_stack;

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

void kuKernelIcacheInvalidateAll(void) {
    u32 k1 = pspSdkSetK1(0);
    sctrlFlushCache();
    pspSdkSetK1(k1);
}