#include <pspsdk.h>
#include <pspkernel.h>

#define _ADRENALINE_LOG_IMPL_
#include <adrenaline_log.h>

#include "umdman.h"
#include "umd9660.h"
#include "mediaman.h"

PSP_MODULE_INFO("EPI-March33Driver", 0x1006, 2, 0);
PSP_MAIN_THREAD_ATTR(0);

int module_start(SceSize args, void *argp) {
	logInit("ms0:/log_march33.txt");
	logmsg("March33 driver started...\n")

	int res = InitUmdMan();
	logmsg("%s: InitUmdMan -> 0x%08X\n", __func__, res);
	if (res < 0) {
		return res;
	}

	res = InitUmd9660();
	logmsg("%s: InitUmd9660 -> 0x%08X\n", __func__, res);
	if (res < 0) {
		return res;
	}

	res = InitMediaMan();
	logmsg("%s: InitMediaMan -> 0x%08X\n", __func__, res);
	if (res < 0) {
		return res;
	}

	return 0;
}

