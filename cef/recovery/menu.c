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

#include <common.h>

#include "main.h"
#include "menu.h"

typedef struct {
	Entry *entries;
	int n_entries;
	char *title;
	int x;
} MenuStruct;

MenuStruct menu_struct;

int sel = 0;
u32 select_color = 0x00FF0000;

void MenuResetSelection() {
	sel = 0;
	rDebugScreenClear();
}

void MenuReset(Entry *entries, int size_entries, char *title, int x) {
	menu_struct.entries = entries;
	menu_struct.n_entries = size_entries / sizeof(Entry);
	menu_struct.title = title;
	menu_struct.x = x;

	MenuResetSelection();
}

int WaitRelease(SceCtrlData *pad, u32 buttons) {
	if ((pad->Buttons & buttons) == buttons) {
		while ((pad->Buttons & buttons) == buttons)
			sceCtrlReadBufferPositive(pad, 1);

		return 1;
	}

	return 0;
}

void ChangeValue(int interval) {
	if (menu_struct.entries[sel].options) {
		int max = menu_struct.entries[sel].size_options / sizeof(char **);
		(*menu_struct.entries[sel].value) = (max + (*menu_struct.entries[sel].value) + interval) % max;
	}

	if (menu_struct.entries[sel].function) {
		if (strcmp(menu_struct.entries[sel].name, "Back") == 0) {
			printf(" > Back...");
			sceKernelDelayThread(1 * 300 * 1000);
		}
		menu_struct.entries[sel].function(sel);
	}

	if (menu_struct.entries[sel].options) {
		sceKernelDelayThread(100 * 1000);
		rDebugScreenClear();
	}
}

void MenuDisplayCtrl() {

	rDebugScreenSetTextColor(select_color, 0);
	rDebugScreenSetXY(0, 1);
	printf("Adrenaline Recovery Menu %d\n%s", sel, menu_struct.title);

	int y = 0;
	for (int i = 0; i < menu_struct.n_entries; i++) {
		rDebugScreenSetXY(menu_struct.x, 5 + i + y);

		if (strcmp(menu_struct.entries[i].name, "Back") == 0) {
			y = 1;
		}

		rDebugScreenClearLineWithColor(sel == i ? 0x181818 : 0);

		rDebugScreenSetTextColor(sel == i ? select_color : 0xFFFFFF, sel == i ? 0x181818 : 0);
		printf(menu_struct.entries[i].name);

		if (menu_struct.entries[i].options) {
			int max = menu_struct.entries[i].size_options / sizeof(char **);
			rDebugScreenSetXY(menu_struct.x + (68 - 22), 5 + i + y);
			rDebugScreenSetTextColor((*menu_struct.entries[i].value) == 0 ? 0x00FF00 : 0x0000FF, sel == i ? 0x181818 : 0);
			printf("%14s", menu_struct.entries[i].options[(*menu_struct.entries[i].value) % max]);
		}

		printf("\n");
	}


	rDebugScreenSetTextColor(select_color, 0);
	rDebugScreenSetXY(1, 29);

	for (int i = 0; i < 67; i++)
		printf("*");

	printf("\n");


	SceCtrlData pad;
	sceCtrlReadBufferPositive(&pad, 1);

	if (WaitRelease(&pad, PSP_CTRL_CROSS) || WaitRelease(&pad, PSP_CTRL_RIGHT)) {
		ChangeValue(+1);
	}
	if (WaitRelease(&pad, PSP_CTRL_LEFT)) {
		ChangeValue(-1);
	}
	if (WaitRelease(&pad, PSP_CTRL_UP)) {
		sel = (menu_struct.n_entries + sel - 1) % menu_struct.n_entries;
	}
	if (WaitRelease(&pad, PSP_CTRL_DOWN)) {
		sel = (menu_struct.n_entries + sel + 1) % menu_struct.n_entries;
	}
	if (WaitRelease(&pad, PSP_CTRL_TRIANGLE)) {
		sel = 0;
	}
}