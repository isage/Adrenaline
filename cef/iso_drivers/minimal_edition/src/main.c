#include <stdio.h>
#include <string.h>

#include <pspsdk.h>
#include <pspkernel.h>
#include <pspsysevent.h>
#include <psploadexec_kernel.h>

#define _ADRENALINE_LOG_IMPL_
#include <adrenaline_log.h>

#include "externs.h"

PSP_MODULE_INFO("EPI-MEisoDriver", 0x1006, 2, 0);
PSP_MAIN_THREAD_ATTR(0);

static int SysEventHandler(int ev_id, char* ev_name, void* param, int* result) {
	if (ev_id == 0x400 ) {
		if (sceKernelWaitSema(g_umd_sema, 1, 0) >= 0) {
			pspUmdCallback( 0x9 );
		}
	} else if (ev_id == 0x400000 ) {
		if (sceKernelSignalSema(g_umd_sema, 1) >= 0) {
			pspUmdCallback( 0x32 );
		}
	}

	return 0;
}

static PspSysEventHandler event_handler = {
	sizeof(PspSysEventHandler),
	"meIsoSysEvent",
	0x00FFFF00,
	SysEventHandler
};

static int sub_000013E0() {
	sceKernelRegisterSysEventHandler( &event_handler );
	return 0;
}

int module_start(SceSize args, void *argp) {
	logInit("ms0:/log_me-iso.txt");
	logmsg("Minimal Edition ISO driver started...\n")

//	cls(0x000000ff);

	int r = sub_000013E0();
	if (r < 0) {
		return r;
	}

	r = march_init();
	if (r < 0) {
		return r;
	}

	r = sceUmd_Init();//sub_00001514
	if (r < 0) {
		return r;
	}

	return 0;
}

int module_stop(void) {
	sceKernelUnregisterSysEventHandler( &event_handler );//data4B80
	sceIoDelDrv("umd");
	return 0;
}
