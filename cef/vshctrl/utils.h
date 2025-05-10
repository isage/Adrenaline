#ifndef __UTILS_H__
#define __UTILS_H__

#include <stddef.h>
#include <strings.h>
#include <string.h>

enum IsoRunlevel {
	ISO_RUNLEVEL = 0x123,
	ISO_RUNLEVEL_GO = 0x125,
	ISO_PBOOT_RUNLEVEL = 0x124,
	ISO_PBOOT_RUNLEVEL_GO = 0x126,
};

#endif