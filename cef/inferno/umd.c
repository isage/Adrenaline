/*
 * This file is part of PRO CFW.

 * PRO CFW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * PRO CFW is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PRO CFW. If not, see <http://www.gnu.org/licenses/ .
 */

#include <string.h>

#include <pspreg.h>
#include <pspumd.h>
#include <psprtc.h>
#include <psperror.h>
#include <pspsysmem_kernel.h>
#include <pspkernel.h>
#include <pspthreadman_kernel.h>

#include <cfwmacros.h>
#include <systemctrl.h>
#include <systemctrl_se.h>

#include <adrenaline_log.h>

#include "inferno.h"

extern int sceKernelGetCompiledSdkVersion(void);
extern int sceKernelCancelEventFlag(SceUID evf, SceUInt new_value, int *num_wait_threads);

static void do_umd_notify(int arg);

int g_umd_error_status = 0;
int g_drive_status = 0;

u32 g_prev_gp = 0;
int (*g_ie_callback)(int, void*, int) = NULL;
u32 g_ie_callback_id = 0;
void* g_ie_callback_arg = NULL;

SceUID g_umd_cbid = 0;
SceUID g_drive_status_evf = -1;
int g_disc_type = PSP_UMD_TYPE_GAME;

extern int sceKernelCancelSema(SceUID semaid, int newcount, int *num_wait_threads);

static int check_memory(const void *addr, int size) {
	u32 k1 = pspSdkGetK1();
	const void *end_addr = addr + size - 1;

	if ((int)(((u32)end_addr | (u32)addr) & (k1 << 11)) < 0) {
		return 0;
	}

	return 1;
}

int sceUmdCheckMedium(void) {
	if (g_iso_fn[0] == '\0'){
		return 0;
	}

	while (!g_iso_opened) {
		sceKernelDelayThread(10000);
	}

	int ret = 1;
	logmsg("%s: () -> 0x%08X\n", __func__, ret);
	return ret;
}

int sceUmdReplacePermit(void) {
	int ret = 0;
	logmsg("%s: -> 0x%08X\n", __func__, ret);
	return ret;
}

int sceUmdReplaceProhibit(void) {
	int ret = 0;
	logmsg("%s: -> 0x%08X\n", __func__, ret);
	return ret;
}

// 0x00001A14
static void do_umd_notify(int arg) {
	if (g_umd_cbid < 0) {
		return;
	}

	sceKernelNotifyCallback(g_umd_cbid, arg);
}

int sceUmdRegisterUMDCallBack(int cbid) {
	u32 k1 = pspSdkSetK1(0);
	int ret = sceKernelGetThreadmanIdType(cbid);

	if (ret != SCE_KERNEL_TMID_Callback) {
		ret = SCE_EINVAL;
		goto exit;
	}

	int intr = sceKernelCpuSuspendIntr();
	g_umd_cbid = cbid;
	sceKernelCpuResumeIntr(intr);
	ret = 0;

exit:
	pspSdkSetK1(k1);

	logmsg("%s: cbid=0x%08X -> 0x%08X\n", __func__, cbid, ret);
	return ret;
}

int sceUmdUnRegisterUMDCallBack(int cbid) {
	u32 k1 = pspSdkSetK1(0);
	int ret = SCE_EINVAL;

	int intr;
	uidControlBlock *type;
	if (sceKernelGetUIDcontrolBlock(cbid, &type) == 0) {
		if (g_umd_cbid == cbid) {
			intr = sceKernelCpuSuspendIntr();
			g_umd_cbid = -1;
			sceKernelCpuResumeIntr(intr);
			ret = 0;
		}
	}

	pspSdkSetK1(k1);

	return ret;
}

#define UMD_TYPE_ALL (PSP_UMD_TYPE_GAME | PSP_UMD_TYPE_VIDEO | PSP_UMD_TYPE_AUDIO)

int infernoSetDiscType(int type) {
	int oldtype = g_disc_type;
	g_disc_type = type;

	if (type == 0 || ((type & UMD_TYPE_ALL) == UMD_TYPE_ALL)) {
		logmsg("[ERROR]: %s: Invalid UMD kind 0x%02X. Defaulting to UMD game kind (0x%02X)\n", __func__, type, PSP_UMD_TYPE_GAME);
		g_disc_type = PSP_UMD_TYPE_GAME;
	}

	return oldtype;
}

int sceUmdGetDiscInfo(pspUmdInfo *info) {
	int ret;

	if (!check_memory(info, sizeof(*info))) {
		ret = SCE_EINVAL;
		goto exit;
	}

	u32 k1 = pspSdkSetK1(0);

	if (info != NULL && sizeof(*info) == info->size) {
		logmsg4("[DEBUG]: %s: g_disc_type=0x%02X\n", __func__, g_disc_type);
		info->type = g_disc_type;
		ret = 0;
	} else {
		ret = SCE_EINVAL;
	}

	pspSdkSetK1(k1);

exit:
	logmsg("%s: disc_info=0x%p -> 0x%08X\n", __func__, info, ret);
	return ret;
}

int sceUmdCancelWaitDriveStat(void) {
	u32 k1 = pspSdkSetK1(0);
	int ret = sceKernelCancelEventFlag(g_drive_status_evf, g_drive_status, NULL);
	pspSdkSetK1(k1);

	return ret;
}

u32 sceUmdGetErrorStatus(void) {
	logmsg("%s: () -> 0x%08X\n", __func__, g_umd_error_status);
	return g_umd_error_status;
}

void sceUmdSetErrorStatus(u32 status) {
	g_umd_error_status = status;
	logmsg("%s: () -> 0x%08X\n", __func__, g_umd_error_status);
}

int sceUmdGetDriveStat(void) {
	logmsg2("%s: () -> 0x%08X\n", __func__, g_drive_status);
	return g_drive_status;
}

u32 sceUmdGetDriveStatus(u32 status) {
	logmsg("%s: status=0x%08lX -> 0x%08X\n", __func__, status, g_drive_status);
	return g_drive_status;
}

int sceUmdManRegisterImposeCallBack(void) {
	int ret = 0;
	logmsg("%s: () -> 0x%08X\n", __func__, ret);
	return ret;
}

int sceUmdManUnRegisterImposeCallBack(void) {
	int ret = 0;
	logmsg("%s: () -> 0x%08X\n", __func__, ret);
	return ret;
}

int sceUmdManUnRegisterInsertEjectUMDCallBack(u32 id) {
	if (g_ie_callback_id != id) {
		return SCE_ENOENT;
	}

	g_prev_gp = 0;
	g_ie_callback = NULL;
	g_ie_callback_id = 0;
	g_ie_callback_arg = NULL;

	return 0;
}

int sceUmdManIsDvdDrive(void) {
	int ret = 0;
	logmsg("%s: () -> 0x%08X\n", __func__, ret);
	return ret;
}

static inline u32 get_gp(void) {
	u32 gp;

	__asm__ volatile ("move %0, $gp" : "=r"(gp));

	return gp;
}

static inline void set_gp(u32 gp) {
	__asm__ volatile ("move $gp, %0" : :"r"(gp));
}

// for now 6.20/6.35 share the same patch
int sceUmdManRegisterInsertEjectUMDCallBack(u32 id, void* callback, void* arg) {
	int res = 0;
	if (0 != g_ie_callback_id) {
		res = SCE_ENOMEM;
		goto exit;
	}

	g_ie_callback_id = id;
	g_prev_gp = get_gp();
	g_ie_callback_arg = arg;
	g_ie_callback = callback;

	SceModule* mod = (SceModule*)sceKernelFindModuleByName("sceIsofs_driver");
	u32 text_addr = mod->text_addr;
	u32 intr = LI_V0(0);

	int patches = 4;
	u32 top_addr = text_addr+mod->text_size;
	for (u32 addr=text_addr; addr<top_addr && patches; addr+=4){
		u32 data = VREAD32(addr);
		if (data == 0x86430048){
			MAKE_INSTRUCTION(addr-16, intr);
			patches--;
		}
		else if (data == 0x3C147FDE){
			MAKE_INSTRUCTION(addr+8, intr);
			patches--;
		}
		else if (data == 0x8D240018){
			MAKE_INSTRUCTION(addr+4, intr);
			patches--;
		}
		else if (data == 0x34C30016){
			MAKE_INSTRUCTION(addr-16, intr);
			patches--;
		}
	}
	sctrlFlushCache();

	if (NULL == g_ie_callback) {
		res = 0;
		goto exit;
	}

	set_gp(g_prev_gp);
	(*g_ie_callback)(g_ie_callback_id, g_ie_callback_arg, 1);

exit:
	logmsg("%s: id=0x%08lX, cb=0x%p, arg=0x%p -> 0x%08X\n", __func__, id, callback, arg, res);
	return res;
}

// 0x000014F0
void sceUmdClearDriveStatus(u32 mask) {
	int intr = sceKernelCpuSuspendIntr();
	sceKernelClearEventFlag(g_drive_status_evf, mask);
	g_drive_status &= mask;
	sceKernelCpuResumeIntr(intr);
	do_umd_notify(g_drive_status);
}

int sceUmd9660_driver_63342C0F(void) {
	int ret = 0;
	logmsg("%s: -> 0x%08X\n", __func__, ret);
	return ret;
}

int sceUmd9660_driver_6FFFEE54(void) {
	int ret = 0;
	logmsg("%s: -> 0x%08X\n", __func__, ret);
	return ret;
}

int sceUmd9660_driver_7CB291E3(void) {
	int ret = 0;
	logmsg("%s: -> 0x%08X\n", __func__, ret);
	return ret;
}

int sceUmdWaitDriveStatWithTimer(int stat, SceUInt timeout) {
	int ret = SCE_EINVAL;
	if (!(stat & (PSP_UMD_NOT_PRESENT | PSP_UMD_PRESENT | PSP_UMD_INITING | PSP_UMD_INITED | PSP_UMD_READY))) {
		ret = SCE_EINVAL;
		goto exit;
	}

	u32 result;
	u32 k1 = pspSdkSetK1(0);
	ret = sceKernelWaitEventFlag(g_drive_status_evf, stat, PSP_EVENT_WAITOR, &result, timeout == 0 ? NULL : &timeout);
	pspSdkSetK1(k1);

exit:
	logmsg("%s: stat=0x%08X, timeout=%d -> 0x%08X\n", __func__, stat, timeout, ret);
	return ret;
}

int sceUmdWaitDriveStatCB(int stat, SceUInt timeout) {
	int ret = SCE_EINVAL;
	if (!(stat & (PSP_UMD_NOT_PRESENT | PSP_UMD_PRESENT | PSP_UMD_INITING | PSP_UMD_INITED | PSP_UMD_READY))) {
		ret = SCE_EINVAL;
		goto exit;
	}

	u32 result;
	u32 k1 = pspSdkSetK1(0);
	ret = sceKernelWaitEventFlagCB(g_drive_status_evf, stat, PSP_EVENT_WAITOR, &result, timeout == 0 ? NULL : &timeout);
	pspSdkSetK1(k1);

exit:
	logmsg("%s: stat=0x%08X, timeout=%d -> 0x%08X\n", __func__, stat, timeout, ret);
	return ret;
}

int sceUmdWaitDriveStat(int stat) {
	int ret = 0;

	if (!(stat & (PSP_UMD_NOT_PRESENT | PSP_UMD_PRESENT | PSP_UMD_INITING | PSP_UMD_INITED | PSP_UMD_READY))) {
		ret = SCE_EINVAL;
		goto exit;
	}

	u32 result;
	u32 k1 = pspSdkSetK1(0);
	ret = sceKernelWaitEventFlag(g_drive_status_evf, stat, PSP_EVENT_WAITOR, &result, NULL);
	pspSdkSetK1(k1);

exit:
	logmsg("%s: stat=0x%08X -> 0x%08X\n", __func__, stat, ret);
	return ret;
}

int sceUmdActivate(int unit, const char* drive) {
	int res = SCE_EINVAL;
	if (!g_iso_opened) {
		res = SCE_EINVAL;
		goto exit;
	}

	if (drive == NULL || !check_memory(drive, strlen(drive) + 1)) {
		res = SCE_EINVAL;
		goto exit;
	}

	u32 k1 = pspSdkSetK1(0);

	if (0 != strcmp(drive, "disc0:")) {
		pspSdkSetK1(k1);

		res = SCE_EINVAL;
		goto exit;
	}

	int value = 1;
	sceIoAssign(drive, "umd0:", "isofs0:", 1, &value, sizeof(value));
	sceUmdSetDriveStatus(PSP_UMD_PRESENT | PSP_UMD_INITED | PSP_UMD_READY);

	if (g_game_fix_type == 1) {
		do_umd_notify(PSP_UMD_PRESENT | PSP_UMD_READY);
		pspSdkSetK1(k1);

		res = 0;
		goto exit;
	}

	if (g_drive_status & PSP_UMD_READY) {
		pspSdkSetK1(k1);

		res = 0;
		goto exit;
	}

	do_umd_notify(g_drive_status);
	pspSdkSetK1(k1);

exit:
	logmsg("%s: unit=0x%08X, drive=%s -> 0x%08X\n", __func__, unit, drive, res);
	return res;
}

int sceUmdDeactivate(int unit, const char *drive) {
	if (drive == NULL || !check_memory(drive, strlen(drive) + 1)) {
		return SCE_EINVAL;
	}

	u32 k1 = pspSdkSetK1(0);
	int ret = sceIoUnassign(drive);

	if (ret < 0) {
		pspSdkSetK1(k1);

		return ret;
	}

	sceUmdSetDriveStatus(PSP_UMD_PRESENT | PSP_UMD_INITED);
	pspSdkSetK1(k1);

	return ret;
}

int sceUmdGetErrorStat(void) {
	u32 k1 = pspSdkSetK1(0);
	int ret = g_umd_error_status;
	pspSdkSetK1(k1);

	return ret;
}

// 0x000018A4
// call @EPI-InfernoDriver:sceUmd,0xF60013F8@
void sceUmdSetDriveStatus(int status) {
	int intr = sceKernelCpuSuspendIntr();

	if (!(status & PSP_UMD_NOT_PRESENT)) {
		if (status & (PSP_UMD_PRESENT | PSP_UMD_CHANGED | PSP_UMD_INITING | PSP_UMD_INITED | PSP_UMD_READY)) {
			g_drive_status &= ~PSP_UMD_NOT_PRESENT;
		}
	} else {
		g_drive_status &= ~(PSP_UMD_PRESENT | PSP_UMD_CHANGED | PSP_UMD_INITING | PSP_UMD_INITED | PSP_UMD_READY);
	}

	if (!(status & PSP_UMD_INITING)) {
		if (status & (PSP_UMD_INITED | PSP_UMD_READY)) {
			g_drive_status &= ~PSP_UMD_INITING;
		}
	} else {
		g_drive_status &= ~(PSP_UMD_INITED | PSP_UMD_READY);
	}

	if (!(status & PSP_UMD_READY)) {
		g_drive_status &= ~PSP_UMD_READY;
	}

	g_drive_status |= status;

	if (g_drive_status & PSP_UMD_READY) {
		g_drive_status |= PSP_UMD_INITED;
	}

	if (g_drive_status & PSP_UMD_INITED) {
		g_drive_status |= PSP_UMD_PRESENT;
		sceUmdSetErrorStatus(0);
	}

	sceKernelSetEventFlag(g_drive_status_evf, g_drive_status);
	sceKernelCpuResumeIntr(intr);
	do_umd_notify(g_drive_status);
}

int sceUmd_040A7090(int orig_error_code) {
	u32 compiled_version;
	int error_code = orig_error_code;

	if (error_code == 0) {
		goto exit;
	}

	compiled_version = sceKernelGetCompiledSdkVersion();

	if (compiled_version == 0) {
		if (error_code == SCE_ETIMEDOUT) {
			error_code = SCE_ERROR150_ETIMEDOUT;
		} else if (error_code == SCE_EADDRINUSE) {
			error_code = SCE_ERROR150_EADDRINUSE;
		} else if (error_code == SCE_ENAMETOOLONG) {
			error_code = SCE_ERROR150_EADDRINUSE;
		} else if (error_code == SCE_ECONNABORTED) {
			error_code = SCE_ERROR150_ECONNABORTED;
		} else if (error_code == SCE_ENOSYS) {
			error_code = SCE_ERROR150_ENOTSUP;
		} else if (error_code == SCE_ENOMEDIUM) {
			error_code = SCE_ERROR150_ENOMEDIUM;
		} else if (error_code == SCE_EWRONGMEDIUM) {
			error_code = SCE_ERROR150_EMEDIUMTYPE;
		}
	}

exit:
	logmsg("%s: error=0x%08X -> 0x%08X\n", __func__, orig_error_code, error_code);
	return error_code;
}

static u32 g_suspend_resume_mode = 0;

/* Used by vshbridge */
u32 sceUmdGetSuspendResumeMode(void) {
	return g_suspend_resume_mode;
}

void sceUmdSetSuspendResumeMode(u32 mode) {
	g_suspend_resume_mode = mode;
}

int power_event_handler(int ev_id, char *ev_name, void *param, int *result) {
	if (ev_id == 0x400 || ev_id == 0x40000) { // melt
		do_umd_notify(PSP_UMD_INITING  | PSP_UMD_NOT_PRESENT);
	}

	if (ev_id == 0x400000) { // resume complete
		do_umd_notify(PSP_UMD_READY | PSP_UMD_INITED | PSP_UMD_PRESENT);
	}

	return 0;
}
