#ifndef __COMMON_H__
#define __COMMON_H__

#include <pspsdk.h>
#include <pspkernel.h>

#include <psperror.h>

#include <kubridge.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <psperror.h>

#include <pspexception.h>
#include <pspsysevent.h>
#include <psputilsforkernel.h>
#include <pspsysmem.h>
#include <psploadexec.h>
#include <pspthreadman_kernel.h>

#include <pspusb.h>
#include <pspusbstor.h>
#include <pspumd.h>
#include <psprtc.h>
#include <pspreg.h>
#include <pspctrl.h>
#include <psppower.h>
#include <pspaudio.h>
#include <pspdisplay.h>
#include <pspdisplay_kernel.h>
#include <psputility.h>
#include <pspcrypt.h>

#include <rebootexconfig.h>
#include <cfwmacros.h>

// If COMMON_H_SYSCLIB_USER is defined, it includes the sysclib_user.h definitions
// And other other headers from STD C must be manually included if necessary
#ifdef COMMON_H_SYSCLIB_USER
#include <sysclib_user.h>
#include <stdarg.h>
#else
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <malloc.h>
#endif

#include "../../adrenaline_compat.h"

#include <extratypes.h>

int sctrlGetUsbState();
int sctrlStartUsb();
int sctrlStopUsb();

#endif