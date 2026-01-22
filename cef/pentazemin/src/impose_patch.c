#include <string.h>

#include <pspsdk.h>
#include <pspsysmem_kernel.h>
#include <psputilsforkernel.h>
#include <pspinit.h>

#include <systemctrl_ark.h>
#include <cfwmacros.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <systemctrl_private.h>

#include "adrenaline.h"
#include "../../adrenaline_compat.h"
#include "externs.h"


extern SEConfigARK* se_config;

////////////////////////////////////////////////////////////////////////////////
// HELPERS
////////////////////////////////////////////////////////////////////////////////
static int exit_callback(int arg1, int arg2, void *common) {
	sceKernelSuspendAllUserThreads();

	g_adrenaline->pops_mode = 0;
	SendAdrenalineCmd(ADRENALINE_VITA_CMD_RESUME_POPS, 0);

	static u32 vshmain_args[0x100];
	memset(vshmain_args, 0, sizeof(vshmain_args));

	vshmain_args[0] = sizeof(vshmain_args);
	vshmain_args[1] = 0x20;
	vshmain_args[16] = 1;

	SceKernelLoadExecVSHParam param;

	memset(&param, 0, sizeof(param));
	param.size = sizeof(param);
	param.argp = NULL;
	param.args = 0;
	param.vshmain_args = vshmain_args;
	param.vshmain_args_size = sizeof(vshmain_args);
	param.key = "vsh";

	sctrlKernelExitVSH(&param);

	return 0;
}

static int CallbackThread(SceSize args, void *argp) {
	SceUID cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
	if (cbid < 0) {
		return cbid;
	}

	int (* sceKernelRegisterExitCallback)(SceUID cbid) = (void *)sctrlHENFindFunction("sceLoadExec", "LoadExecForUser", 0x4AC57943);
	sceKernelRegisterExitCallback(cbid);

	sceKernelSleepThreadCB();

	return 0;
}

static SceUID SetupCallbacks() {
	SceUID thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0xFA0, 0, 0);
	if (thid >= 0) {
		sceKernelStartThread(thid, 0, 0);
	}
	return thid;
}

////////////////////////////////////////////////////////////////////////////////
// PATCHED IMPLEMENTATIONS
////////////////////////////////////////////////////////////////////////////////

int (* SetIdleCallback)(int flags);
int SetIdleCallbackPatched(int flags) {
	// Only allow idle callback for music player sleep-timer
	if (flags & 8) {
		return SetIdleCallback(flags);
	}

	return 0;
}

int sceKernelWaitEventFlagPatched(int evid, u32 bits, u32 wait, u32 *outBits, SceUInt *timeout) {
	int res = sceKernelWaitEventFlag(evid, bits, wait, outBits, timeout);

	if (*outBits & 0x1) {
		SendAdrenalineCmd(ADRENALINE_VITA_CMD_PAUSE_POPS, 0);
	} else if (*outBits & 0x2) {
		SendAdrenalineCmd(ADRENALINE_VITA_CMD_RESUME_POPS, 0);
	}

	return res;
}


////////////////////////////////////////////////////////////////////////////////
// MODULE PATCHERS
////////////////////////////////////////////////////////////////////////////////

void PatchImposeDriver(SceModule* mod) {
	u32 text_addr = mod->text_addr;

	// Hide volume bar
	MAKE_NOP(text_addr + 0x4AEC);

	HIJACK_FUNCTION(text_addr + 0x381C, SetIdleCallbackPatched, SetIdleCallback);

	if (sceKernelApplicationType() == PSP_INIT_KEYCONFIG_POPS) {
		SetupCallbacks();
		MAKE_DUMMY_FUNCTION(text_addr + 0x91C8, PSP_INIT_KEYCONFIG_GAME);
	}

	REDIRECT_FUNCTION(text_addr + 0x92B0, sceKernelWaitEventFlagPatched);

	sctrlFlushCache();
}