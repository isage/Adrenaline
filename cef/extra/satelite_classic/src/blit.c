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

#include <psptypes.h>
#include <pspdisplay.h>

#include "blit.h"

extern u8 msx[];

static int g_pwidth, g_pheight, g_bufferwidth, g_pixelformat;
static u32 *g_vram32;

__attribute__((noinline)) static u32 adjust_alpha(u32 col) {
	u32 alpha = col >> 24;

	if (alpha == 0) return col;
	if (alpha == 0xFF) return col;

	u32 c1 = col & 0x00FF00FF;
	u32 c2 = col & 0x0000FF00;
	u8 mul = (u8)(255 - alpha);
	c1 = ((c1 * mul) >> 8) & 0x00FF00FF;
	c2 = ((c2 * mul) >> 8) & 0x0000FF00;
	return (alpha << 24) | c1 | c2;
}

int blit_setup() {
	int unk;
	sceDisplayGetMode(&unk, &g_pwidth, &g_pheight);
	sceDisplayGetFrameBuf((void *)&g_vram32, &g_bufferwidth, &g_pixelformat, unk);
	if (g_bufferwidth == 0 || g_pixelformat != 3) {
		return -1;
	}

	return 0;
}

int blit_string(int sx, int sy, int fcolor, int bcolor, const char *msg) {
	u32 fg_col = adjust_alpha(fcolor);
	u32 bg_col = adjust_alpha(bcolor);

	if (g_bufferwidth == 0 || g_pixelformat != 3) {
		return -1;
	}

	int x;
	for (x = 0; msg[x] && x < (g_pwidth / 8); x++) {
		char code = msg[x] & 0x7F; // 7bit ANK

		int y;
		for (y = 0; y < 8; y++) {
			int offset = (sy + y) * g_bufferwidth + sx + x * 8;
			u8 font = y >= 7 ? 0x00 : msx[code * 8 + y];

			int p;
			for (p = 0; p < 8; p++) {
				u32 col = (font & 0x80) ? fg_col : bg_col;

				u32 alpha = col >> 24;
				if (alpha == 0) {
					g_vram32[offset] = col;
				} else if (alpha != 0xFF) {
					u32 c2 = g_vram32[offset];
					u32 c1 = c2 & 0x00FF00FF;
					c2 = c2 & 0x0000FF00;
					c1 = ((c1 * alpha) >> 8) & 0x00FF00FF;
					c2 = ((c2 * alpha) >> 8) & 0x0000FF00;
					g_vram32[offset] = (col & 0xFFFFFF) + c1 + c2;
				}

				font <<= 1;
				offset++;
			}
		}
	}

	return x;
}