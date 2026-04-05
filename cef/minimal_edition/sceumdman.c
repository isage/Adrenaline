#include <stdio.h>
#include <string.h>

#include <pspsdk.h>
#include <pspkernel.h>

#include <systemctrl.h>
#include <psperror.h>
#include <cfwmacros.h>

static int g_gp_iecallback = 0;
static void *g_iecallback = 0;
static int g_id_iecallback = 0;
static void *g_arg_iecallback = 0;

//500 = sceUmdMan_driver_B54D5BE8
int sceUmdManRegisterImposeCallBack() {
	return 0;
}

//500 = sceUmdMan_driver_B9B02322
int sceUmdManUnRegisterImposeCallback() {
	return 0;
}

//reset cache read?
//500 = sceUmdMan_driver_31699C86
int sceUmdManUnRegisterInsertEjectUMDCallBack(int a0) {
	if (a0 != g_id_iecallback) {
		return SCE_ENOENT;
	}

	g_gp_iecallback = 0;
	g_iecallback = 0;
	g_id_iecallback = 0;
	g_arg_iecallback = 0;

	return 0;
}

//500= sceUmdMan_driver_988597A2
int sceUmdManIsDvdDrive() {
	return 0;
}

#define GET_GP(gp) asm volatile ("move %0, $gp\n" : "=r" (gp))
#define SET_GP(gp) asm volatile ("move $gp, %0\n" :: "r" (gp))

//500 = sceUmdMan_driver_63B69CE1
int sceUmdManRegisterInsertEjectUMDCallBack(int id,void * callback, void *arg) {
	if (g_id_iecallback) {
		return SCE_ENOMEM;
	}

	g_id_iecallback = id;
	GET_GP(g_gp_iecallback);
	g_arg_iecallback = arg;
	g_iecallback = callback;

	SceModule *mod = sceKernelFindModuleByName("sceIsofs_driver");
	u32 text_addr=mod->text_addr;

	//addu	$v0, $zr, $zr
	MAKE_INSTRUCTION(text_addr+0x3FEC, 0x00001021);
	MAKE_INSTRUCTION(text_addr+0x4024, 0x00001021);
	MAKE_INSTRUCTION(text_addr+0x40D8, 0x00001021);
	MAKE_INSTRUCTION(text_addr+0x42B4, 0x00001021);

	sctrlFlushCache();

	if (g_iecallback) {
		int (* func)(int, void *, int) = (void*)g_iecallback;

		//$gp = g_gp_iecallback;
		SET_GP( g_gp_iecallback );

		func( g_id_iecallback , g_arg_iecallback , 1);
	}

	return 0;
}


int sceUmd9660_driver_7EB57F56() {
	return 0;
}

int sceUmd9660_driver_C0933C16() {
	return 0;
}

int sceUmd9660_driver_887C3193() {
	return 0;
}

void sceUmd9660_driver_3CC9CE54(){}
void sceUmd9660_driver_FE3A8B67(){}

int sceNp9660_driver_B925CA6C() {
	return 0;
}

int sceNp9660_driver_8EF69DC6() {
	return 0;
}

int sceNp9660_driver_7A05EB3C(int *a0) {
	if ( a0 ) {
		*a0 = 0;
	}
	return 0;
}