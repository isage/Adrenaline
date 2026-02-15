#include <string.h>

#include <pspsdk.h>
#include <pspumd.h>
#include <psperror.h>
#include <pspkernel.h>
#include <pspsysmem_kernel.h>

#include <adrenaline_log.h>

#include "mediaman.h"
#include "../bits/iso_common.h"

extern int g_game_group;

static int g_drivestat = 0;
static int g_umdcallback = 0;
static int g_errorstat = 0;
static SceUID g_mediaman_sema = -1;

int sceKernelCancelSema(SceUID semaid, int newcount, int *num_wait_threads);

void UmdNotifyCallback(int stat) {
	logmsg4("[DEBUG]: %s: stat=0x%08X -> ()\n", __func__, stat);

	if (g_umdcallback >= 0) {
		sceKernelNotifyCallback(g_umdcallback, stat);
	}
}

int sceUmdActivate(const int mode, const char *aliasname) {
	int k1 = pspSdkSetK1(0);
	int res = 0;

	if (strcmp(aliasname, "disc0:") == 0) {
		u32 unk = 1;
		int report_callback = !(g_drivestat & PSP_UMD_READY);

		sceIoAssign(aliasname, "umd0:", "isofs0:", IOASSIGN_RDONLY, &unk, 4);
		g_drivestat = PSP_UMD_PRESENT | PSP_UMD_INITED | PSP_UMD_READY;

		if (g_game_group == 1) {
			UmdNotifyCallback(PSP_UMD_READY | PSP_UMD_PRESENT);
		} else {
			if (report_callback) {
				UmdNotifyCallback(g_drivestat);
			}
		}

		res = 0;
	} else {
		res = SCE_EINVAL;
	}

	pspSdkSetK1(k1);
	logmsg("%s: mode=0x%08X, aliasname=%s -> 0x%08X\n", __func__, mode, aliasname, res);
	return res;
}

int sceUmdDeactivate(const int mode, const char *aliasname) {
	int k1 = pspSdkSetK1(0);

	int res = sceIoUnassign(aliasname);

	if (res >= 0) {
		g_drivestat = PSP_UMD_PRESENT | PSP_UMD_INITED;
		UmdNotifyCallback(g_drivestat);
	}

	pspSdkSetK1(k1);
	logmsg("%s: mode=0x%08X, aliasname=%s -> 0x%08X\n", __func__, mode, aliasname, res);
	return res;
}

int sceUmdGetDiscInfo(SceUmdDiscInfo *disc_info) {
	int k1 = pspSdkSetK1(0);
	int res = 0;

	if (disc_info && disc_info->size == 8) {
		disc_info->type = PSP_UMD_TYPE_GAME;
	} else {
		res = SCE_EINVAL;
	}

	pspSdkSetK1(k1);
	logmsg("%s: disc_info=0x%p -> 0x%08X\n", __func__, disc_info, res);
	return res;
}

int sceUmdRegisterUMDCallBack(SceUID cbid) {
	int k1 = pspSdkSetK1(0);
	int res = 0;

	if (sceKernelGetThreadmanIdType(cbid) == SCE_KERNEL_TMID_Callback) {
		g_umdcallback = cbid;
		UmdNotifyCallback(g_drivestat);
	} else {
		res = SCE_EINVAL;
	}

	pspSdkSetK1(k1);
	logmsg("%s: cbid=0x%08X -> 0x%08X\n", __func__, cbid, res);
	return res;
}

int sceUmdUnRegisterUMDCallBack(SceUID cbid) {
	int k1 = pspSdkSetK1(0);
	int res = 0;
	uidControlBlock *block;

	if (sceKernelGetUIDcontrolBlock(cbid, &block) != 0 || cbid != g_umdcallback) {
		res = SCE_EINVAL;
	} else {
		g_umdcallback = -1;
	}

	pspSdkSetK1(k1);
	logmsg("%s: cbid=0x%08X -> 0x%08X\n", __func__, cbid, res);
	return res;
}

int sceUmdCheckMedium() {
	int res = 0;
	if (g_iso_fn[0] == '\0'){
		res = 0;
		goto exit;
	}

	while (!g_iso_opened) {
		sceKernelDelayThread(10000);
	}

	res = 1;

exit:
	logmsg("%s: () -> 0x%08X\n", __func__, res);
	return res;
}

int sceUmdGetErrorStat() {
	int k1 = pspSdkSetK1(0);

	int res = g_errorstat;

	pspSdkSetK1(k1);
	logmsg("%s: () -> 0x%08X\n", __func__, res);
	return res;
}

int sceUmdGetErrorStatus() {
	logmsg("%s: () -> 0x%08X\n", __func__, g_errorstat);
	return g_errorstat;
}

void sceUmdSetErrorStatus(int error) {
	logmsg("%s: 0x%08X SET.\n", __func__, error);
	g_errorstat = error;
}

int sceUmdGetDriveStat() {
	int k1 = pspSdkSetK1(0);

	int res = g_drivestat;

	pspSdkSetK1(k1);
	logmsg2("%s: () -> 0x%08X\n", __func__, res);
	return res;
}

int sceUmdGetDriveStatus(u32 status) {
	logmsg("%s: status=0x%08lX -> 0x%08X\n", __func__, status, g_drivestat);
	return g_drivestat;
}

void sceUmdClearDriveStatus(int clear) {
	g_drivestat &= clear;
	logmsg("%s: clear=0x%08X -> g_drivestat=0x%08X\n", __func__, clear, g_drivestat);
}

void sceUmdSetDriveStatus(int status) {
	logmsg("%s: status=0x%08X -> ()\n", __func__, status);

	int intr = sceKernelCpuSuspendIntr();

	if (status & PSP_UMD_NOT_PRESENT) {
		g_drivestat &= ~(PSP_UMD_PRESENT | PSP_UMD_CHANGED | PSP_UMD_INITING | PSP_UMD_INITED | PSP_UMD_READY);
	} else if (status & (PSP_UMD_PRESENT | PSP_UMD_CHANGED | PSP_UMD_INITING | PSP_UMD_INITED | PSP_UMD_READY)) {
		g_drivestat &= ~(PSP_UMD_NOT_PRESENT);
	}

	if (status & PSP_UMD_INITING) {
		g_drivestat &= ~(PSP_UMD_INITED | PSP_UMD_READY);
	} else if (status & (PSP_UMD_INITED | PSP_UMD_READY)) {
		g_drivestat &= ~(PSP_UMD_INITING);
	}

	g_drivestat |= status;

	if (g_drivestat & PSP_UMD_READY) {
		g_drivestat |= PSP_UMD_INITED;
	}

	if (g_drivestat & PSP_UMD_INITED) {
		g_drivestat |= PSP_UMD_PRESENT;
		sceUmdSetErrorStatus(0);
	}

	sceKernelCpuResumeIntr(intr);
}

static int WaitDriveStat(int stat, SceUInt timer, int cb) {
	int k1 = pspSdkSetK1(0);
	int res = 0;
	int (*wait_sema)(SceUID, int, SceUInt *);

	if (stat & (PSP_UMD_NOT_PRESENT | PSP_UMD_PRESENT | PSP_UMD_INITING | PSP_UMD_INITED | PSP_UMD_READY)) {
		if (stat & PSP_UMD_READY) {
			g_drivestat |= PSP_UMD_READY;
		} else if (stat & (PSP_UMD_PRESENT | PSP_UMD_INITED)) {
		} else if (stat & (PSP_UMD_NOT_PRESENT | PSP_UMD_INITING)) {
			wait_sema = (cb) ? sceKernelWaitSemaCB : sceKernelWaitSema;

			if (timer != 0) {
				wait_sema(g_mediaman_sema, 1, &timer);
			} else {
				wait_sema(g_mediaman_sema, 1, NULL);
			}
		}
	} else {
		res = SCE_EINVAL;
	}

	if (cb) {
		if (g_game_group == 2) {
			sceKernelCheckCallback();
		}
	}

	pspSdkSetK1(k1);

	logmsg4("[DEBUG]: %s: stat=0x%08X, timer=%d, cb=0x%08X -> 0x%08X\n", __func__, stat, timer, cb, res);
	return res;
}

int sceUmdWaitDriveStat(int stat) {
	int res = WaitDriveStat(stat, 0, 0);

	logmsg("%s: stat=0x%08X -> 0x%08X\n", __func__, stat, res);
	return res;
}

int sceUmdWaitDriveStatCB(int stat, SceUInt timer) {
	int res = WaitDriveStat(stat, timer, 1);

	logmsg("%s: stat=0x%08X, timeout=%d -> 0x%08X\n", __func__, stat, timer, res);
	return res;
}

int sceUmdWaitDriveStatWithTimer(int stat, SceUInt timer) {
	int res = WaitDriveStat(stat, timer, 0);

	logmsg("%s: stat=0x%08X, timeout=%d -> 0x%08X\n", __func__, stat, timer, res);
	return res;
}

int sceUmdCancelWaitDriveStat() {
	int k1 = pspSdkSetK1(0);

	int res = sceKernelCancelSema(g_mediaman_sema, -1, NULL);

	pspSdkSetK1(k1);
	logmsg("%s: () -> 0x%08X\n", __func__, res);
	return res;
}

int sceUmdReplaceProhibit() {
	return 0;
}

int sceUmdReplacePermit() {
	return 0;
}

int sceUmd_040A7090(int error) {
	if (error == 0) {
		return 0;
	}

	if (sceKernelGetCompiledSdkVersion() != 0) {
		return error;
	}

	if (error == SCE_ETIMEDOUT) {
		error = SCE_ERROR150_ETIMEDOUT;
	} else if (error == SCE_EADDRINUSE) {
		error = SCE_ERROR150_EADDRINUSE;
	} else if (error == SCE_ENAMETOOLONG) {
		error = SCE_ERROR150_EADDRINUSE;
	} else if (error == SCE_ECONNABORTED) {
		error = SCE_ERROR150_ECONNABORTED;
	} else if (error == SCE_ENOSYS) {
		error = SCE_ERROR150_ENOTSUP;
	} else if (error == SCE_ENOMEDIUM) {
		error = SCE_ERROR150_ENOMEDIUM;
	} else if (error == SCE_EWRONGMEDIUM) {
		error = SCE_ERROR150_EMEDIUMTYPE;
	}

	logmsg3("%s: error=0x%08X -> 0x%08X\n", __func__, error, error);
	return error;
}

int InitMediaMan() {
	g_drivestat = PSP_UMD_READY | PSP_UMD_INITED | PSP_UMD_PRESENT;
	g_umdcallback = -1;
	g_errorstat = 0;

	g_mediaman_sema = sceKernelCreateSema("MediaManSema", 0, 0, 1, NULL);

	if (g_mediaman_sema < 0) {
		return g_mediaman_sema;
	}

	logmsg4("[DEBUG]: Media man inited.\n");

	return 0;
}



