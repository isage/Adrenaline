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
#include <pspsdk.h>
#include <systemctrl_se.h>
#include "pspcrypt.h"

// Calculate Random Number via KIRK
unsigned int sctrlKernelRand(void) {
    // Elevate Permission Level
    unsigned int k1 = pspSdkSetK1(0);

    // Allocate KIRK Buffer
    unsigned char * alloc = oe_malloc(20 + 4);

    // Allocation Error
    if(alloc == NULL) __asm__ volatile ("break");

    // Align Buffer to 4 Bytes
    unsigned char * buffer = (void *)(((unsigned int)alloc & (~(4-1))) + 4);

    // KIRK Random Generator Opcode
    enum {
        KIRK_PRNG_CMD=0xE,
    };

    // Create 20 Random Bytes
    sceUtilsBufferCopyWithRange(buffer, 20, NULL, 0, KIRK_PRNG_CMD);

    // Fetch Random Number
    unsigned int random = *(unsigned int *)buffer;

    // Free Buffer
    oe_free(alloc);

    // Restore Permission Level
    pspSdkSetK1(k1);

    // Return Random Number
    return random;
}
