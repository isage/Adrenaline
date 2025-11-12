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

#include <pspkernel.h>
#include <pspreg.h>
#include <psputilsforkernel.h>
#include <pspsysmem.h>
#include <pspthreadman_kernel.h>

#include <systemctrl.h>
#include <systemctrl_se.h>
#include <adrenaline_log.h>
#include <iso_common.h>
#include <cfwmacros.h>

#include <stdio.h>
#include <string.h>

#include "utils.h"
#include "inferno.h"

u8 umd_seek = 0;
u8 umd_speed = 0;
u32 cur_offset = 0;
u32 last_read_offset = 0;

int (*iso_reader)(IoReadArg *args) = &iso_read;
int iso_read_with_stack(u32 offset, void *ptr, u32 data_len) {
    int ret = sceKernelWaitSema(g_umd9660_sema_id, 1, 0);

    if (ret < 0) {
        return -1;
    }

    g_read_arg.offset = offset;
    g_read_arg.address = ptr;
    g_read_arg.size = data_len;

    int retv = sceKernelExtendKernelStack(0x2000, (void*)iso_reader, &g_read_arg);

    ret = sceKernelSignalSema(g_umd9660_sema_id, 1);

    if (ret < 0) {
        return -1;
    }

    if (umd_seek){
        // simulate seek time
        u32 diff = 0;
        last_read_offset = offset+data_len;
        if (cur_offset > last_read_offset) {
			diff = cur_offset-last_read_offset;
		} else {
			diff = last_read_offset-cur_offset;
		}
        cur_offset = last_read_offset;
        u32 seek_time = (diff*umd_seek)/1024;
        sceKernelDelayThread(seek_time);
    }
    if (umd_speed){
        // simulate read time
        sceKernelDelayThread(data_len*umd_speed);
    }

    return retv;
}

void infernoSetUmdDelay(int seek, int speed) {
    umd_seek = seek;
    umd_speed = speed;
}