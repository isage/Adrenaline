#include <common.h>

#include <adrenaline_log.h>

#include "virtualpbpmgr.h"
#include "isofs_driver/umd9660_driver.h"
#include "isofs_driver/isofs_driver.h"

#include "virtualsfo.h"

#define MAX_FILES 128

VirtualPbp *vpbps;
InternalState *states;
SceUID vpsema;
int g_index;

// TODO: rewrite using virtualsfo?
void GetSFOInfo(char *title, int n, char *discid, int m, char *system_version , int *opnssmp, int *parental, char *sfo)
{
	SFOHeader *header = (SFOHeader *)sfo;
	SFODir *entries = (SFODir *)(sfo+0x14);

	opnssmp[0] = 0;
	*(u32 *)system_version = 0x30302E31;

	for (int i = 0; i < header->nitems; i++)
	{
		char *fields_str = (char *)( sfo+header->fields_table_offs+entries[i].field_offs );
		char *values_str = (char *)( sfo+header->values_table_offs+entries[i].val_offs );

		if (strcmp( fields_str , "TITLE") == 0)
		{
			memset(title, 0, n);
			strncpy(title, values_str , n);
		}
		else if (strcmp( fields_str , "DISC_ID") == 0)
		{
			memset(discid, 0, m);
			strncpy(discid, values_str , m);
		}
		else if (strcmp( fields_str , "HRKGMP_VER") == 0)
		{
			*opnssmp = *(int *)(values_str);
		}
		else if (strcmp( fields_str , "PSP_SYSTEM_VER" ) == 0)
		{
			*(u32 *)system_version = *(u32 *)(values_str);
		}
		else if( strcmp( fields_str ,"PARENTAL_LEVEL") == 0 )
		{
			*parental = *(int *)(values_str);
		}
	}
}

int virtualpbp_init()
{
	vpbps = (VirtualPbp *)oe_malloc(MAX_FILES*sizeof(VirtualPbp));
	memset(vpbps, 0, MAX_FILES*sizeof(VirtualPbp));

	if (!vpbps)
	{
		return -1;
	}

	states = (InternalState *)oe_malloc(MAX_FILES*sizeof(InternalState));
	memset(states, 0, MAX_FILES*sizeof(InternalState));
	if (!states)
	{
		return -1;
	}

	vpsema = sceKernelCreateSema("VirtualPBPMgr", 0, 1, 1, NULL);
	if (vpsema < 0)
	{
		return vpsema;
	}

	memset(vpbps, 0, MAX_FILES*sizeof(VirtualPbp));
	g_index = 0;
	virtual_sfo_init();
	return 0;
}

int virtualpbp_exit()
{
	sceKernelWaitSema(vpsema, 1, NULL);
	oe_free(vpbps);
	oe_free(states);
	sceKernelDeleteSema(vpsema);

	return 0;
}

int virtualpbp_reset()
{
	sceKernelWaitSema(vpsema, 1, NULL);

	memset(vpbps, 0, MAX_FILES*sizeof(VirtualPbp));
	memset(states, 0, MAX_FILES*sizeof(InternalState));
	g_index = 0;

	sceKernelSignalSema(vpsema, 1);
	return 0;
}

void getlba_andsize(PspIoDrvFileArg *arg, const char *file, int *lba, int *size)
{
	SceIoStat stat;

	memset(&stat, 0, sizeof(SceIoStat));
	if (isofs_getstat(file, &stat) >= 0)
	{
		*lba = stat.st_private[0];
		*size = stat.st_size;
	}
}

int virtualpbp_add(char *isofile, ScePspDateTime *mtime, VirtualPbp *res)
{
	PspIoDrvFileArg arg;
	int offset;

	sceKernelWaitSema(vpsema, 1, NULL);

	if (g_index >= MAX_FILES)
	{
		sceKernelSignalSema(vpsema, 1);
		return -1;
	}

	memset(vpbps[g_index].isofile, 0, sizeof(vpbps[g_index].isofile));
	strncpy(vpbps[g_index].isofile, isofile, sizeof(vpbps[g_index].isofile));
	VshCtrlSetUmdFile(isofile);

	memset(&arg, 0, sizeof(arg));
	if (isofs_init() < 0)
	{
		isofs_exit();
		sceKernelSignalSema(vpsema, 1);
		return -1;
	}

	SceUID fd = isofs_open("/PSP_GAME/PARAM.SFO", PSP_O_RDONLY, 0);
	if (fd >= 0)
	{
		char *buf = (char *)oe_malloc(1024);

		isofs_read(fd, buf, 1024);
		isofs_close(fd);

		GetSFOInfo(
			vpbps[g_index].sfotitle, sizeof(vpbps[g_index].sfotitle),
			vpbps[g_index].discid, sizeof(vpbps[g_index].discid),
			(char*)&(vpbps[g_index].system_ver),
			&(vpbps[g_index].opnssmp_type),
			&(vpbps[g_index].parental_level),
			buf
		);

		oe_free(buf);
	}
	else
	{
		isofs_exit();
		sceKernelSignalSema(vpsema, 1);
		return -1;
	}

	getlba_andsize(&arg, "/PSP_GAME/ICON0.PNG", &vpbps[g_index].i0png_lba, &vpbps[g_index].i0png_size);
	getlba_andsize(&arg, "/PSP_GAME/ICON1.PMF", &vpbps[g_index].i1pmf_lba, &vpbps[g_index].i1pmf_size);
	getlba_andsize(&arg, "/PSP_GAME/PIC0.PNG", &vpbps[g_index].p0png_lba, &vpbps[g_index].p0png_size);
	getlba_andsize(&arg, "/PSP_GAME/PIC1.PNG", &vpbps[g_index].p1png_lba, &vpbps[g_index].p1png_size);
	getlba_andsize(&arg, "/PSP_GAME/SND0.AT3", &vpbps[g_index].s0at3_lba, &vpbps[g_index].s0at3_size);

	isofs_exit();

	vpbps[g_index].header[0] = 0x50425000;
	vpbps[g_index].header[1] = 0x10000;

	offset = 0x28;

	/*vpbps[g_index].header[2] = offset; // SFO
	offset += vpbps[g_index].psfo_size;*/

	vpbps[g_index].header[2] = offset;
	offset += virtual_sfo_size();

	vpbps[g_index].header[3] = offset; // ICON0.PNG
	offset += vpbps[g_index].i0png_size;

	vpbps[g_index].header[4] = offset; // ICON1.PMF
	offset += vpbps[g_index].i1pmf_size;

	vpbps[g_index].header[5] = offset; // PIC0.PNG
	offset += vpbps[g_index].p0png_size;

	vpbps[g_index].header[6] = offset; // PIC1.PNG
	offset += vpbps[g_index].p1png_size;

	vpbps[g_index].header[7] = offset; // SND0.AT3
	offset += vpbps[g_index].s0at3_size;

	vpbps[g_index].header[8] = offset; // DATA.PSP
	vpbps[g_index].header[9] = offset; // DATA.PSAR

	vpbps[g_index].filesize = offset;

	memcpy(&vpbps[g_index].mtime, mtime, sizeof(ScePspDateTime));

	if (res)
	{
		memcpy(res, &vpbps[g_index], sizeof(VirtualPbp));
	}

	g_index++;

	logmsg("[INFO]: %s ISO file was added to VirtualPBP list\n", isofile);
	sceKernelSignalSema(vpsema, 1);
	return 0;
}

int virtualpbp_fastadd(VirtualPbp *pbp)
{
	sceKernelWaitSema(vpsema, 1, NULL);

	if (g_index >= MAX_FILES)
	{
		sceKernelSignalSema(vpsema, 1);
		return -1;
	}

	memcpy(&vpbps[g_index], pbp, sizeof(VirtualPbp));

	g_index++;

	logmsg2("[INFO]: %s ISO file was added to VirtualPBP list\n", vpbps[g_index].isofile);
	sceKernelSignalSema(vpsema, 1);
	return 0;
}

int virtualpbp_open(int i)
{
	sceKernelWaitSema(vpsema, 1, NULL);

	if (i < 0 || i >= g_index || states[i].deleted)
	{
		sceKernelSignalSema(vpsema, 1);
		return -1;
	}

	vpbps[i].filepointer = 0;

	sceKernelSignalSema(vpsema, 1);
	return 0x7000+i;
}

int virtualpbp_close(SceUID fd)
{
	sceKernelWaitSema(vpsema, 1, NULL);

	fd = fd-0x7000;

	if (fd < 0 || fd >= g_index)
	{
		sceKernelSignalSema(vpsema, 1);
		return -1;
	}

	sceKernelSignalSema(vpsema, 1);
	return 0;
}

int virtualpbp_read(SceUID fd, void *data, SceSize size)
{
	sceKernelWaitSema(vpsema, 1, NULL);

	fd = fd-0x7000;

	if (fd < 0 || fd >= g_index)
	{
		sceKernelSignalSema(vpsema, 1);
		return -1;
	}

	VshCtrlSetUmdFile(vpbps[fd].isofile);
	isofs_fastinit();

	PspIoDrvFileArg arg;
	int remaining;
	int n, read, base;
	void *p;
	char filename[32];

	memset(&arg, 0, sizeof(PspIoDrvFileArg));
	remaining = size;
	read = 0;
	p = data;

	while (remaining > 0)
	{
		if (vpbps[fd].filepointer >= 0 && vpbps[fd].filepointer < 0x28)
		{
			u8 *header = (u8 *)&vpbps[fd].header;

			n = 0x28-vpbps[fd].filepointer;
			if (remaining < n)
			{
				n = remaining;
			}

			memcpy(p, header+vpbps[fd].filepointer, n);
			remaining -= n;
			p += n;
			vpbps[fd].filepointer += n;
			read += n;
		}

		/*if ((vpbps[fd].filepointer >= vpbps[fd].header[2]) &&
			(vpbps[fd].filepointer < vpbps[fd].header[3]))
		{
			base = vpbps[fd].filepointer - vpbps[fd].header[2];

			n = vpbps[fd].psfo_size-(base);
			if (remaining < n)
			{
				n = remaining;
			}

			sprintf(filename, "/sce_lbn0x%x_size0x%x", vpbps[fd].psfo_lba, vpbps[fd].psfo_size);

			SceUID fd = isofs_open(filename, PSP_O_RDONLY, 0);
			isofs_lseek(fd, base, PSP_SEEK_SET);
			isofs_read(fd, p, n);
			isofs_close(fd);

			remaining -= n;
			p += n;
			vpbps[fd].filepointer += n;
			read += n;
		}*/

		if ((vpbps[fd].filepointer >= vpbps[fd].header[2]) &&
			(vpbps[fd].filepointer < vpbps[fd].header[3]))
		{
			sfo_set_string_param("TITLE", vpbps[fd].sfotitle);

			if (vpbps[fd].discid[0])
			{
				sfo_set_string_param("DISC_ID", vpbps[fd].discid);
			}

			char *ver_str = vpbps[fd].system_ver;
			sfo_set_string_param("PSP_SYSTEM_VER", ver_str);

			int parental = vpbps[fd].parental_level;
			if( parental == 0 )
			{
				parental = 1;
			}

			sfo_set_int_param("PARENTAL_LEVEL", parental);

			// TODO: Set APP_VER from PBOOT

			base = vpbps[fd].filepointer - vpbps[fd].header[2];

			n = virtual_sfo_size()-(base);
			if (remaining < n)
			{
				n = remaining;
			}

			memcpy(p, virtual_sfo_get()+base, n);
			remaining -= n;
			p += n;
			vpbps[fd].filepointer += n;
			read += n;
		}

		if ((vpbps[fd].filepointer >= vpbps[fd].header[3]) &&
			(vpbps[fd].filepointer < vpbps[fd].header[4]))
		{
			base = vpbps[fd].filepointer - vpbps[fd].header[3];

			n = vpbps[fd].i0png_size-(base);
			if (remaining < n)
			{
				n = remaining;
			}

			sprintf(filename, "/sce_lbn0x%x_size0x%x", vpbps[fd].i0png_lba, vpbps[fd].i0png_size);

			SceUID fp = isofs_open(filename, PSP_O_RDONLY, 0);
			isofs_lseek(fp, base, PSP_SEEK_SET);
			isofs_read(fp, p, n);
			isofs_close(fp);

			remaining -= n;
			p += n;
			vpbps[fd].filepointer += n;
			read += n;
		}

		if ((vpbps[fd].filepointer >= vpbps[fd].header[4]) &&
			(vpbps[fd].filepointer < vpbps[fd].header[5]))
		{
			base = vpbps[fd].filepointer - vpbps[fd].header[4];

			n = vpbps[fd].i1pmf_size-(base);
			if (remaining < n)
			{
				n = remaining;
			}

			sprintf(filename, "/sce_lbn0x%x_size0x%x", vpbps[fd].i1pmf_lba, vpbps[fd].i1pmf_size);

			SceUID fp = isofs_open(filename, PSP_O_RDONLY, 0);
			isofs_lseek(fp, base, PSP_SEEK_SET);
			isofs_read(fp, p, n);
			isofs_close(fp);

			remaining -= n;
			p += n;
			vpbps[fd].filepointer += n;
			read += n;
		}

		if ((vpbps[fd].filepointer >= vpbps[fd].header[5]) &&
			(vpbps[fd].filepointer < vpbps[fd].header[6]))
		{
			base = vpbps[fd].filepointer - vpbps[fd].header[5];

			n = vpbps[fd].p0png_size-(base);
			if (remaining < n)
			{
				n = remaining;
			}

			sprintf(filename, "/sce_lbn0x%x_size0x%x", vpbps[fd].p0png_lba, vpbps[fd].p0png_size);

			SceUID fp = isofs_open(filename, PSP_O_RDONLY, 0);
			isofs_lseek(fp, base, PSP_SEEK_SET);
			isofs_read(fp, p, n);
			isofs_close(fp);

			remaining -= n;
			p += n;
			vpbps[fd].filepointer += n;
			read += n;
		}

		if ((vpbps[fd].filepointer >= vpbps[fd].header[6]) &&
			(vpbps[fd].filepointer < vpbps[fd].header[7]))
		{
			base = vpbps[fd].filepointer - vpbps[fd].header[6];

			n = vpbps[fd].p1png_size-(base);
			if (remaining < n)
			{
				n = remaining;
			}

			sprintf(filename, "/sce_lbn0x%x_size0x%x", vpbps[fd].p1png_lba, vpbps[fd].p1png_size);

			SceUID fp = isofs_open(filename, PSP_O_RDONLY, 0);
			isofs_lseek(fp, base, PSP_SEEK_SET);
			isofs_read(fp, p, n);
			isofs_close(fp);

			remaining -= n;
			p += n;
			vpbps[fd].filepointer += n;
			read += n;
		}

		if ((vpbps[fd].filepointer >= vpbps[fd].header[7]) &&
			(vpbps[fd].filepointer < vpbps[fd].header[8]))
		{
			base = vpbps[fd].filepointer - vpbps[fd].header[7];

			n = vpbps[fd].s0at3_size-(base);
			if (remaining < n)
			{
				n = remaining;
			}

			sprintf(filename, "/sce_lbn0x%x_size0x%x", vpbps[fd].s0at3_lba, vpbps[fd].s0at3_size);

			SceUID fp = isofs_open(filename, PSP_O_RDONLY, 0);
			isofs_lseek(fp, base, PSP_SEEK_SET);
			isofs_read(fp, p, n);
			isofs_close(fp);

			remaining -= n;
			p += n;
			vpbps[fd].filepointer += n;
			read += n;
		}

		if (vpbps[fd].filepointer >= vpbps[fd].filesize)
		{
			break;
		}
	}

	isofs_exit();

	sceKernelSignalSema(vpsema, 1);
	return read;
}

int virtualpbp_lseek(SceUID fd, SceOff offset, int whence)
{
	sceKernelWaitSema(vpsema, 1, NULL);

	fd = fd - 0x7000;

	if (fd < 0 || fd >= g_index)
	{
		sceKernelSignalSema(vpsema, 1);
		return -1;
	}

	if (whence == PSP_SEEK_SET)
	{
		vpbps[fd].filepointer = (int)offset;
	}
	else if (whence == PSP_SEEK_CUR)
	{
		vpbps[fd].filepointer += (int)offset;
	}
	else if (vpbps[fd].filepointer == PSP_SEEK_END)
	{
		vpbps[fd].filepointer = vpbps[fd].filesize - (int)offset;
	}
	else
	{
		sceKernelSignalSema(vpsema, 1);
		return -1;
	}

	sceKernelSignalSema(vpsema, 1);
	return vpbps[fd].filepointer;
}

int virtualpbp_getstat(int i, SceIoStat *stat)
{
	int res;

	sceKernelWaitSema(vpsema, 1, NULL);

	if (i < 0 || i >= g_index || states[i].deleted)
	{
		sceKernelSignalSema(vpsema, 1);
		return -1;
	}

	res =  sceIoGetstat(vpbps[i].isofile, stat);
	stat->st_size = vpbps[i].filesize;

	memcpy(&stat->sce_st_mtime, &stat->sce_st_ctime, sizeof(ScePspDateTime));

	sceKernelSignalSema(vpsema, 1);
	return res;
}

int virtualpbp_chstat(int i, SceIoStat *stat, int bits)
{
	sceKernelWaitSema(vpsema, 1, NULL);

	if (i < 0 || i >= g_index)
	{
		sceKernelSignalSema(vpsema, 1);
		return -1;
	}

	sceIoChstat(vpbps[i].isofile, stat, bits);

	sceKernelSignalSema(vpsema, 1);
	return 0;
}

int virtualpbp_remove(int i)
{
	sceKernelWaitSema(vpsema, 1, NULL);

	if (i < 0 || i >= g_index)
	{
		sceKernelSignalSema(vpsema, 1);
		return -1;
	}

	VshCtrlSetUmdFile("");
	int res = sceIoRemove(vpbps[i].isofile);
	if (res >= 0)
	{
		states[i].deleted = 1;
	}

	sceKernelSignalSema(vpsema, 1);
	return 0;
}

int virtualpbp_rmdir(int i)
{
	sceKernelWaitSema(vpsema, 1, NULL);

	if (i < 0 || i >= g_index || states[i].psdirdeleted)
	{
		sceKernelSignalSema(vpsema, 1);
		return -1;
	}

	states[i].psdirdeleted = 1;

	sceKernelSignalSema(vpsema, 1);
	return 0;
}

int virtualpbp_dread(SceUID fd, SceIoDirent *dir)
{
	SceFatMsDirentPrivate *private;

	sceKernelWaitSema(vpsema, 1, NULL);

	fd = fd - 0x7000;

	if (fd < 0 || fd >= g_index)
	{
		sceKernelSignalSema(vpsema, 1);
		return -1;
	}

	if (states[fd].dread == 2)
	{
		states[fd].dread = 0;
		sceKernelSignalSema(vpsema, 1);
		return 0;
	}

	sceIoGetstat(vpbps[fd].isofile, &dir->d_stat);
	memcpy(&dir->d_stat.sce_st_mtime, &dir->d_stat.sce_st_ctime, sizeof(ScePspDateTime));
	private = (SceFatMsDirentPrivate *)dir->d_private;

	if (states[fd].dread == 1)
	{
		dir->d_stat.st_size = vpbps[fd].filesize;
		strcpy(dir->d_name, "EBOOT.PBP");
		if (private)
		{
			strcpy(private->FileName, "EBOOT.PBP");
			strcpy(private->LongName, "EBOOT.PBP");
		}
	}
	else
	{
		dir->d_stat.st_size -= vpbps[fd].filesize;
		strcpy(dir->d_name, "IMAGE.ISO");
		if (private)
		{
			strcpy(private->FileName, "IMAGE.ISO");
			strcpy(private->LongName, "IMAGE.ISO");
		}
	}

	states[fd].dread++;
	sceKernelSignalSema(vpsema, 1);
	return 1;
}

char *virtualpbp_getfilename(int i)
{
	sceKernelWaitSema(vpsema, 1, NULL);

	if (i < 0 || i >= g_index)
	{
		sceKernelSignalSema(vpsema, 1);
		return NULL;
	}

	sceKernelSignalSema(vpsema, 1);
	return vpbps[i].isofile;
}

int virtualpbp_get_isotype(int i)
{
	sceKernelWaitSema(vpsema, 1, NULL);

	if (i < 0 || i >= g_index)
	{
		sceKernelSignalSema(vpsema, 1);
		return 0;
	}

	sceKernelSignalSema(vpsema, 1);
	return vpbps[i].opnssmp_type;
}

char *virtualpbp_getdiscid(int i)
{
	sceKernelWaitSema(vpsema, 1, NULL);

	if (i < 0 || i >= g_index)
	{
		sceKernelSignalSema(vpsema, 1);
		return NULL;
	}

	sceKernelSignalSema(vpsema, 1);
	return vpbps[i].discid;
}
