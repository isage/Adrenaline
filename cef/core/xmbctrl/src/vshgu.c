/*
Copyright (c) 2025 m-c/d & Acid_Snake

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”),
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#include <pspgu.h>
#include <pspgum.h>
#include <pspsdk.h>
#include <psppower.h>
#include <pspkernel.h>
#include <pspdisplay.h>
#include <psputilsforkernel.h>
#include <psperror.h>

#include <vshctrl.h>
#include <kubridge.h>
#include <cfwmacros.h>
#include <systemctrl.h>
#include <adrenaline_log.h>

#include "xmbctrl.h"


#define BUF_WIDTH 512

static SctrlFunctionPatchData g_display_patch = { 0 };
static int (*prevDisplaySetFrameBuf)(void*, int, int, int) = NULL;

static void* g_list = NULL;

static void gu_sync();

static int vshDisplaySetFrameBuf(void *frameBuf, int bufferwidth, int pixelformat, int sync) {
    void* frame = (void*)(0x1fffffff & (u32)frameBuf);

    if (frame && g_vshmenu_running){
        int intr = sceKernelCpuSuspendIntr();
        // save context
        PspGeContext* gectx = paf_memalign(64, sizeof(PspGeContext));
		if (g_list == NULL) {
			logmsg("[ERROR]: %s: Failed to allocate `PspGeContext`\n", __func__);
			return SCE_ENOMEM;
		}
        int state = sceKernelSuspendDispatchThread();
        sceGeSaveContext(gectx);
        // draw
        g_list = paf_memalign(64, 2048);
		if (g_list == NULL) {
			logmsg("[ERROR]: %s: Failed to allocate `g_list`\n", __func__);
			return SCE_ENOMEM;
		}
        sceGuStart(GU_DIRECT, g_list);
        sceGuDrawBuffer(GU_PSM_8888, frame, BUF_WIDTH);
        if (g_vshmenu_draw){
            g_vshmenu_draw(frame);
        }

		// sync
        gu_sync();
        sceGuFinish();
        sceGuSync(GU_SYNC_FINISH, GU_SYNC_WHAT_DONE);
        // restore context
        sceKernelResumeDispatchThread(state);
        sceGeRestoreContext(gectx);
        paf_free(gectx);
        paf_free(g_list);
        sceKernelCpuResumeIntrWithSync(intr);
    }

    return prevDisplaySetFrameBuf(frameBuf, bufferwidth, pixelformat, sync);
}

int vshgu_init() {
    sceGuInit();
    sceGuDisplay(GU_FALSE);

    void* func_addr = (void*)sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0x289D82FE);
    sctrlHENHijackFunction(&g_display_patch, func_addr, vshDisplaySetFrameBuf, (void**)&prevDisplaySetFrameBuf);

    return 0;
}

static inline void gu_sync() {
	u32 a = 0xffff;
	while(--a) {
		__asm__("nop; sync");
	}
}