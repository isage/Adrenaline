#include <pspsdk.h>
#include <pspkernel.h>
#include <pspthreadman_kernel.h>
#include <psperror.h>

#include <string.h>
#include <systemctrl_se.h>

#include "malloc.h"
#include "umd9660.h"
#include "isoread.h"
#include "csoread.h"

int game_group = 0;


char *umdfile;
SceUID umdsema;
SceUID umdfd = -1;

int discsize=0x7FFFFFFF;

UmdFD descriptors[MAX_DESCRIPTORS];

//int lastLBA = -1;

//u8 *umdsector = NULL;
u8 *umdpvd = NULL;

int cso = 0, mounted = 0;

#define N_GAME_GROUP1	4
#define N_GAME_GROUP2	1

char *game_group1[N_GAME_GROUP1] =
{
	"ULES-00124", "ULUS-10019", "ULJM-05024", "ULAS-42009" // Coded Arms
};

char *game_group2[N_GAME_GROUP2] =
{
	"TERTURADOR" // NPUG-80086 (flow PSN)
};

int GetDiscSize()
{
	if (cso == 0)
		return IsofileGetDiscSize(umdfd);

	return CisofileGetDiscSize(umdfd);
}

/*typedef struct
{
	int lba;
	int nsectors;
	void *buf;	
	int result;
} ReadSectorsParams;

ReadSectorsParams params;

int Umd9660ReadSectors_()
{
	if (!mounted)
	{
		int i;

		for (i = 0; i < 0x10; i++)
		{
			if (sceIoLseek32(umdfd, 0, PSP_SEEK_CUR) < 0)
				OpenIso();
			else
				break;
		}
	}

	if (!mounted)
	{
		return SCE_ERROR_ERRNO_ENODEV;
	}

	if (!cso)
	{
		return IsofileReadSectors(params.lba, params.nsectors, params.buf);		
	}
	
	return CisofileReadSectors(params.lba, params.nsectors, params.buf);
}

int Umd9660ReadSectors2(int lba, int nsectors, void *buf)
{	
	//Kprintf("umd read.\n");
	
	if (!mounted)
	{
		int i;

		for (i = 0; i < 0x10; i++)
		{
			if (sceIoLseek32(umdfd, 0, PSP_SEEK_CUR) < 0)
				OpenIso();
			else
				break;
		}
	}

	if (!mounted)
	{
		//Kprintf("read not mounted.\n");
		return SCE_ERROR_ERRNO_ENODEV;
	}

	if (!cso)
	{
		return IsofileReadSectors(lba, nsectors, buf);			
	}
	
	return CisofileReadSectors(lba, nsectors, buf);	
}

int Umd9660ReadSectors(int lba, int nsectors, void *buf)
{
	//Kprintf("begin read stacked, lba = 0x%08X\n", lba);
	
	params.lba = lba;
	params.nsectors = nsectors;
	params.buf = buf;
	
	return sceKernelExtendKernelStack(0x2000, (void *)Umd9660ReadSectors_, NULL);
}*/

int g_args[3];

#define LOCK() \
	if (sceKernelWaitSema(umdsema, 1, NULL) < 0) \
		return -1; 

#define UNLOCK() \
	if (sceKernelSignalSema(umdsema, 1) < 0) \
		return -1;


int ReadUmdFile_(int *args)
{
	int offset = args[0];
	void *buf = (void *)args[1];
	int outsize = args[2];
	int res;
	
	if (!cso)
	{
		res = ReadUmdFileRetry(buf, outsize, offset);	
	}
	else
	{
		res = CisoReadFile(buf, outsize, offset);
	}

	return res;
}

int ReadUmdFile(int offset, void *buf, int outsize)
{
	int res;

	LOCK();
	
	g_args[0] = offset;
	g_args[1] = (int)buf;
	g_args[2] = outsize;

	res = sceKernelExtendKernelStack(0x2000, (void *)ReadUmdFile_, g_args);

	UNLOCK();
	
	return res;
}

void WaitMs()
{
	u32 status = 0;

	while (1)
	{
		if (sceIoDevctl("mscmhc0:", 0x02025801, 0, 0, &status, 4) >= 0)
		{		
			if (status == 4)
				break;
		}
		
		sceKernelDelayThread(20000);
	}
}

int OpenIso()
{
	//Kprintf("Wait Ms.\n");
	WaitMs();
	//Kprintf("Wait Ms finished.\n");

	mounted = 0;
	sceIoClose(umdfd);

	//Kprintf(umdfile);
	
	umdfd = sceIoOpen(umdfile, PSP_O_RDONLY | 0x000f0000, 0777);
	if (umdfd >= 0)
	{
		cso = 0;
		if (CisoOpen(umdfd) >= 0)
			cso = 1;		

		discsize = GetDiscSize();
		//lastLBA = -1;
		mounted = 1;
		//Kprintf("Mounted succesfull, size %d\n", discsize);
		return 0;
	}

	//Kprintf("Mounted unsuccesfull.\n");
	return -1;
}

int ReadUmdFileRetry(void *buf, int size, int fpointer)
{
	int i;

	for (i = 0; i < 0x10; i++)
	{
		if (sceIoLseek32(umdfd, fpointer, PSP_SEEK_SET) < 0)
			OpenIso();
		else
			break;
	}

	if (i == 0x10)
		return SCE_ERROR_ERRNO_ENODEV;

	for (i = 0; i < 0x10; i++)
	{
		int read = sceIoRead(umdfd, buf, size);

		if (read < 0)
			OpenIso();
		else
			return read;
	}

	return SCE_ERROR_ERRNO_ENODEV;
}

int umd_init(PspIoDrvArg* arg)
{
	int i;
	//Kprintf("umd_init.\n");
	
	//umdsector = (u8 *)oe_malloc(SECTOR_SIZE);	
	umdpvd = (u8 *)oe_malloc(SECTOR_SIZE);
	
	if (!umdpvd)
	{
		return -1;
	}

	umdsema = sceKernelCreateSema("EcsUmd9660DeviceFile", 0, 1, 1, NULL);

	if (umdsema < 0)
	{
		return umdsema;
	}

	while (!mounted)
	{
		//Kprintf("Attempting to open iso.\n");
		OpenIso();
		sceKernelDelayThread(20000);
	}

	memset(&descriptors, 0, sizeof(descriptors));

	//Kprintf("umd_init.\n");

	g_args[0] = 0x10*SECTOR_SIZE;
	g_args[1] = (int)umdpvd;
	g_args[2] = SECTOR_SIZE;

	ReadUmdFile_(g_args);

	char *gamecode = (char *)umdpvd+0x373;
	
	for (i = 0; i < N_GAME_GROUP1; i++)
	{
		if (memcmp(gamecode, game_group1[i], 10) == 0)
		{
			game_group = 1;
			break;
		}
	}

	if (game_group == 0)
	{
		for (i = 0; i < N_GAME_GROUP2; i++)
		{
			if (memcmp(gamecode, game_group2[i], 10) == 0)
			{
				game_group = 2;
				break;
			}
		}
	}

	return 0;
}

int umd_exit(PspIoDrvArg* arg)
{
	SceUInt timeout = 500000;
	
	sceKernelWaitSema(umdsema, 1, &timeout);
	
	/*if (umdsector)
	{
		oe_free(umdsector);
	}*/

	if (umdpvd)
	{
		oe_free(umdpvd);
		umdpvd = NULL;
	}

	if (umdsema >= 0)
	{
		sceKernelDeleteSema(umdsema);
		umdsema = -1;		
	}	

	return 0;
}

int umd_mount(PspIoDrvFileArg *arg)
{
	return 0;
}

int umd_umount(PspIoDrvFileArg *arg)
{
	return 0;
}

int umd_open(PspIoDrvFileArg *arg, char *file, int flags, SceMode mode)
{
	int i;

	//Kprintf("umd open.\n");
	//sceKernelWaitSema(umdsema, 1, NULL);

	for (i = 0; i < 0x10; i++)
	{
		if (sceIoLseek32(umdfd, 0, PSP_SEEK_SET) < 0)
			OpenIso();
		else
			break;
	}

	if (i == 0x10)
	{
		//sceKernelSignalSema(umdsema, 1);		
		return SCE_ERROR_ERRNO_ENODEV;
	}
	
	LOCK();

	for (i = 0; i < MAX_DESCRIPTORS; i++)
	{
		if (!descriptors[i].busy)
			break;
		
	}

	if (i == MAX_DESCRIPTORS)
	{
		UNLOCK();
		return 0x80010018; // EM_FILE
	}

	arg->arg = (void *)i;
	descriptors[i].busy = 1;
	descriptors[i].discpointer = 0;

	UNLOCK();	
	return 0;
}

int umd_close(PspIoDrvFileArg *arg)
{
	int res = 0;
	int i = (int)arg->arg;

	LOCK();

	if (!descriptors[i].busy)
	{
		res = 0x80010016;
	}
	else
	{
		descriptors[i].busy = 0;
	}

	UNLOCK();

	return res;
}

int umd_read(PspIoDrvFileArg *arg, char *data, int len)
{
	int i = (int)arg->arg;
	
	LOCK();

	int discpointer = descriptors[i].discpointer;

	UNLOCK();

	if (discpointer + len > discsize)
	{
		len = discsize - discpointer;
	}

	int res = ReadUmdFile(discpointer*SECTOR_SIZE, data, len*SECTOR_SIZE); //***

	if (res > 0)
	{
		res = res / SECTOR_SIZE;

		LOCK();
		descriptors[i].discpointer += res;
		UNLOCK();
	}

	//sceKernelWaitSema(umdsema, 1);
	return res;
}

SceOff umd_lseek(PspIoDrvFileArg *arg, SceOff ofs, int whence)
{
	int i = (int)arg->arg;
	
	LOCK();

	if (whence == PSP_SEEK_SET)
	{
		descriptors[i].discpointer = ofs;
	}
	else if (whence == PSP_SEEK_CUR)
	{		
		descriptors[i].discpointer += ofs;
	}
	else if (whence == PSP_SEEK_END)
	{
		descriptors[i].discpointer = discsize-ofs;
	}
	else
	{
		UNLOCK();
		return SCE_ERROR_ERRNO_EINVAL;
	}

	if (descriptors[i].discpointer > discsize)
		descriptors[i].discpointer = discsize;

	//sceKernelWaitSema(umdsema, 1);
	int res = descriptors[i].discpointer;

	UNLOCK();
	return res;
}

typedef struct 
{
	SceOff sk_off;
	SceInt32 sk_reserved;
	SceInt32 sk_whence;
} SceUmdSeekParam;

int umd_ioctl(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen)
{
	u32 *outdata32 = (u32 *)outdata;
	u32 *indata32 = (u32 *)indata;
	int i = (int)arg->arg;

	//sceKernelWaitSema(umdsema, 1, NULL);
	switch (cmd)
	{
		case 0x01d20001: // SCE_UMD_GET_FILEPOINTER
		{
			LOCK();
			
			outdata32[0] = descriptors[i].discpointer;
			
			UNLOCK();
			return 0;
		}

		case 0x01f010db:
		{
			return 0;
		}

		case 0x01f100a6: // SCE_UMD_SEEK_FILE
		{
			if (!indata || inlen < 4)
				return 0x80010016;

			SceUmdSeekParam *seekparam = (SceUmdSeekParam *)indata;

			return (int)umd_lseek(arg, seekparam->sk_off, seekparam->sk_whence);
		}
		 
		case 0x01f30003: // SCE_UMD_UNCACHED_READ
		{
			if (!indata || inlen < 4)
				return 0x80010016;

			if (!outdata || outlen < indata32[0])
				return 0x80010016;

			return umd_read(arg, outdata, indata32[0]);
		}
	}

	Kprintf("Unknown ioctl 0x%08X\n", cmd);
	return 0x80010086;
}

/*int ProcessDevctlRead(u32 *indata, u8 *outdata, int outdatasize)
{
	int dataoffset = indata[6];
	int lba = indata[2];
	int datasize = indata[4];
	int dirsectors;
	u8 *p = outdata;
	int remaining = datasize;

	if (outdatasize < datasize)
		return 0x80010069;

	//Kprintf("devctl read, outsize 0x%08X.\n", outdatasize);

	if (dataoffset != 0)
	{
		if (indata[5] != 0 || indata[7] != 0)
		{
			dataoffset = SECTOR_SIZE - dataoffset;		
		}
	}

	if (datasize <= 0)
	{
		return 0;
	}

	if (dataoffset != 0)
	{
		// Read and write the first incomplete sector 

		//Kprintf("dataoffset 0x%08X.\n", dataoffset);
		int x = (SECTOR_SIZE - dataoffset);

		if (x > datasize)
			x = datasize;
		
		ReadOneSector(lba);
		memcpy(p, umdsector+dataoffset, x);

		//Kprintf("x = 0x%08X.\n", x);

		lba += 1;
		p += x;
		datasize -= x;
		remaining = datasize;
	}

	dirsectors = datasize / SECTOR_SIZE; // n of sectors that can be directly written 
		
	if (dirsectors != 0)
	{
		//Kprintf("dirsectors 0x%08X\n", dirsectors);
		
		Umd9660ReadSectors(lba, dirsectors, p); //
		p += (dirsectors*SECTOR_SIZE);
		lba += dirsectors;
		remaining = datasize - (dirsectors*SECTOR_SIZE);
	}

	if ((datasize % SECTOR_SIZE) != 0)
	{
		// Read one and write the remaining
		ReadOneSector(lba);
		memcpy(p, umdsector, remaining);
		//Kprintf("Remaining data 0x%08X.\n", remaining);
	}	

	return indata[3];	
}*/
int ProcessDevctlRead(void *outdata, int size, u32 *indata)
{
	int datasize = indata[4]; // 0x10
	int lba = indata[2]; // 0x08
	int dataoffset = indata[6]; // 0x18
	
	int offset;
	
	if (size < datasize)
		return 0x80010069;

	if (dataoffset == 0)
	{
		offset = lba*0x800;		
	}

	else if (indata[5] != 0)
	{
		offset = (lba*0x800)-dataoffset+0x800;		
	}

	else if (indata[7] == 0)
	{
		offset = (lba*0x800)+dataoffset;		
	}
	else
	{
		offset = (lba*0x800)-dataoffset+0x800;
	}

	return ReadUmdFile(offset, outdata, datasize);
}

/*void WriteFile(char *file, void *buf, int size)
{
	SceUID fd = sceIoOpen(file, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	sceIoWrite(fd, buf, size);
	sceIoClose(fd);
}*/

int umd_devctl(PspIoDrvFileArg *arg, const char *devname, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen)
{
	u32 *outdata32 = (u32 *)outdata;
	int res;

	//sceKernelWaitSema(umdsema, 1, NULL);

	//Kprintf("umd_devctl: %08X\n", cmd);

	// Devctl seen in np9660, and not in isofs:
	// 01e18024 -> return 0x80010086
	// 01e000d5 -> return 0x80010086
	// 01e18033 -> return 0x80010086
	// 01e180c1 -> return 0x80010086
	// 01e180d0 -> return 0x80010086
	// 01e180d6 -> return 0x80010086
	// 01f000a6 -> return 0x80010086

	switch (cmd)
	{
		case 0x01e28035:
		{
			*outdata32 = (u32)umdpvd;
			//sceKernelWaitSema(umdsema, 1);
			return 0;
		}
	
		case 0x01e280a9:
		{
			*outdata32 = 0x800;
			//sceKernelWaitSema(umdsema, 1);
			return 0;
		}
	
		case 0x01e380c0: case 0x01f200a1: case 0x01f200a2:
		{			
			if (!indata ||!outdata)
			{
				return SCE_ERROR_ERRNO_EINVAL;
			}

			res = ProcessDevctlRead(outdata, outlen, indata);
			//sceKernelWaitSema(umdsema, 1);
			
			return res;
		}		

		case 0x01e18030:
		{
			// region related
			//sceKernelWaitSema(umdsema, 1);
			return 1; 
		}
		
		case 0x01e38012:
		{
			if (outlen < 0)
				outlen += 3;

			memset(outdata32, 0, outlen);
			outdata32[0] = 0xe0000800;
			outdata32[7] = outdata32[9] = discsize;
			outdata32[2] = 0;

			//sceKernelWaitSema(umdsema, 1);
			return 0;
		}
		
		case 0x01e38034:
		{
			if (!indata || !outdata)
			{
				return SCE_ERROR_ERRNO_EINVAL;
			}

			*outdata32 = 0;
			
			//sceKernelWaitSema(umdsema, 1);
			return 0;
		}

		case 0x01f20001: /* get disc type */
		{
			outdata32[1] = 0x10; /* game */
			outdata32[0] = 0xFFFFFFFF; 
			
			//sceKernelWaitSema(umdsema, 1);
			return 0;
		}

		case 0x01f00003:
		{
			//sceKernelWaitSema(umdsema, 1); /* activate driver */
			return 0;
		}

		case 0x01f20002:
		{
			//Kprintf("Warning: get last lba devctl.\n");
			outdata32[0] = discsize;
			//sceKernelWaitSema(umdsema, 1);
			return 0;
		}

		case 0x01f20003:
		{
			*outdata32 = discsize;
			//sceKernelWaitSema(umdsema, 1);
			return 0;
		}

		case 0x01e180d3: case 0x01e080a8:
		{
			//sceKernelWaitSema(umdsema, 1);
			return 0x80010086;
		}

		case 0x01f100a3: case 0x01f100a4: case 0x01f010db:
		{
			//sceKernelWaitSema(umdsema, 1);
			return 0;
		}
	}

	Kprintf("unknown devctl 0x%08X\n", cmd);		
	//WriteFile("ms0:/unknown_devctl.bin", &cmd, 4);
	//WriteFile("ms0:/unknown_devctl_indata.bin", indata, inlen);

	//sceKernelWaitSema(umdsema, 1);
	return 0x80010086;
}

int sceUmd9660_driver_2C6C3F4C()
{
	//Kprintf("9660 1.\n");
	return 0;
}

int sceUmd9660_driver_44EF600C()
{
	//Kprintf("9660 2.\n");
	return 0;
}

int sceUmd9660_driver_C7CD9CE8()
{
	//Kprintf("9660 3.\n");
	return 0;
}

PspIoDrvFuncs umd_funcs = 
{ 
	umd_init,
	umd_exit,
	umd_open,
	umd_close,
	umd_read,
	NULL,
	umd_lseek,
	umd_ioctl,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,/*umd_mount,*/
	NULL,/*umd_umount,*/
	umd_devctl,
	NULL
};

PspIoDrv umd_driver = { "umd", 0x4, 0x800, "UMD9660", &umd_funcs };

int SysMemForKernel_C7E57B9C(const u8 *umdid);

static const u8 dummy_umd_id[16] = 
{
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
	0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
};

int InitUmd9660()
{
	int res;
	
	umdfile = GetUmdFile();
	res = sceIoAddDrv(&umd_driver);

	if (res < 0)
		return res;
	
	SysMemForKernel_C7E57B9C(dummy_umd_id);		

	//Kprintf("umd9660 inited.\n");
	
	return 0;
}

int module_stop(SceSize args, void *argp)
{
	sceIoDelDrv("umd");
	return 0;
}

/* Disasm np9660 
int ProcssDevctlRead(void *outdata, int size, u32 *indata)
{
	int datasize = indata[4]; // 0x10
	int lba = indata[2]; // 0x08
	int dataoffset = indata[6]; // 0x18
	int u14 = indata[5]; // 0x14
	int u1C = indata[7]; // 0x1C
	
	if (size < datasize)
		return 0x80010069;

	if (dataoffset == 0)
	{
		res = Read13(lba*0x800, outdata, datasize);
		goto end;
	}

	if (u14 != 0)
	{
		res = Read13((lba*0x800)-dataoffset+0x800, outdata, datasize);
		goto end;
	}

	if (u1C == 0)
	{
		res = Read13((lba*0x800)+dataoffset, outdata, outdatasize);
		goto end;
	}

	res = Read13((lba*0x800)-dataoffset+0x800
}*/

