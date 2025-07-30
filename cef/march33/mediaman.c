#include <pspsdk.h>
#include <pspkernel.h>
#include <pspsysmem_kernel.h>

#include <stdio.h>
#include <string.h>

#include "mediaman.h"
#include "psperror.h"

int drivestat;
int umdcallback;
int errorstat;
int mediaman_sema;

extern int game_group;

static void UmdCallback(int stat)
{
	//Kprintf("umdcallback.\n");
	
	if (umdcallback >= 0)
	{
		sceKernelNotifyCallback(umdcallback, stat);
	}
}

int sceUmdActivate(const int mode, const char *aliasname)
{
	int k1 = pspSdkSetK1(0);
	int res = 0;

	//Kprintf("Activate.\n");

	if (strcmp(aliasname, "disc0:") == 0)
	{
		u32 unk = 1;
		int report_callback = !(drivestat & SCE_UMD_READABLE);		

		sceIoAssign(aliasname, "umd0:", "isofs0:", IOASSIGN_RDONLY, &unk, 4);
		drivestat = SCE_UMD_MEDIA_IN | SCE_UMD_READY | SCE_UMD_READABLE;
		
		if (game_group == 1)
		{
			UmdCallback(SCE_UMD_READABLE | SCE_UMD_MEDIA_IN);
		}
		else
		{
			if (report_callback)		
				UmdCallback(drivestat);
		}

		res = 0;
	}
	else
	{
		res = SCE_ERROR_ERRNO_EINVAL;
	}	
	
	pspSdkSetK1(k1);
	return res;
}

int sceUmdDeactivate(const int mode, const char *aliasname)
{
	int k1 = pspSdkSetK1(0);

	//Kprintf("Deactivate.\n");

	int res = sceIoUnassign(aliasname);

	if (res >= 0)
	{	
		drivestat = SCE_UMD_MEDIA_IN | SCE_UMD_READY;
		UmdCallback(drivestat);
	}
	
	pspSdkSetK1(k1);
	return res;
}

int sceUmdGetDiscInfo(SceUmdDiscInfo *disc_info)
{
	int k1 = pspSdkSetK1(0);
	int res = 0;

	//Kprintf("Disc info.\n");

	if (disc_info && disc_info->uiSize == 8)
	{
		disc_info->uiMediaType = SCE_UMD_FMT_GAME;
	}
	else
	{
		res = SCE_ERROR_ERRNO_EINVAL;
	}

	pspSdkSetK1(k1);
	return res;
}

int sceUmdRegisterUMDCallBack(SceUID cbid)
{
	int k1 = pspSdkSetK1(0);
	int res = 0;

	//Kprintf("register umdcallback.\n");

	if (sceKernelGetThreadmanIdType(cbid) == SCE_KERNEL_TMID_Callback)
	{
		umdcallback = cbid;
		UmdCallback(drivestat);
	}
	else
	{
		//Kprintf("Invalid callback.\n");
		res = SCE_ERROR_ERRNO_EINVAL;
	}

	pspSdkSetK1(k1);
	return res;
}

int sceUmdUnRegisterUMDCallBack(SceUID cbid)
{
	int k1 = pspSdkSetK1(0);
	int res = 0;
	uidControlBlock *block;

	//Kprintf("Unregister callback.\n");
	
	if (sceKernelGetUIDcontrolBlock(cbid, &block) != 0 || cbid != umdcallback)
	{
		res = SCE_ERROR_ERRNO_EINVAL;
	}
	else
	{
		umdcallback = -1;
	}

	pspSdkSetK1(k1);
	return 0;
}

int sceUmdCheckMedium()
{
	//Kprintf("check medium.\n");
	return 1;
}

int sceUmdGetErrorStat()
{
	int k1 = pspSdkSetK1(0);
	
	//Kprintf("geterrorstat.\n");
	int res = errorstat;
	
	pspSdkSetK1(k1);
	return res;
}

int sceUmdGetErrorStatus()
{
	//Kprintf("get error status %08X", errorstat);
	return errorstat;
}

void sceUmdSetErrorStatus(int error)
{
	//Kprintf("set error status %08X.\n", error);
	errorstat = error;	
}

int sceUmdGetDriveStat()
{
	int k1 = pspSdkSetK1(0);
	
	int res = drivestat;
	//Kprintf("get drive stat 0x%08X\n", res);
	
	pspSdkSetK1(k1);
	return res;
}

int sceUmdGetDriveStatus()
{
	//Kprintf("get drive status 0x%08X\n", drivestat);
	return drivestat;
}

void sceUmdClearDriveStatus(int clear)
{
	drivestat &= clear;
}

void sceUmdSetDriveStatus(int status)
{
	//Kprintf("set drive status 0x%08X\n", status);	
	
	int intr = sceKernelCpuSuspendIntr();

	if (status & SCE_UMD_MEDIA_OUT)
	{
		drivestat &= ~(SCE_UMD_MEDIA_IN | SCE_UMD_MEDIA_CHG | SCE_UMD_NOT_READY | SCE_UMD_READY | SCE_UMD_READABLE);
	}
	else if (status & (SCE_UMD_MEDIA_IN | SCE_UMD_MEDIA_CHG | SCE_UMD_NOT_READY | SCE_UMD_READY | SCE_UMD_READABLE))
	{
		drivestat &= ~(SCE_UMD_MEDIA_OUT);
	}
	
	if (status & SCE_UMD_NOT_READY)
	{
		drivestat &= ~(SCE_UMD_READY | SCE_UMD_READABLE);
	}
	else if (status & (SCE_UMD_READY | SCE_UMD_READABLE))
	{
		drivestat &= ~(SCE_UMD_NOT_READY);
	}

	drivestat |= status;

	if (drivestat & SCE_UMD_READABLE)
	{
		drivestat |= SCE_UMD_READY;
	}

	if (drivestat & SCE_UMD_READY)
	{
		drivestat |= SCE_UMD_MEDIA_IN;
		sceUmdSetErrorStatus(0);
	}

	sceKernelCpuResumeIntr(intr);
}

static int WaitDriveStat(int stat, u32 timer, int cb)
{
	int k1 = pspSdkSetK1(0);
	int res = 0;
	int (*wait_sema)(SceUID, int, SceUInt *);

	//Kprintf("wait drive stat 0x%08X\n", stat);

	if (stat & (SCE_UMD_MEDIA_OUT | SCE_UMD_MEDIA_IN | SCE_UMD_NOT_READY | SCE_UMD_READY | SCE_UMD_READABLE))
	{
		if (stat & SCE_UMD_READABLE)
			drivestat |= SCE_UMD_READABLE;

		else if (stat & (SCE_UMD_MEDIA_IN | SCE_UMD_READY))
		{
		}

		else if (stat & (SCE_UMD_MEDIA_OUT | SCE_UMD_NOT_READY))
		{
			wait_sema = (cb) ? sceKernelWaitSemaCB : sceKernelWaitSema;
			
			if (timer != 0)
			{
				wait_sema(mediaman_sema, 1, &timer);
			}
			else
			{
				wait_sema(mediaman_sema, 1, NULL);
			}
		}
	}
	else
	{
		res = SCE_ERROR_ERRNO_EINVAL;
	}

	if (cb)
	{
		if (game_group == 2)
			sceKernelCheckCallback();
	}

	pspSdkSetK1(k1);
	return res;
}

int sceUmdWaitDriveStat(int stat)
{
	return WaitDriveStat(stat, 0, 0);
}

int sceUmdWaitDriveStatCB(int stat, int timer)
{
	return WaitDriveStat(stat, timer, 1);
}

int sceUmdWaitDriveStatWithTimer(int stat, int timer)
{
	return WaitDriveStat(stat, timer, 0);
}

int sceUmdCancelWaitDriveStat()
{
	int k1 = pspSdkSetK1(0);

	//Kprintf("Cancel WaitDrive Stat.\n");

	int res = sceKernelCancelSema(mediaman_sema, -1, NULL);

	pspSdkSetK1(k1);
	return res;
}

int sceUmdReplaceProhibit()
{
	return 0;
}

int sceUmdReplacePermit()
{
	return 0;
}

int sceUmd_659587F7(int error)
{
	if (error == 0)
		return 0;

	if (sceKernelGetCompiledSdkVersion() != 0)
		return error;

	switch (error)
	{
		case 0x8001005b:
			error = 0x80010024;
	
		case 0x80010070:
			error = 0x80010062;
		break;

		case 0x80010071:
			error = 0x80010067;
		break;
	
		case 0x80010074:
			error = 0x8001006e;
		break;

		case 0x80010086:
			error = 0x8001b000;
		break;

		case 0x80010087:
			error = 0x8001007b;
		break;

		case 0x8001b006:
			error = 0x8001007c;	
		break;
	}

	return error;
}

int InitMediaMan()
{
	drivestat = SCE_UMD_READABLE | SCE_UMD_READY | SCE_UMD_MEDIA_IN;
	umdcallback = -1;
	errorstat = 0;

	mediaman_sema = sceKernelCreateSema("MediaManSema", 0, 0, 1, NULL);

	if (mediaman_sema < 0)
		return mediaman_sema;	

	//Kprintf("media man inited.\n");

	return 0;
}



