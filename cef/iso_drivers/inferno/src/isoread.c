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
#include <pspsysmem.h>
#include <pspkernel.h>
#include <psputilsforkernel.h>
#include <pspthreadman_kernel.h>

#include <isoctrl.h>
#include <cfwmacros.h>
#include <systemctrl.h>
// #include <systemctrl_se.h>
#include <adrenaline_log.h>


#include "inferno.h"
#include "../bits/iso_common.h"

u8 g_umd_seek = 0;
u8 g_umd_speed = 0;
u8 g_umd_delay_strat = UMD_DELAY_STRAT_PER_FD;
u32 g_cur_offset = 0;
u32 g_last_read_offset = 0;

int (*iso_reader)(IoReadArg *args) = &iso_read;
int isoReadUmdFile(u32 offset, void *ptr, u32 data_len) {
	int ret = sceKernelWaitSema(g_umd9660_sema_id, 1, 0);

	if (ret < 0) {
		return -1;
	}

	g_read_arg.offset = offset;
	g_read_arg.address = ptr;
	g_read_arg.size = data_len;

	int retv = sceKernelExtendKernelStack(0x2000, (void*)iso_reader, &g_read_arg);

	if (g_umd_seek && g_umd_delay_strat == UMD_DELAY_STRAT_GLOBAL) {
		// simulate seek time
		u32 diff = 0;
		g_last_read_offset = offset+data_len;
		if (g_cur_offset > g_last_read_offset) {
			diff = g_cur_offset - g_last_read_offset;
		} else {
			diff = g_last_read_offset - g_cur_offset;
		}
		g_cur_offset = g_last_read_offset;
		u32 seek_time = (diff * g_umd_seek)/1024;
		sceKernelDelayThread(seek_time);
	}
	if (g_umd_speed && g_umd_delay_strat == UMD_DELAY_STRAT_GLOBAL) {
		// simulate read time
		sceKernelDelayThread(data_len * g_umd_speed);
	}

	ret = sceKernelSignalSema(g_umd9660_sema_id, 1);

	if (ret < 0) {
		return -1;
	}

	return retv;
}

void infernoSetUmdDelay(int seek, int speed) {
	g_umd_seek = seek;
	g_umd_speed = speed;
}

void isoSetUmdDelay(int seek, int speed, int strategy) {
	infernoSetUmdDelay(seek, speed);
	g_umd_delay_strat = strategy;
}