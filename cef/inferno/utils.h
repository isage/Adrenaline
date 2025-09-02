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

#ifndef UTILS_H
#define UTILS_H

#include <psptypes.h>

typedef unsigned int uint;


int sceKernelGetModel(void);

void sync_cache(void);

/**
 * @return interrupted status
 * 0 - we are definitely in interrupt disabled status. And the interrupt status won't change as long as our code didn't
 * 1 - we are in interrupt enable status. but the interrupt status would change in later code
 */
int is_cpu_intr_enable(void);

#ifdef DEBUG
void hexdump(void *addr, int size);
void fill_vram(u32 color);
#else
static inline void hexdump(void *addr, int size) {}
static inline void fill_vram(u32 color) {}
#endif

int get_device_name(char *device, int size, const char* path);

SceUID get_thread_id(const char *name);

/** Check if user syscall didn't pass kernel memory, as OFW did */
int check_memory(const void *addr, int size);

#endif
