/*
	Adrenaline
	Copyright (C) 2016-2018, TheFloW
	Copyright (C) 2024-2025, isage
	Copyright (C) 2025, GrayJack

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <string.h>

#include <pspthreadman.h>
#include <pspiofilemgr.h>
#include <pspiofilemgr_kernel.h>

#include <systemctrl_se.h>
#include <pspextratypes.h>

#include <adrenaline_log.h>

#include "virtualpbpmgr.h"
#include "isofs_driver/umd9660_driver.h"
#include "isofs_driver/isofs_driver.h"

#include "virtualsfo.h"

#define MAX_FILES 128

static VirtualPbp *g_vpbps;
static InternalState *g_states;
static SceUID g_vpsema;
static int g_index;

// TODO: rewrite using virtualsfo?
void GetSFOInfo(char *title, int n, char *discid, int m, char *system_version , int *opnssmp, int *parental, char *sfo) {
	SFOHeader *header = (SFOHeader *)sfo;
	SFODir *entries = (SFODir *)(sfo+0x14);

	opnssmp[0] = 0;
	*(u32 *)system_version = 0x30302E31;

	for (int i = 0; i < header->nitems; i++) {
		char *fields_str = (char *)( sfo+header->fields_table_offs+entries[i].field_offs );
		char *values_str = (char *)( sfo+header->values_table_offs+entries[i].val_offs );

		if (strcmp( fields_str , "TITLE") == 0) {
			memset(title, 0, n);
			strncpy(title, values_str , n);
		} else if (strcmp( fields_str , "DISC_ID") == 0) {
			memset(discid, 0, m);
			strncpy(discid, values_str , m);
		} else if (strcmp( fields_str , "HRKGMP_VER") == 0) {
			*opnssmp = *(int *)(values_str);
		} else if (strcmp( fields_str , "PSP_SYSTEM_VER" ) == 0) {
			*(u32 *)system_version = *(u32 *)(values_str);
		} else if( strcmp( fields_str ,"PARENTAL_LEVEL") == 0 ) {
			*parental = *(int *)(values_str);
		}
	}
}

int virtualpbp_init() {
	g_vpbps = (VirtualPbp *)oe_malloc(MAX_FILES*sizeof(VirtualPbp));
	memset(g_vpbps, 0, MAX_FILES*sizeof(VirtualPbp));

	if (!g_vpbps) {
		return -1;
	}

	g_states = (InternalState *)oe_malloc(MAX_FILES*sizeof(InternalState));
	memset(g_states, 0, MAX_FILES*sizeof(InternalState));
	if (!g_states) {
		return -1;
	}

	g_vpsema = sceKernelCreateSema("VirtualPBPMgr", 0, 1, 1, NULL);
	if (g_vpsema < 0) {
		return g_vpsema;
	}

	memset(g_vpbps, 0, MAX_FILES*sizeof(VirtualPbp));
	g_index = 0;
	virtual_sfo_init();
	return 0;
}

int virtualpbp_exit() {
	sceKernelWaitSema(g_vpsema, 1, NULL);
	oe_free(g_vpbps);
	oe_free(g_states);
	sceKernelDeleteSema(g_vpsema);

	return 0;
}

int virtualpbp_reset() {
	sceKernelWaitSema(g_vpsema, 1, NULL);

	memset(g_vpbps, 0, MAX_FILES*sizeof(VirtualPbp));
	memset(g_states, 0, MAX_FILES*sizeof(InternalState));
	g_index = 0;

	sceKernelSignalSema(g_vpsema, 1);
	return 0;
}

void getlba_andsize(PspIoDrvFileArg *arg, const char *file, int *lba, int *size) {
	SceIoStat stat;

	memset(&stat, 0, sizeof(SceIoStat));
	if (isofs_getstat(file, &stat) >= 0) {
		*lba = stat.st_private[0];
		*size = stat.st_size;
	}
}

int virtualpbp_add(char *isofile, ScePspDateTime *mtime, VirtualPbp *res) {
	PspIoDrvFileArg arg;
	int offset;

	sceKernelWaitSema(g_vpsema, 1, NULL);

	if (g_index >= MAX_FILES) {
		sceKernelSignalSema(g_vpsema, 1);
		return -1;
	}

	memset(g_vpbps[g_index].isofile, 0, sizeof(g_vpbps[g_index].isofile));
	strncpy(g_vpbps[g_index].isofile, isofile, sizeof(g_vpbps[g_index].isofile));
	VshCtrlSetUmdFile(isofile);

	memset(&arg, 0, sizeof(arg));
	if (isofs_init() < 0) {
		isofs_exit();
		sceKernelSignalSema(g_vpsema, 1);
		return -1;
	}

	SceUID fd = isofs_open("/PSP_GAME/PARAM.SFO", PSP_O_RDONLY, 0);
	if (fd >= 0) {
		char *buf = (char *)oe_malloc(1024);

		isofs_read(fd, buf, 1024);
		isofs_close(fd);

		GetSFOInfo(
			g_vpbps[g_index].sfotitle, sizeof(g_vpbps[g_index].sfotitle),
			g_vpbps[g_index].discid, sizeof(g_vpbps[g_index].discid),
			(char*)&(g_vpbps[g_index].system_ver),
			&(g_vpbps[g_index].opnssmp_type),
			&(g_vpbps[g_index].parental_level),
			buf
		);

		oe_free(buf);
	} else {
		isofs_exit();
		sceKernelSignalSema(g_vpsema, 1);
		return -1;
	}

	getlba_andsize(&arg, "/PSP_GAME/ICON0.PNG", &g_vpbps[g_index].i0png_lba, &g_vpbps[g_index].i0png_size);
	getlba_andsize(&arg, "/PSP_GAME/ICON1.PMF", &g_vpbps[g_index].i1pmf_lba, &g_vpbps[g_index].i1pmf_size);
	getlba_andsize(&arg, "/PSP_GAME/PIC0.PNG", &g_vpbps[g_index].p0png_lba, &g_vpbps[g_index].p0png_size);
	getlba_andsize(&arg, "/PSP_GAME/PIC1.PNG", &g_vpbps[g_index].p1png_lba, &g_vpbps[g_index].p1png_size);
	getlba_andsize(&arg, "/PSP_GAME/SND0.AT3", &g_vpbps[g_index].s0at3_lba, &g_vpbps[g_index].s0at3_size);

	isofs_exit();

	g_vpbps[g_index].header[0] = 0x50425000;
	g_vpbps[g_index].header[1] = 0x10000;

	offset = 0x28;

	/*vpbps[g_index].header[2] = offset; // SFO
	offset += vpbps[g_index].psfo_size;*/

	g_vpbps[g_index].header[2] = offset;
	offset += virtual_sfo_size();

	g_vpbps[g_index].header[3] = offset; // ICON0.PNG
	offset += g_vpbps[g_index].i0png_size;

	g_vpbps[g_index].header[4] = offset; // ICON1.PMF
	offset += g_vpbps[g_index].i1pmf_size;

	g_vpbps[g_index].header[5] = offset; // PIC0.PNG
	offset += g_vpbps[g_index].p0png_size;

	g_vpbps[g_index].header[6] = offset; // PIC1.PNG
	offset += g_vpbps[g_index].p1png_size;

	g_vpbps[g_index].header[7] = offset; // SND0.AT3
	offset += g_vpbps[g_index].s0at3_size;

	g_vpbps[g_index].header[8] = offset; // DATA.PSP
	g_vpbps[g_index].header[9] = offset; // DATA.PSAR

	g_vpbps[g_index].filesize = offset;

	memcpy(&g_vpbps[g_index].mtime, mtime, sizeof(ScePspDateTime));

	if (res) {
		memcpy(res, &g_vpbps[g_index], sizeof(VirtualPbp));
	}
	logmsg("[INFO]: ISO file was added to VirtualPBP list: `%s`\n", g_vpbps[g_index].isofile);

	g_index++;

	sceKernelSignalSema(g_vpsema, 1);
	return 0;
}

int virtualpbp_fastadd(VirtualPbp *pbp) {
	sceKernelWaitSema(g_vpsema, 1, NULL);

	if (g_index >= MAX_FILES) {
		sceKernelSignalSema(g_vpsema, 1);
		return -1;
	}

	memcpy(&g_vpbps[g_index], pbp, sizeof(VirtualPbp));
	logmsg2("[INFO]: ISO file was added to VirtualPBP list: `%s`\n", g_vpbps[g_index].isofile);

	g_index++;

	sceKernelSignalSema(g_vpsema, 1);
	return 0;
}

int virtualpbp_open(int i) {
	sceKernelWaitSema(g_vpsema, 1, NULL);

	if (i < 0 || i >= g_index || g_states[i].deleted) {
		sceKernelSignalSema(g_vpsema, 1);
		return -1;
	}

	g_vpbps[i].filepointer = 0;

	sceKernelSignalSema(g_vpsema, 1);
	return 0x7000+i;
}

int virtualpbp_close(SceUID fd) {
	sceKernelWaitSema(g_vpsema, 1, NULL);

	fd = fd-0x7000;

	if (fd < 0 || fd >= g_index) {
		sceKernelSignalSema(g_vpsema, 1);
		return -1;
	}

	sceKernelSignalSema(g_vpsema, 1);
	return 0;
}

int virtualpbp_read(SceUID fd, void *data, SceSize size) {
	sceKernelWaitSema(g_vpsema, 1, NULL);

	fd = fd-0x7000;

	if (fd < 0 || fd >= g_index) {
		sceKernelSignalSema(g_vpsema, 1);
		return -1;
	}

	VshCtrlSetUmdFile(g_vpbps[fd].isofile);
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

	while (remaining > 0) {
		if (g_vpbps[fd].filepointer >= 0 && g_vpbps[fd].filepointer < 0x28) {
			u8 *header = (u8 *)&g_vpbps[fd].header;

			n = 0x28-g_vpbps[fd].filepointer;
			if (remaining < n) {
				n = remaining;
			}

			memcpy(p, header+g_vpbps[fd].filepointer, n);
			remaining -= n;
			p += n;
			g_vpbps[fd].filepointer += n;
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

		if ((g_vpbps[fd].filepointer >= g_vpbps[fd].header[2]) && (g_vpbps[fd].filepointer < g_vpbps[fd].header[3])) {
			sfo_set_string_param("TITLE", g_vpbps[fd].sfotitle);

			if (g_vpbps[fd].discid[0]) {
				sfo_set_string_param("DISC_ID", g_vpbps[fd].discid);
			}

			char *ver_str = g_vpbps[fd].system_ver;
			sfo_set_string_param("PSP_SYSTEM_VER", ver_str);

			int parental = g_vpbps[fd].parental_level;
			if (parental == 0) {
				parental = 1;
			}

			sfo_set_int_param("PARENTAL_LEVEL", parental);

			// TODO: Set APP_VER from PBOOT

			base = g_vpbps[fd].filepointer - g_vpbps[fd].header[2];

			n = virtual_sfo_size()-(base);
			if (remaining < n) {
				n = remaining;
			}

			memcpy(p, virtual_sfo_get()+base, n);
			remaining -= n;
			p += n;
			g_vpbps[fd].filepointer += n;
			read += n;
		}

		if ((g_vpbps[fd].filepointer >= g_vpbps[fd].header[3]) && (g_vpbps[fd].filepointer < g_vpbps[fd].header[4])) {
			base = g_vpbps[fd].filepointer - g_vpbps[fd].header[3];

			n = g_vpbps[fd].i0png_size-(base);
			if (remaining < n) {
				n = remaining;
			}

			sprintf(filename, "/sce_lbn0x%x_size0x%x", g_vpbps[fd].i0png_lba, g_vpbps[fd].i0png_size);

			SceUID fp = isofs_open(filename, PSP_O_RDONLY, 0);
			isofs_lseek(fp, base, PSP_SEEK_SET);
			isofs_read(fp, p, n);
			isofs_close(fp);

			remaining -= n;
			p += n;
			g_vpbps[fd].filepointer += n;
			read += n;
		}

		if ((g_vpbps[fd].filepointer >= g_vpbps[fd].header[4]) && (g_vpbps[fd].filepointer < g_vpbps[fd].header[5])) {
			base = g_vpbps[fd].filepointer - g_vpbps[fd].header[4];

			n = g_vpbps[fd].i1pmf_size-(base);
			if (remaining < n) {
				n = remaining;
			}

			sprintf(filename, "/sce_lbn0x%x_size0x%x", g_vpbps[fd].i1pmf_lba, g_vpbps[fd].i1pmf_size);

			SceUID fp = isofs_open(filename, PSP_O_RDONLY, 0);
			isofs_lseek(fp, base, PSP_SEEK_SET);
			isofs_read(fp, p, n);
			isofs_close(fp);

			remaining -= n;
			p += n;
			g_vpbps[fd].filepointer += n;
			read += n;
		}

		if ((g_vpbps[fd].filepointer >= g_vpbps[fd].header[5]) && (g_vpbps[fd].filepointer < g_vpbps[fd].header[6])) {
			base = g_vpbps[fd].filepointer - g_vpbps[fd].header[5];

			n = g_vpbps[fd].p0png_size-(base);
			if (remaining < n) {
				n = remaining;
			}

			sprintf(filename, "/sce_lbn0x%x_size0x%x", g_vpbps[fd].p0png_lba, g_vpbps[fd].p0png_size);

			SceUID fp = isofs_open(filename, PSP_O_RDONLY, 0);
			isofs_lseek(fp, base, PSP_SEEK_SET);
			isofs_read(fp, p, n);
			isofs_close(fp);

			remaining -= n;
			p += n;
			g_vpbps[fd].filepointer += n;
			read += n;
		}

		if ((g_vpbps[fd].filepointer >= g_vpbps[fd].header[6]) && (g_vpbps[fd].filepointer < g_vpbps[fd].header[7])) {
			base = g_vpbps[fd].filepointer - g_vpbps[fd].header[6];

			n = g_vpbps[fd].p1png_size-(base);
			if (remaining < n) {
				n = remaining;
			}

			sprintf(filename, "/sce_lbn0x%x_size0x%x", g_vpbps[fd].p1png_lba, g_vpbps[fd].p1png_size);

			SceUID fp = isofs_open(filename, PSP_O_RDONLY, 0);
			isofs_lseek(fp, base, PSP_SEEK_SET);
			isofs_read(fp, p, n);
			isofs_close(fp);

			remaining -= n;
			p += n;
			g_vpbps[fd].filepointer += n;
			read += n;
		}

		if ((g_vpbps[fd].filepointer >= g_vpbps[fd].header[7]) && (g_vpbps[fd].filepointer < g_vpbps[fd].header[8])) {
			base = g_vpbps[fd].filepointer - g_vpbps[fd].header[7];

			n = g_vpbps[fd].s0at3_size-(base);
			if (remaining < n) {
				n = remaining;
			}

			sprintf(filename, "/sce_lbn0x%x_size0x%x", g_vpbps[fd].s0at3_lba, g_vpbps[fd].s0at3_size);

			SceUID fp = isofs_open(filename, PSP_O_RDONLY, 0);
			isofs_lseek(fp, base, PSP_SEEK_SET);
			isofs_read(fp, p, n);
			isofs_close(fp);

			remaining -= n;
			p += n;
			g_vpbps[fd].filepointer += n;
			read += n;
		}

		if (g_vpbps[fd].filepointer >= g_vpbps[fd].filesize) {
			break;
		}
	}

	isofs_exit();

	sceKernelSignalSema(g_vpsema, 1);
	return read;
}

int virtualpbp_lseek(SceUID fd, SceOff offset, int whence) {
	sceKernelWaitSema(g_vpsema, 1, NULL);

	fd = fd - 0x7000;

	if (fd < 0 || fd >= g_index) {
		sceKernelSignalSema(g_vpsema, 1);
		return -1;
	}

	if (whence == PSP_SEEK_SET) {
		g_vpbps[fd].filepointer = (int)offset;
	} else if (whence == PSP_SEEK_CUR) {
		g_vpbps[fd].filepointer += (int)offset;
	} else if (g_vpbps[fd].filepointer == PSP_SEEK_END) {
		g_vpbps[fd].filepointer = g_vpbps[fd].filesize - (int)offset;
	} else {
		sceKernelSignalSema(g_vpsema, 1);
		return -1;
	}

	sceKernelSignalSema(g_vpsema, 1);
	return g_vpbps[fd].filepointer;
}

int virtualpbp_getstat(int i, SceIoStat *stat) {
	int res;

	sceKernelWaitSema(g_vpsema, 1, NULL);

	if (i < 0 || i >= g_index || g_states[i].deleted) {
		sceKernelSignalSema(g_vpsema, 1);
		return -1;
	}

	res =  sceIoGetstat(g_vpbps[i].isofile, stat);
	stat->st_size = g_vpbps[i].filesize;

	memcpy(&stat->sce_st_mtime, &stat->sce_st_ctime, sizeof(ScePspDateTime));

	sceKernelSignalSema(g_vpsema, 1);
	return res;
}

int virtualpbp_chstat(int i, SceIoStat *stat, int bits) {
	sceKernelWaitSema(g_vpsema, 1, NULL);

	if (i < 0 || i >= g_index) {
		sceKernelSignalSema(g_vpsema, 1);
		return -1;
	}

	sceIoChstat(g_vpbps[i].isofile, stat, bits);

	sceKernelSignalSema(g_vpsema, 1);
	return 0;
}

int virtualpbp_remove(int i) {
	sceKernelWaitSema(g_vpsema, 1, NULL);

	if (i < 0 || i >= g_index) {
		sceKernelSignalSema(g_vpsema, 1);
		return -1;
	}

	VshCtrlSetUmdFile("");
	int res = sceIoRemove(g_vpbps[i].isofile);
	if (res >= 0) {
		g_states[i].deleted = 1;
	}

	sceKernelSignalSema(g_vpsema, 1);
	return 0;
}

int virtualpbp_rmdir(int i) {
	sceKernelWaitSema(g_vpsema, 1, NULL);

	if (i < 0 || i >= g_index || g_states[i].psdirdeleted) {
		sceKernelSignalSema(g_vpsema, 1);
		return -1;
	}

	g_states[i].psdirdeleted = 1;

	sceKernelSignalSema(g_vpsema, 1);
	return 0;
}

int virtualpbp_dread(SceUID fd, SceIoDirent *dir) {
	SceFatMsDirentPrivate *private;

	sceKernelWaitSema(g_vpsema, 1, NULL);

	fd = fd - 0x7000;

	if (fd < 0 || fd >= g_index) {
		sceKernelSignalSema(g_vpsema, 1);
		return -1;
	}

	if (g_states[fd].dread == 2) {
		g_states[fd].dread = 0;
		sceKernelSignalSema(g_vpsema, 1);
		return 0;
	}

	sceIoGetstat(g_vpbps[fd].isofile, &dir->d_stat);
	memcpy(&dir->d_stat.sce_st_mtime, &dir->d_stat.sce_st_ctime, sizeof(ScePspDateTime));
	private = (SceFatMsDirentPrivate *)dir->d_private;

	if (g_states[fd].dread == 1) {
		dir->d_stat.st_size = g_vpbps[fd].filesize;
		strcpy(dir->d_name, "EBOOT.PBP");
		if (private) {
			strcpy(private->FileName, "EBOOT.PBP");
			strcpy(private->LongName, "EBOOT.PBP");
		}
	} else {
		dir->d_stat.st_size -= g_vpbps[fd].filesize;
		strcpy(dir->d_name, "IMAGE.ISO");
		if (private) {
			strcpy(private->FileName, "IMAGE.ISO");
			strcpy(private->LongName, "IMAGE.ISO");
		}
	}

	g_states[fd].dread++;
	sceKernelSignalSema(g_vpsema, 1);
	return 1;
}

char *virtualpbp_getfilename(int i) {
	sceKernelWaitSema(g_vpsema, 1, NULL);

	if (i < 0 || i >= g_index) {
		sceKernelSignalSema(g_vpsema, 1);
		return NULL;
	}

	sceKernelSignalSema(g_vpsema, 1);
	return g_vpbps[i].isofile;
}

int virtualpbp_get_isotype(int i) {
	sceKernelWaitSema(g_vpsema, 1, NULL);

	if (i < 0 || i >= g_index) {
		sceKernelSignalSema(g_vpsema, 1);
		return 0;
	}

	sceKernelSignalSema(g_vpsema, 1);
	return g_vpbps[i].opnssmp_type;
}

char *virtualpbp_getdiscid(int i) {
	sceKernelWaitSema(g_vpsema, 1, NULL);

	if (i < 0 || i >= g_index) {
		sceKernelSignalSema(g_vpsema, 1);
		return NULL;
	}

	sceKernelSignalSema(g_vpsema, 1);
	return g_vpbps[i].discid;
}

void virtualpbp_fixisopath(int index, char* path) {
	char *title_id = virtualpbp_getdiscid(index);

	char* tmp = strrchr(path, '/');
	char* filename = tmp+1;
	*tmp = 0;

	tmp = strrchr(path, '/');
	sprintf(tmp+1, "%s/%s", title_id, filename);
}