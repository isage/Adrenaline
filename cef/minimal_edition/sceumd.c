#include <stdio.h>
#include <string.h>

#include <pspsdk.h>
#include <pspkernel.h>
#include <pspthreadman_kernel.h>
#include <pspsysmem_kernel.h>
#include <pspumd.h>
#include <pspthreadman.h>

#include <psperror.h>

#include "umd9660_driver.h"
#include "../bits/iso_common.h"

int sceKernelCancelSema(int semaid, int signal, int *result);

/* UMD Info struct
typedef struct pspUmdInfo
{
	// Set to sizeof(pspUmdInfo)
	unsigned int size;
	// One or more of ::pspUmdTypes
	unsigned int type;
} pspUmdInfo;
*/

extern int g_game_group;

static int g_umd_status = 0;
static int g_umdcbid = -1;
static int g_umd_error_stat = 0;
static int g_umd_sema_id = 0;

//int event_id;

int sceUmd_Init() {
	g_umd_status = PSP_UMD_READY | PSP_UMD_INITED | PSP_UMD_PRESENT;
	g_umd_error_stat = 0;
	g_umdcbid =-1;

//	event_id = sceKernelCreateEventFlag("SceMediaManUser" , 513 , 0 , 0 );
	g_umd_sema_id = sceKernelCreateSema("MediaManSema" ,0 ,0 ,1 ,NULL);

	return (0 < g_umd_sema_id) ? 0 : g_umd_sema_id;
}

void pspUmdCallback(int a0) {
	if (g_umdcbid >= 0) {
		//SceUID 	cb,int 	arg2
		sceKernelNotifyCallback( g_umdcbid , a0);
	}
}

int sceUmdCheckMedium(void) {
	int res = 0;
	if (g_iso_fn[0] == '\0') {
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

int sceUmdGetDiscInfo(pspUmdInfo *info) {
	int k1 = pspSdkSetK1(0);
	if (info) {
		if (info->size == 8) {
			info->type = PSP_UMD_TYPE_GAME;
			pspSdkSetK1(k1);
			return 0;
		}
	}

	pspSdkSetK1(k1);
	return SCE_EINVAL;
}

int sceUmdActivate(int unit, const char *drive) {
	int k1 = pspSdkSetK1(0);
	int sp;

//	cls(0x0000FF00);

	if (strcmp(drive,"disc0:") != 0) {
		pspSdkSetK1(k1);
		return SCE_EINVAL;
	}

	sp=1;

	sceIoAssign( drive ,"umd0:","isofs0:", IOASSIGN_RDONLY ,&sp,4);

	g_umd_status = PSP_UMD_READY | PSP_UMD_INITED | PSP_UMD_PRESENT;

	if (g_game_group == 1 ) {
		pspUmdCallback( PSP_UMD_READY | PSP_UMD_PRESENT );

	} else if (!(g_umd_status & PSP_UMD_READY) ) {
		pspUmdCallback( PSP_UMD_READY | PSP_UMD_INITED | PSP_UMD_PRESENT );
	}

	pspSdkSetK1(k1);
	return 0;
}

int sceUmdDeactivate(int unit, const char *drive) {
	int k1 = pspSdkSetK1(0);

	int r =sceIoUnassign(drive);

	if (r>=0) {
		pspUmdCallback( PSP_UMD_INITED | PSP_UMD_PRESENT );
		g_umd_status = PSP_UMD_INITED | PSP_UMD_PRESENT ;
	}

	pspSdkSetK1(k1);
	return r;
}

int WaitDriveStat(int stat , SceUInt timeout , int flag) {
	int k1 = pspSdkSetK1(0);
	u32 sp = timeout;
	int ret = 0;

	if ( stat & (PSP_UMD_NOT_PRESENT | PSP_UMD_PRESENT | PSP_UMD_INITING | PSP_UMD_INITED | PSP_UMD_READY)) {
		if ( stat & PSP_UMD_READY) {
			g_umd_status |= PSP_UMD_READY;

		} else if ( !(stat & (PSP_UMD_INITED | PSP_UMD_PRESENT) )) {
			if (stat & (PSP_UMD_INITING | PSP_UMD_NOT_PRESENT)) {
				int (* WaitSema)(SceUID id, int signal, u32 * timeout);

				if (flag)
					WaitSema = (void *)sceKernelWaitSemaCB;
				else
					WaitSema = (void *)sceKernelWaitSema;

				if (sp)
					WaitSema( g_umd_sema_id , 1 , &sp);
				else
					WaitSema( g_umd_sema_id , 1 , NULL );

			}
		}
	} else {
		ret = SCE_EINVAL;
	}

	if (flag) {
		if (g_game_group == 2) {
			sceKernelCheckCallback();
		}
	}

	pspSdkSetK1(k1);
	return ret;
}

int sceUmdWaitDriveStat(int stat) {
	return WaitDriveStat(stat ,0 ,0);
}

int sceUmdWaitDriveStatWithTimer(int stat, unsigned int timeout) {
	return WaitDriveStat( stat, timeout , 0);
}

int sceUmdWaitDriveStatCB(int stat, unsigned int timeout) {
	return WaitDriveStat(stat , timeout , 1);
}

int sceUmdCancelWaitDriveStat(void) {
	int k1 = pspSdkSetK1(0);

//	int r = sceKernelCancelEventFlag( event_id , UmdStatus  , 0);
	int r =sceKernelCancelSema( g_umd_sema_id , -1 /* UmdStatus */ , NULL);

	pspSdkSetK1(k1);
	return r;

}

void sceUmdSetErrorStatus(int stat) {
	g_umd_error_stat = stat;
}

void sceUmdSetDriveStatus(int stat) {
	int intr = sceKernelCpuSuspendIntr();

	if ( stat & PSP_UMD_NOT_PRESENT) {
		g_umd_status &= ~( PSP_UMD_READY |  PSP_UMD_INITED | PSP_UMD_INITING | PSP_UMD_CHANGED | PSP_UMD_PRESENT);
	} else if (stat & (PSP_UMD_READY |  PSP_UMD_INITED | PSP_UMD_INITING | PSP_UMD_CHANGED | PSP_UMD_PRESENT)) {
		g_umd_status &= ~(PSP_UMD_NOT_PRESENT);
	}

	if (stat & PSP_UMD_INITING) {
		g_umd_status &= ~(PSP_UMD_INITED | PSP_UMD_READY);
	} else if (stat & (PSP_UMD_READY |  PSP_UMD_INITED)) {
		g_umd_status &= ~(PSP_UMD_INITING);
	}

	if ( !(stat & PSP_UMD_READY) ) {
		g_umd_status &= 0xFFFFFFDF;
	}

	// PSP_UMD_READY
	if ((stat | g_umd_status ) & PSP_UMD_READY) {
		g_umd_status |= stat;
		g_umd_status |= PSP_UMD_INITED;
	}

	// PSP_UMD_INITED
	if ( (stat|g_umd_status) & PSP_UMD_INITED) {
		g_umd_status |= PSP_UMD_PRESENT;
		sceUmdSetErrorStatus(0);
	}

//	sceKernelSetEventFlag( event_id , UmdStatus );

	sceKernelCpuResumeIntr(intr);
}

int sceUmdGetDriveStatus(void) {
	return g_umd_status;
}

int sceUmdGetDriveStat(void) {
	int k1 = pspSdkSetK1(0);
	int r = g_umd_status;
	pspSdkSetK1(k1);
	return r;
}

int sceUmdGetErrorStatus(void) {
	return g_umd_error_stat;
}

int sceUmdGetErrorStat(void) {
	int k1 = pspSdkSetK1(0);

	int r = g_umd_error_stat;
	pspSdkSetK1(k1);
	return r;
}

int sceUmdRegisterUMDCallBack(int cbid) {
	int k1 = pspSdkSetK1(0);

	int r = sceKernelGetThreadmanIdType(cbid);

	if (r == SCE_KERNEL_TMID_Callback) {
		g_umdcbid = cbid;
		pspUmdCallback(g_umd_status);

		pspSdkSetK1(k1);
		return 0;
	}

	pspSdkSetK1(k1);
	return SCE_EINVAL;
}

int sceUmdUnRegisterUMDCallBack(int cbid) {
	int k1 = pspSdkSetK1(0);
	uidControlBlock *sp;

	int r = sceKernelGetUIDcontrolBlock(cbid , &sp);
	if (r == 0) {
		if (g_umdcbid == cbid) {
			g_umdcbid = -1;
		}
	} else {
		r = SCE_EINVAL;
	}

	pspSdkSetK1(k1);
	return r;
}

int sceUmdClearDriveStatus(int clear) {
	int intr = sceKernelCpuSuspendIntr();

//	sceKernelClearEventFlag( event_id , a0);
	g_umd_status &= clear;

	sceKernelCpuResumeIntr(intr);
	return g_umd_status;
}

//sceUmd_36FF82F3:
int sceUmdReplacePermit(void) {
	return 0;
}

//sceUmd_30DCD985:
int sceUmdReplaceProhibit(void) {
	return 0;
}

int sceUmd_040A7090(int error) {
	if ( error == 0) {
		return 0;
	}

	u32 version = sceKernelGetCompiledSdkVersion();
	if (!version) {
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
	}
	return error;
}
