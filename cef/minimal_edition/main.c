#include <pspsdk.h>
#include <pspkernel.h>
#include <pspsysevent.h>
#include <psploadexec_kernel.h>

#include <stdio.h>
#include <string.h>
//#include <systemctrl_se.h>
//#include <systemctrl.h>

PSP_MODULE_INFO("EPI-MEisoDriver", 0x1006, 2, 0);
PSP_MAIN_THREAD_ATTR(0);

extern SceUID g_umd_sema;

int sceUmd_Init();
int march_init();
void pspUmdCallback(int a0);

static int SysEventHandler(int ev_id, char* ev_name, void* param, int* result) {
	if ( ev_id == 0x400 ) {
		if (sceKernelWaitSema(g_umd_sema, 1, 0) >= 0) {
			pspUmdCallback( 0x9 );
		}
	}
	/*
	else if ( ev_id == 0x10009 )
	{
//		wait_ms_flag = 1;
//		pspUmdCallback( 0x32 );
	}
	*/
	else if ( ev_id == 0x400000 ) {
		if (sceKernelSignalSema(g_umd_sema, 1) >= 0) {
			pspUmdCallback( 0x32 );
		}
	}
	/*
	else
	{
//		printf("ev_id = 0x%08X \n", ev_id);
	}
*/
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

/*
#include <pspdisplay.h>

typedef union
{
	int rgba;
	struct
	{
		char r;
		char g;
		char b;
		char a;
	} c;
} color_t;
void SetColor(int col)
{
	int i;
	color_t *pixel = (color_t *)0x44000000;
	for (i = 0; i < 512*272; i++) {
		pixel->rgba = col;
		pixel++;
	}
}

void cls(int color)
{
    sceDisplaySetFrameBuf((void *)0x44000000, 512, PSP_DISPLAY_PIXEL_FORMAT_8888, 1);
    SetColor(color);
}

*/



int module_start(SceSize args, void *argp) {

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

//	cls(0x00FF0000);

	return 0;
}

int module_stop(void) {
	sceKernelUnregisterSysEventHandler( &event_handler );//data4B80
	sceIoDelDrv("umd");
	return 0;
}
