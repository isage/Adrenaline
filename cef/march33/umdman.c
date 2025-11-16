#include <pspsdk.h>
#include <psperror.h>
#include <pspkernel.h>

#include <systemctrl.h>
#include <cfwmacros.h>

#include <adrenaline_log.h>

#include "umdman.h"

static int g_id_iecallback;
static u32 g_gp_iecallback;
static void *g_arg_iecallback;
static int (* g_iecallback)(int id, void *arg, int unk);

int sceUmdManRegisterImposeCallBack(int id, void *callback) {
	return 0;
}

int sceUmdManUnRegisterImposeCallback(int id) {
	return 0;
}

static void NotifyInsertEjectCallback(int u) {
	if (g_iecallback) {
		asm("lw $gp, 0(%0)\n" :: "r"(&g_gp_iecallback));
		g_iecallback(g_id_iecallback, g_arg_iecallback, u);
	}
}

int sceUmdManRegisterInsertEjectUMDCallBack(int id, void *callback, void *arg) {
	int res = 0;

	if (g_id_iecallback != 0) {
		res = SCE_ENOMEM;
		goto out;
	}

	g_id_iecallback = id;
	asm("sw $gp, 0(%0)\n" :: "r"(&g_gp_iecallback));
	g_arg_iecallback = arg;
	g_iecallback = callback;

	u32 *mod =  (u32 *)sceKernelFindModuleByName("sceIsofs_driver");
	u32 text_addr = *(mod+27);

	MAKE_INSTRUCTION(text_addr+0x3FEC, 0x00001021);
	MAKE_INSTRUCTION(text_addr+0x4024, 0x00001021);
	MAKE_INSTRUCTION(text_addr+0x40D8, 0x00001021);
	MAKE_INSTRUCTION(text_addr+0x42B4, 0x00001021);

	sctrlFlushCache();

	NotifyInsertEjectCallback(1);

out:
	logmsg("%s: id=0x%08X, cb=0x%p, arg=0x%p -> 0x%08X\n", __func__, id, callback, arg, res);
	return res;
}

int sceUmdManUnRegisterInsertEjectUMDCallBack(int id) {
	int res = 0;
	if (id != g_id_iecallback) {
		res = SCE_ENOENT;
		goto out;
	}

	g_id_iecallback = 0;
	g_gp_iecallback = 0;
	g_arg_iecallback = 0;
	g_iecallback = NULL;

out:
	logmsg("%s: id=0x%08X -> 0x%08X\n", __func__, id, res);
	return res;
}

int sceUmdManIsDvdDrive() {
	return 0;
}

int InitUmdMan() {
	return 0;
}
