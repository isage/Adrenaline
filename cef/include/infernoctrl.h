/*
	Inferno UMDemu ISO driver API
	Copyright (C) 2025, Adrenaline Project
	Copyright (C) 2025, ARK-5 Project

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

/**
 * Header for the API of the Inferno UMDemu ISO driver.
 *
 * To use the functions, it is required to link to the driver stub.
 */

#ifndef INFERNO_CTRL_H
#define INFERNO_CTRL_H

#include <psptypes.h>

#include <isoctrl.h>

enum InfernoCachePolicy {
    INFERNO_CACHE_DISABLED = 0,
	// Least Recently Used
    INFERNO_CACHE_LRU = 1,
	// Random Replacement
    INFERNO_CACHE_RR = 2,
};

#ifdef __KERNEL__

/**
 * Issue a direct ISO/CSO read request to Inferno.
 *
 * @param offset The offset/sector within the Virtual UMD to start reading from. Absolute Offset.
 * @param buf The buffer to write data to.
 * @param size The amount of bytes to read.
 *
 * @return The amount of bytes read, `< 0` on error.
 */
int infernoIsoRead(u32 offset, void *buf, u32 size);

/**
 * Set UMD disc type (enum `pspUmdTypes`).
 *
 * @param type The type of the Virtual UMD. One of `pspUmdTypes` (at `pspumd.h`)
 *
 * @return `0` on success, `< 0` on error.
 */
int infernoSetDiscType(int type);

/**
 * Sets Cache Policy, necessary before initializing cache.
 *
 * @param policy The cache policy to use. One of `InfernoCachePolicy`.
 */
void infernoCacheSetPolicy(int policy);

/**
 * Initialize Inferno Cache. Requires Cache Policy to be set beforehand.
 *
 * If cache size is set to `0`, then cache is turned off.
 * Cache is not allowed on homebrew, it will return `0` but will not enable.
 *
 * @param cache_size The size of each cache block, in bytes.
 * @param cache_num The amount of cache blocks.
 * @param partition The ram partition where to allocate cache.
 *
 * @return `0` on success, `< 0` on error.
 * @return `-1` if total cache size (cache_size*cache_num) is not a multiple of `512`.
 * @return `-2`/`-3`/`-4` if failed to allocate cache buffers.
 * @return `-10` if cache policy not set.
 *
 */
int infernoCacheInit(int cache_size, int cache_num, int partition);

/**
 * Sets UMD speed simulation.
 *
 * @param seek The seek delay factor. `0` to disable, `> 0` to enable, the higher the value the slower the seek time.
 * @param speed The read speed delay factor. `0` to disable, `> 0` to enable, the higher the value the slower the read time.
 *
 */
void infernoSetUmdDelay(int seek, int speed);

#endif // __KERNEL__

#endif