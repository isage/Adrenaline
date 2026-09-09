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

#ifndef __BLIT_H__
#define __BLIT_H__

#define CENTER(x) (480 - (((x >> 1) << 1) * 8)) / 2

#define GLYPH_SQUARE   0x01
#define GLYPH_CIRCLE   0x02
#define GLYPH_TRIANGLE 0x03
#define GLYPH_CROSS    0x04
#define GLYPH_UP     0x05
#define GLYPH_DOWN   0x06
#define GLYPH_LEFT   0x07
#define GLYPH_RIGHT  0x08

extern u8 msx[];

int blit_setup();
int blit_string(int sx, int sy, int fcolor, int bcolor, const char *msg);
void blit_rect(int sx, int sy, int w, int h, unsigned int color);

#endif