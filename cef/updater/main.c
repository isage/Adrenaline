/*
	Adrenaline
	Copyright (C) 2016-2018, TheFloW

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
#include <stdarg.h>
#include <string.h>

#include <pspsdk.h>
#include <pspctrl.h>
#include <pspiofilemgr.h>

#include <systemctrl.h>
#include <systemctrl_se.h>
#include <systemctrl_adrenaline.h>

#include "frame_xml.h"
#include "template_xml.h"
#include "adrenaline_vsh.h"
#include "adrenaline_user.h"
#include "adrenaline_kernel.h"
#include "../../adrenaline_version.h"

PSP_MODULE_INFO("updater", 0x800, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VSH);

#define printf pspDebugScreenPrintf

typedef struct {
	char *path;
	void *buf;
	int size;
} File;

static File g_files[] = {
	{ "ms0:/__ADRENALINE__/sce_module/adrenaline_user.suprx", adrenaline_user, sizeof(adrenaline_user) },
	{ "ms0:/__ADRENALINE__/sce_module/adrenaline_kernel.skprx", adrenaline_kernel, sizeof(adrenaline_kernel) },
	{ "ms0:/__ADRENALINE__/sce_module/adrenaline_vsh.suprx", adrenaline_vsh, sizeof(adrenaline_vsh) },
	{ "ms0:/__ADRENALINE__/frame.xml", frame_xml, sizeof(frame_xml) },
	{ "ms0:/__ADRENALINE__/sce_sys/livearea/contents/template.xml", template_xml, sizeof(template_xml) },
};

void ErrorExit(int milisecs, char *fmt, ...) {
	va_list list;
	char msg[256];

	va_start(list, fmt);
	vsprintf(msg, fmt, list);
	va_end(list);

	printf(msg);

	sceKernelDelayThread(milisecs * 1000);
	sceKernelExitGame();
	sceKernelSleepThread();
}

int WriteFile(char *file, void *buf, int size) {
	SceUID fd = sceIoOpen(file, PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
	if (fd < 0)
		return fd;

	int written = sceIoWrite(fd, buf, size);

	sceIoClose(fd);
	return written;
}

int main(void) {
	pspDebugScreenInit();

	if (sctrlSEGetVersion() < 0x00060004) {
		ErrorExit(5000, "This update can only be applied with v6.4 or higher.\n");
	}

	#if defined(NIGHTLY) && NIGHTLY == 1
	if (sctrlSEGetVersion() > ADRENALINE_VERSION) {
		ErrorExit(5000, "This update or a higher one was already applied.\n");
	}
	#else
	if (sctrlSEGetVersion() >= ADRENALINE_VERSION) {
		ErrorExit(5000, "This update or a higher one was already applied.\n");
	}
	#endif

	#if defined(NIGHTLY) && NIGHTLY == 1
	printf("6.61 Adrenaline-%d.%d.%d-%s Installer\n", ADRENALINE_VERSION_MAJOR, ADRENALINE_VERSION_MINOR, ADRENALINE_VERSION_MICRO, EPI_NIGHTLY_VER);
	#else
	printf("6.61 Adrenaline-%d.%d.%d Installer\n", ADRENALINE_VERSION_MAJOR, ADRENALINE_VERSION_MINOR, ADRENALINE_VERSION_MICRO);
	#endif
	printf("Changes:\n\n");

	printf("- See this site http://adrenaline.sarcasticat.com/docs/XX-Changelog.html\n");

	printf("\n");

	printf("Press X to install, R to exit.\n\n");

	while (1) {
		SceCtrlData pad;
		sceCtrlReadBufferPositive(&pad, 1);

		if (pad.Buttons & PSP_CTRL_CROSS) {
			break;
		} else if (pad.Buttons & PSP_CTRL_RTRIGGER) {
			ErrorExit(5000, "Cancelled by user.\n");
		}
	}

	for (int i = 0; i < (sizeof(g_files) / sizeof(File)); i++) {
		char *p = strrchr(g_files[i].path, '/');
		printf("Writing %s (%d)... ", p+1, g_files[i].size);
		sceKernelDelayThread(100 * 1000);

		int written = WriteFile(g_files[i].path, g_files[i].buf, g_files[i].size);
		if (written != g_files[i].size) {
			ErrorExit(5000, "Error 0x%08X\n", written);
		}

		printf("OK\n");
		sceKernelDelayThread(100 * 1000);
	}

	printf("\nUpdate complete. Press X to reboot your device.\n\n");

	while (1) {
		SceCtrlData pad;
		sceCtrlReadBufferPositive(&pad, 1);

		if (pad.Buttons & PSP_CTRL_CROSS) {
			break;
		}
	}

	printf("Rebooting...\n");
	sceKernelDelayThread(2 * 1000 * 1000);
	sctrlRebootDevice();

	return 0;
}
