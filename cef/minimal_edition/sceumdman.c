#include <stdio.h>
#include <string.h>

#include <pspsdk.h>
#include <pspkernel.h>

// #include <systemctrl_me.h>
#include <systemctrl.h>
#include <psperror.h>
#include <cfwmacros.h>

int data2794 = 0;
void *data2798 = 0;
int data279C = 0;
void *data27A0 = 0;

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
	if (a0 != data279C) {
		return SCE_ENOENT;
	}

	data2794 = 0;
	data2798 = 0;
	data279C = 0;
	data27A0 = 0;

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
	if (data279C) {
		return SCE_ENOMEM;
	}

	data279C = id;
	GET_GP(data2794);
	data27A0 = arg;
	data2798 = callback;

	SceModule *mod = sceKernelFindModuleByName("sceIsofs_driver");
	u32 text_addr=mod->text_addr;

	//addu	$v0, $zr, $zr
	MAKE_INSTRUCTION(text_addr+0x3FEC, 0x00001021);
	MAKE_INSTRUCTION(text_addr+0x4024, 0x00001021);
	MAKE_INSTRUCTION(text_addr+0x40D8, 0x00001021);
	MAKE_INSTRUCTION(text_addr+0x42B4, 0x00001021);

	sctrlFlushCache();

	if (data2798) {
		int (* func)(int, void *, int) = (void*)data2798;

		//$gp = data2794;
		SET_GP( data2794 );

		func( data279C , data27A0 , 1);
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