#include <string.h>

#include <psptypes.h>
#include <pspdisplay.h>

#include <systemctrl.h>
#include <systemctrl_se.h>

#include "blit.h"

static int g_pwidth = 0;
static int g_pheight = 0;
static int g_bufferwidth = 0;
static int g_pixelformat = 0;
static unsigned int *g_vram32 = NULL;

static const unsigned char button_glyphs[8][8] = {
	/* Square */
	{
		0xFF,
		0x81,
		0x81,
		0x81,
		0x81,
		0x81,
		0x81,
		0xFF
	},

	/* Circle */
	{
		0x3C,
		0x42,
		0x81,
		0x81,
		0x81,
		0x81,
		0x42,
		0x3C
	},

	/* Triangle */
	{
		0x10,
		0x10,
		0x28,
		0x28,
		0x44,
		0x44,
		0x82,
		0xFE
	},

	/* Cross */
	{
		0x81,
		0x42,
		0x24,
		0x18,
		0x18,
		0x24,
		0x42,
		0x81
	},

	/* D-pad Up */
    {
        0x18,
        0x3C,
        0x7E,
        0xFF,
        0x18,
        0x18,
        0x18,
        0x18
    },

    /* D-pad Down */
    {
        0x18,
        0x18,
        0x18,
        0x18,
        0xFF,
        0x7E,
        0x3C,
        0x18
    },

    /* D-pad Left */
    {
        0x18,
        0x0C,
        0x06,
        0xFF,
        0xFF,
        0x06,
        0x0C,
        0x18
    },

    /* D-pad Right */
    {
        0x18,
        0x30,
        0x60,
        0xFF,
        0xFF,
        0x60,
        0x30,
        0x18
    }
};

////////////////////////////////////////////////////////////////////////////////
// HELPERS
////////////////////////////////////////////////////////////////////////////////

static unsigned int adjust_alpha(unsigned int col) {
	unsigned int alpha = col >> 24;
	unsigned int c1;
	unsigned int c2;
	unsigned char mul;

	if (alpha == 0 || alpha == 0xFF) {
		return col;
	}

	c1 = col & 0x00FF00FF;
	c2 = col & 0x0000FF00;
	mul = (unsigned char)(255 - alpha);
	c1 = ((c1 * mul) >> 8) & 0x00FF00FF;
	c2 = ((c2 * mul) >> 8) & 0x0000FF00;

	return (alpha << 24) | c1 | c2;
}


////////////////////////////////////////////////////////////////////////////////
// Public API
////////////////////////////////////////////////////////////////////////////////

int blit_setup(void) {
	int mode;

	sceDisplayGetMode(&mode, &g_pwidth, &g_pheight);
	sceDisplayGetFrameBuf((void *)&g_vram32, &g_bufferwidth, &g_pixelformat, PSP_DISPLAY_SETBUF_IMMEDIATE);

	if (g_bufferwidth == 0 || g_pixelformat != 3 || g_vram32 == NULL) {
		return -1;
	}

	return 0;
}

void blit_rect(int sx, int sy, int w, int h, unsigned int color) {
	unsigned int col = adjust_alpha(color);
	unsigned int alpha = col >> 24;
	int x;
	int y;

	if (g_bufferwidth == 0 || g_pixelformat != 3 || g_vram32 == NULL)
		return;

	if (sx < 0) {
		w += sx;
		sx = 0;
	}
	if (sy < 0) {
		h += sy;
		sy = 0;
	}
	if (sx + w > g_pwidth) {
		w = g_pwidth - sx;
	}
	if (sy + h > g_pheight) {
		h = g_pheight - sy;
	}
	if (w <= 0 || h <= 0) {
		return;
	}

	for (y = 0; y < h; y++) {
		int offset = (sy + y) * g_bufferwidth + sx;

		for (x = 0; x < w; x++) {
			if (alpha == 0) {
				g_vram32[offset + x] = col;
			} else if (alpha != 0xFF) {
				unsigned int c2 = g_vram32[offset + x];
				unsigned int c1 = c2 & 0x00FF00FF;
				c2 &= 0x0000FF00;
				c1 = ((c1 * alpha) >> 8) & 0x00FF00FF;
				c2 = ((c2 * alpha) >> 8) & 0x0000FF00;
				g_vram32[offset + x] = (col & 0xFFFFFF) + c1 + c2;
			}
		}
	}
}

int blit_string(int sx, int sy, int fcolor, int bcolor, const char *msg) {
	unsigned int fg_col = adjust_alpha((unsigned int)fcolor);
	unsigned int bg_col = adjust_alpha((unsigned int)bcolor);
	int x;

	if (g_bufferwidth == 0 || g_pixelformat != 3 || g_vram32 == NULL) {
		return -1;
	}

	if (sx < 0 || sy < 0 || sy + 8 > g_pheight) {
		return -1;
	}

	for (x = 0; msg[x] && sx + (x + 1) * 8 <= g_pwidth; x++) {
		unsigned char code = (unsigned char)msg[x] & 0x7F;
		const unsigned char *glyph = NULL;

		if (code >= GLYPH_SQUARE && code <= GLYPH_RIGHT) {
			glyph = button_glyphs[code - GLYPH_SQUARE];
		} else {
			glyph = &msx[(code & 0x7F) * 8];
		}


		for (int y = 0; y < 8; y++) {
			int offset = (sy + y) * g_bufferwidth + sx + x * 8;
			unsigned char font = glyph[y];
			// unsigned char font = y >= 7 ? 0x00 : msx[code * 8 + y];
			int p;

			for (p = 0; p < 8; p++) {
				unsigned int col = (font & 0x80) ? fg_col : bg_col;
				unsigned int alpha = col >> 24;

				if (alpha == 0) {
					g_vram32[offset] = col;
				} else if (alpha != 0xFF) {
					unsigned int c2 = g_vram32[offset];
					unsigned int c1 = c2 & 0x00FF00FF;
					c2 &= 0x0000FF00;
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
