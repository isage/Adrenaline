#include <psptypes.h>
#include <pspkernel.h>
#include <pspdisplay.h>
#include <pspge.h>
#include <stdio.h>
#include <stdarg.h>

#define PSP_SCREEN_WIDTH 480
#define PSP_SCREEN_HEIGHT 272
#define PSP_LINE_SIZE 512
#define PSP_PIXEL_FORMAT 3

static int X = 0, Y = 0;
static const int MX=60, MY=34;
static u32 bg_col = 0x00000000, fg_col = 0x00FFFFFF, fb_col = 0;

static u32* g_vram_base[2] = {(u32 *) 0x04000000, (u32 *) 0x04100000};
static u8 cur_buf = 0;
static int g_vram_offset = 0;

static int init = 0;

static u8 vgafont[] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x81, 0xa5, 0x81, 0xbd, 0x99, 0x81, 0x7e,
  0x7e, 0xff, 0xdb, 0xff, 0xc3, 0xe7, 0xff, 0x7e, 0x6c, 0xfe, 0xfe, 0xfe, 0x7c, 0x38, 0x10, 0x00,
  0x10, 0x38, 0x7c, 0xfe, 0x7c, 0x38, 0x10, 0x00, 0x38, 0x7c, 0x38, 0xfe, 0xfe, 0x7c, 0x38, 0x7c,
  0x10, 0x10, 0x38, 0x7c, 0xfe, 0x7c, 0x38, 0x7c, 0x00, 0x00, 0x18, 0x3c, 0x3c, 0x18, 0x00, 0x00,
  0xff, 0xff, 0xe7, 0xc3, 0xc3, 0xe7, 0xff, 0xff, 0x00, 0x3c, 0x66, 0x42, 0x42, 0x66, 0x3c, 0x00,
  0xff, 0xc3, 0x99, 0xbd, 0xbd, 0x99, 0xc3, 0xff, 0x0f, 0x07, 0x0f, 0x7d, 0xcc, 0xcc, 0xcc, 0x78,
  0x3c, 0x66, 0x66, 0x66, 0x3c, 0x18, 0x7e, 0x18, 0x3f, 0x33, 0x3f, 0x30, 0x30, 0x70, 0xf0, 0xe0,
  0x7f, 0x63, 0x7f, 0x63, 0x63, 0x67, 0xe6, 0xc0, 0x99, 0x5a, 0x3c, 0xe7, 0xe7, 0x3c, 0x5a, 0x99,
  0x80, 0xe0, 0xf8, 0xfe, 0xf8, 0xe0, 0x80, 0x00, 0x02, 0x0e, 0x3e, 0xfe, 0x3e, 0x0e, 0x02, 0x00,
  0x18, 0x3c, 0x7e, 0x18, 0x18, 0x7e, 0x3c, 0x18, 0x66, 0x66, 0x66, 0x66, 0x66, 0x00, 0x66, 0x00,
  0x7f, 0xdb, 0xdb, 0x7b, 0x1b, 0x1b, 0x1b, 0x00, 0x3e, 0x63, 0x38, 0x6c, 0x6c, 0x38, 0xcc, 0x78,
  0x00, 0x00, 0x00, 0x00, 0x7e, 0x7e, 0x7e, 0x00, 0x18, 0x3c, 0x7e, 0x18, 0x7e, 0x3c, 0x18, 0xff,
  0x18, 0x3c, 0x7e, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x18, 0x7e, 0x3c, 0x18, 0x00,
  0x00, 0x18, 0x0c, 0xfe, 0x0c, 0x18, 0x00, 0x00, 0x00, 0x30, 0x60, 0xfe, 0x60, 0x30, 0x00, 0x00,
  0x00, 0x00, 0xc0, 0xc0, 0xc0, 0xfe, 0x00, 0x00, 0x00, 0x24, 0x66, 0xff, 0x66, 0x24, 0x00, 0x00,
  0x00, 0x18, 0x3c, 0x7e, 0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0x7e, 0x3c, 0x18, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x78, 0x78, 0x30, 0x30, 0x00, 0x30, 0x00,
  0x6c, 0x6c, 0x6c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x6c, 0x6c, 0xfe, 0x6c, 0xfe, 0x6c, 0x6c, 0x00,
  0x30, 0x7c, 0xc0, 0x78, 0x0c, 0xf8, 0x30, 0x00, 0x00, 0xc6, 0xcc, 0x18, 0x30, 0x66, 0xc6, 0x00,
  0x38, 0x6c, 0x38, 0x76, 0xdc, 0xcc, 0x76, 0x00, 0x60, 0x60, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x18, 0x30, 0x60, 0x60, 0x60, 0x30, 0x18, 0x00, 0x60, 0x30, 0x18, 0x18, 0x18, 0x30, 0x60, 0x00,
  0x00, 0x66, 0x3c, 0xff, 0x3c, 0x66, 0x00, 0x00, 0x00, 0x30, 0x30, 0xfc, 0x30, 0x30, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x60, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x00, 0x06, 0x0c, 0x18, 0x30, 0x60, 0xc0, 0x80, 0x00,
  0x7c, 0xc6, 0xce, 0xde, 0xf6, 0xe6, 0x7c, 0x00, 0x30, 0x70, 0x30, 0x30, 0x30, 0x30, 0xfc, 0x00,
  0x78, 0xcc, 0x0c, 0x38, 0x60, 0xcc, 0xfc, 0x00, 0x78, 0xcc, 0x0c, 0x38, 0x0c, 0xcc, 0x78, 0x00,
  0x1c, 0x3c, 0x6c, 0xcc, 0xfe, 0x0c, 0x1e, 0x00, 0xfc, 0xc0, 0xf8, 0x0c, 0x0c, 0xcc, 0x78, 0x00,
  0x38, 0x60, 0xc0, 0xf8, 0xcc, 0xcc, 0x78, 0x00, 0xfc, 0xcc, 0x0c, 0x18, 0x30, 0x30, 0x30, 0x00,
  0x78, 0xcc, 0xcc, 0x78, 0xcc, 0xcc, 0x78, 0x00, 0x78, 0xcc, 0xcc, 0x7c, 0x0c, 0x18, 0x70, 0x00,
  0x00, 0x30, 0x30, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x30, 0x30, 0x00, 0x00, 0x30, 0x30, 0x60,
  0x18, 0x30, 0x60, 0xc0, 0x60, 0x30, 0x18, 0x00, 0x00, 0x00, 0xfc, 0x00, 0x00, 0xfc, 0x00, 0x00,
  0x60, 0x30, 0x18, 0x0c, 0x18, 0x30, 0x60, 0x00, 0x78, 0xcc, 0x0c, 0x18, 0x30, 0x00, 0x30, 0x00,
  0x7c, 0xc6, 0xde, 0xde, 0xde, 0xc0, 0x78, 0x00, 0x30, 0x78, 0xcc, 0xcc, 0xfc, 0xcc, 0xcc, 0x00,
  0xfc, 0x66, 0x66, 0x7c, 0x66, 0x66, 0xfc, 0x00, 0x3c, 0x66, 0xc0, 0xc0, 0xc0, 0x66, 0x3c, 0x00,
  0xf8, 0x6c, 0x66, 0x66, 0x66, 0x6c, 0xf8, 0x00, 0xfe, 0x62, 0x68, 0x78, 0x68, 0x62, 0xfe, 0x00,
  0xfe, 0x62, 0x68, 0x78, 0x68, 0x60, 0xf0, 0x00, 0x3c, 0x66, 0xc0, 0xc0, 0xce, 0x66, 0x3e, 0x00,
  0xcc, 0xcc, 0xcc, 0xfc, 0xcc, 0xcc, 0xcc, 0x00, 0x78, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00,
  0x1e, 0x0c, 0x0c, 0x0c, 0xcc, 0xcc, 0x78, 0x00, 0xe6, 0x66, 0x6c, 0x78, 0x6c, 0x66, 0xe6, 0x00,
  0xf0, 0x60, 0x60, 0x60, 0x62, 0x66, 0xfe, 0x00, 0xc6, 0xee, 0xfe, 0xfe, 0xd6, 0xc6, 0xc6, 0x00,
  0xc6, 0xe6, 0xf6, 0xde, 0xce, 0xc6, 0xc6, 0x00, 0x38, 0x6c, 0xc6, 0xc6, 0xc6, 0x6c, 0x38, 0x00,
  0xfc, 0x66, 0x66, 0x7c, 0x60, 0x60, 0xf0, 0x00, 0x78, 0xcc, 0xcc, 0xcc, 0xdc, 0x78, 0x1c, 0x00,
  0xfc, 0x66, 0x66, 0x7c, 0x6c, 0x66, 0xe6, 0x00, 0x78, 0xcc, 0xe0, 0x70, 0x1c, 0xcc, 0x78, 0x00,
  0xfc, 0xb4, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xfc, 0x00,
  0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x78, 0x30, 0x00, 0xc6, 0xc6, 0xc6, 0xd6, 0xfe, 0xee, 0xc6, 0x00,
  0xc6, 0xc6, 0x6c, 0x38, 0x38, 0x6c, 0xc6, 0x00, 0xcc, 0xcc, 0xcc, 0x78, 0x30, 0x30, 0x78, 0x00,
  0xfe, 0xc6, 0x8c, 0x18, 0x32, 0x66, 0xfe, 0x00, 0x78, 0x60, 0x60, 0x60, 0x60, 0x60, 0x78, 0x00,
  0xc0, 0x60, 0x30, 0x18, 0x0c, 0x06, 0x02, 0x00, 0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78, 0x00,
  0x10, 0x38, 0x6c, 0xc6, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
  0x30, 0x30, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x78, 0x0c, 0x7c, 0xcc, 0x76, 0x00,
  0xe0, 0x60, 0x60, 0x7c, 0x66, 0x66, 0xdc, 0x00, 0x00, 0x00, 0x78, 0xcc, 0xc0, 0xcc, 0x78, 0x00,
  0x1c, 0x0c, 0x0c, 0x7c, 0xcc, 0xcc, 0x76, 0x00, 0x00, 0x00, 0x78, 0xcc, 0xfc, 0xc0, 0x78, 0x00,
  0x38, 0x6c, 0x60, 0xf0, 0x60, 0x60, 0xf0, 0x00, 0x00, 0x00, 0x76, 0xcc, 0xcc, 0x7c, 0x0c, 0xf8,
  0xe0, 0x60, 0x6c, 0x76, 0x66, 0x66, 0xe6, 0x00, 0x30, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00,
  0x0c, 0x00, 0x0c, 0x0c, 0x0c, 0xcc, 0xcc, 0x78, 0xe0, 0x60, 0x66, 0x6c, 0x78, 0x6c, 0xe6, 0x00,
  0x70, 0x30, 0x30, 0x30, 0x30, 0x30, 0x78, 0x00, 0x00, 0x00, 0xcc, 0xfe, 0xfe, 0xd6, 0xc6, 0x00,
  0x00, 0x00, 0xf8, 0xcc, 0xcc, 0xcc, 0xcc, 0x00, 0x00, 0x00, 0x78, 0xcc, 0xcc, 0xcc, 0x78, 0x00,
  0x00, 0x00, 0xdc, 0x66, 0x66, 0x7c, 0x60, 0xf0, 0x00, 0x00, 0x76, 0xcc, 0xcc, 0x7c, 0x0c, 0x1e,
  0x00, 0x00, 0xdc, 0x76, 0x66, 0x60, 0xf0, 0x00, 0x00, 0x00, 0x7c, 0xc0, 0x78, 0x0c, 0xf8, 0x00,
  0x10, 0x30, 0x7c, 0x30, 0x30, 0x34, 0x18, 0x00, 0x00, 0x00, 0xcc, 0xcc, 0xcc, 0xcc, 0x76, 0x00,
  0x00, 0x00, 0xcc, 0xcc, 0xcc, 0x78, 0x30, 0x00, 0x00, 0x00, 0xc6, 0xd6, 0xfe, 0xfe, 0x6c, 0x00,
  0x00, 0x00, 0xc6, 0x6c, 0x38, 0x6c, 0xc6, 0x00, 0x00, 0x00, 0xcc, 0xcc, 0xcc, 0x7c, 0x0c, 0xf8,
  0x00, 0x00, 0xfc, 0x98, 0x30, 0x64, 0xfc, 0x00, 0x1c, 0x30, 0x30, 0xe0, 0x30, 0x30, 0x1c, 0x00,
  0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x00, 0xe0, 0x30, 0x30, 0x1c, 0x30, 0x30, 0xe0, 0x00,
  0x76, 0xdc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x38, 0x6c, 0xc6, 0xc6, 0xfe, 0x00,
  0x78, 0xcc, 0xc0, 0xcc, 0x78, 0x18, 0x0c, 0x78, 0x00, 0xcc, 0x00, 0xcc, 0xcc, 0xcc, 0x7e, 0x00,
  0x1c, 0x00, 0x78, 0xcc, 0xfc, 0xc0, 0x78, 0x00, 0x7e, 0xc3, 0x3c, 0x06, 0x3e, 0x66, 0x3f, 0x00,
  0xcc, 0x00, 0x78, 0x0c, 0x7c, 0xcc, 0x7e, 0x00, 0xe0, 0x00, 0x78, 0x0c, 0x7c, 0xcc, 0x7e, 0x00,
  0x30, 0x30, 0x78, 0x0c, 0x7c, 0xcc, 0x7e, 0x00, 0x00, 0x00, 0x78, 0xc0, 0xc0, 0x78, 0x0c, 0x38,
  0x7e, 0xc3, 0x3c, 0x66, 0x7e, 0x60, 0x3c, 0x00, 0xcc, 0x00, 0x78, 0xcc, 0xfc, 0xc0, 0x78, 0x00,
  0xe0, 0x00, 0x78, 0xcc, 0xfc, 0xc0, 0x78, 0x00, 0xcc, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00,
  0x7c, 0xc6, 0x38, 0x18, 0x18, 0x18, 0x3c, 0x00, 0xe0, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00,
  0xc6, 0x38, 0x6c, 0xc6, 0xfe, 0xc6, 0xc6, 0x00, 0x30, 0x30, 0x00, 0x78, 0xcc, 0xfc, 0xcc, 0x00,
  0x1c, 0x00, 0xfc, 0x60, 0x78, 0x60, 0xfc, 0x00, 0x00, 0x00, 0x7f, 0x0c, 0x7f, 0xcc, 0x7f, 0x00,
  0x3e, 0x6c, 0xcc, 0xfe, 0xcc, 0xcc, 0xce, 0x00, 0x78, 0xcc, 0x00, 0x78, 0xcc, 0xcc, 0x78, 0x00,
  0x00, 0xcc, 0x00, 0x78, 0xcc, 0xcc, 0x78, 0x00, 0x00, 0xe0, 0x00, 0x78, 0xcc, 0xcc, 0x78, 0x00,
  0x78, 0xcc, 0x00, 0xcc, 0xcc, 0xcc, 0x7e, 0x00, 0x00, 0xe0, 0x00, 0xcc, 0xcc, 0xcc, 0x7e, 0x00,
  0x00, 0xcc, 0x00, 0xcc, 0xcc, 0x7c, 0x0c, 0xf8, 0xc3, 0x18, 0x3c, 0x66, 0x66, 0x3c, 0x18, 0x00,
  0xcc, 0x00, 0xcc, 0xcc, 0xcc, 0xcc, 0x78, 0x00, 0x18, 0x18, 0x7e, 0xc0, 0xc0, 0x7e, 0x18, 0x18,
  0x38, 0x6c, 0x64, 0xf0, 0x60, 0xe6, 0xfc, 0x00, 0xcc, 0xcc, 0x78, 0xfc, 0x30, 0xfc, 0x30, 0x30,
  0xf8, 0xcc, 0xcc, 0xfa, 0xc6, 0xcf, 0xc6, 0xc7, 0x0e, 0x1b, 0x18, 0x3c, 0x18, 0x18, 0xd8, 0x70,
  0x1c, 0x00, 0x78, 0x0c, 0x7c, 0xcc, 0x7e, 0x00, 0x38, 0x00, 0x70, 0x30, 0x30, 0x30, 0x78, 0x00,
  0x00, 0x1c, 0x00, 0x78, 0xcc, 0xcc, 0x78, 0x00, 0x00, 0x1c, 0x00, 0xcc, 0xcc, 0xcc, 0x7e, 0x00,
  0x00, 0xf8, 0x00, 0xf8, 0xcc, 0xcc, 0xcc, 0x00, 0xfc, 0x00, 0xcc, 0xec, 0xfc, 0xdc, 0xcc, 0x00,
  0x3c, 0x6c, 0x6c, 0x3e, 0x00, 0x7e, 0x00, 0x00, 0x38, 0x6c, 0x6c, 0x38, 0x00, 0x7c, 0x00, 0x00,
  0x30, 0x00, 0x30, 0x60, 0xc0, 0xcc, 0x78, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xc0, 0xc0, 0x00, 0x00,
  0x00, 0x00, 0x00, 0xfc, 0x0c, 0x0c, 0x00, 0x00, 0xc3, 0xc6, 0xcc, 0xde, 0x33, 0x66, 0xcc, 0x0f,
  0xc3, 0xc6, 0xcc, 0xdb, 0x37, 0x6f, 0xcf, 0x03, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18, 0x18, 0x00,
  0x00, 0x33, 0x66, 0xcc, 0x66, 0x33, 0x00, 0x00, 0x00, 0xcc, 0x66, 0x33, 0x66, 0xcc, 0x00, 0x00,
  0x22, 0x88, 0x22, 0x88, 0x22, 0x88, 0x22, 0x88, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa, 0x55, 0xaa,
  0xdb, 0x77, 0xdb, 0xee, 0xdb, 0x77, 0xdb, 0xee, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
  0x18, 0x18, 0x18, 0x18, 0xf8, 0x18, 0x18, 0x18, 0x18, 0x18, 0xf8, 0x18, 0xf8, 0x18, 0x18, 0x18,
  0x36, 0x36, 0x36, 0x36, 0xf6, 0x36, 0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x36, 0x36, 0x36,
  0x00, 0x00, 0xf8, 0x18, 0xf8, 0x18, 0x18, 0x18, 0x36, 0x36, 0xf6, 0x06, 0xf6, 0x36, 0x36, 0x36,
  0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x00, 0x00, 0xfe, 0x06, 0xf6, 0x36, 0x36, 0x36,
  0x36, 0x36, 0xf6, 0x06, 0xfe, 0x00, 0x00, 0x00, 0x36, 0x36, 0x36, 0x36, 0xfe, 0x00, 0x00, 0x00,
  0x18, 0x18, 0xf8, 0x18, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x18, 0x18, 0x18,
  0x18, 0x18, 0x18, 0x18, 0x1f, 0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0xff, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0xff, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1f, 0x18, 0x18, 0x18,
  0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0x00, 0x00, 0x18, 0x18, 0x18, 0x18, 0xff, 0x18, 0x18, 0x18,
  0x18, 0x18, 0x1f, 0x18, 0x1f, 0x18, 0x18, 0x18, 0x36, 0x36, 0x36, 0x36, 0x37, 0x36, 0x36, 0x36,
  0x36, 0x36, 0x37, 0x30, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0x30, 0x37, 0x36, 0x36, 0x36,
  0x36, 0x36, 0xf7, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0xf7, 0x36, 0x36, 0x36,
  0x36, 0x36, 0x37, 0x30, 0x37, 0x36, 0x36, 0x36, 0x00, 0x00, 0xff, 0x00, 0xff, 0x00, 0x00, 0x00,
  0x36, 0x36, 0xf7, 0x00, 0xf7, 0x36, 0x36, 0x36, 0x18, 0x18, 0xff, 0x00, 0xff, 0x00, 0x00, 0x00,
  0x36, 0x36, 0x36, 0x36, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0xff, 0x18, 0x18, 0x18,
  0x00, 0x00, 0x00, 0x00, 0xff, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x3f, 0x00, 0x00, 0x00,
  0x18, 0x18, 0x1f, 0x18, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0x18, 0x1f, 0x18, 0x18, 0x18,
  0x00, 0x00, 0x00, 0x00, 0x3f, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0x36, 0xff, 0x36, 0x36, 0x36,
  0x18, 0x18, 0xff, 0x18, 0xff, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xf8, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x1f, 0x18, 0x18, 0x18, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
  0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x76, 0xdc, 0xc8, 0xdc, 0x76, 0x00, 0x00, 0x78, 0xcc, 0xf8, 0xcc, 0xf8, 0xc0, 0xc0,
  0x00, 0xfc, 0xcc, 0xc0, 0xc0, 0xc0, 0xc0, 0x00, 0x00, 0xfe, 0x6c, 0x6c, 0x6c, 0x6c, 0x6c, 0x00,
  0xfc, 0xcc, 0x60, 0x30, 0x60, 0xcc, 0xfc, 0x00, 0x00, 0x00, 0x7e, 0xd8, 0xd8, 0xd8, 0x70, 0x00,
  0x00, 0x66, 0x66, 0x66, 0x66, 0x7c, 0x60, 0xc0, 0x00, 0x76, 0xdc, 0x18, 0x18, 0x18, 0x18, 0x00,
  0xfc, 0x30, 0x78, 0xcc, 0xcc, 0x78, 0x30, 0xfc, 0x38, 0x6c, 0xc6, 0xfe, 0xc6, 0x6c, 0x38, 0x00,
  0x38, 0x6c, 0xc6, 0xc6, 0x6c, 0x6c, 0xee, 0x00, 0x1c, 0x30, 0x18, 0x7c, 0xcc, 0xcc, 0x78, 0x00,
  0x00, 0x00, 0x7e, 0xdb, 0xdb, 0x7e, 0x00, 0x00, 0x06, 0x0c, 0x7e, 0xdb, 0xdb, 0x7e, 0x60, 0xc0,
  0x38, 0x60, 0xc0, 0xf8, 0xc0, 0x60, 0x38, 0x00, 0x78, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x00,
  0x00, 0xfc, 0x00, 0xfc, 0x00, 0xfc, 0x00, 0x00, 0x30, 0x30, 0xfc, 0x30, 0x30, 0x00, 0xfc, 0x00,
  0x60, 0x30, 0x18, 0x30, 0x60, 0x00, 0xfc, 0x00, 0x18, 0x30, 0x60, 0x30, 0x18, 0x00, 0xfc, 0x00,
  0x0e, 0x1b, 0x1b, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xd8, 0xd8, 0x70,
  0x30, 0x30, 0x00, 0xfc, 0x00, 0x30, 0x30, 0x00, 0x00, 0x76, 0xdc, 0x00, 0x76, 0xdc, 0x00, 0x00,
  0x38, 0x6c, 0x6c, 0x38, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x0f, 0x0c, 0x0c, 0x0c, 0xec, 0x6c, 0x3c, 0x1c,
  0x78, 0x6c, 0x6c, 0x6c, 0x6c, 0x00, 0x00, 0x00, 0x70, 0x18, 0x30, 0x60, 0x78, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x3c, 0x3c, 0x3c, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static uint32_t vga_palette[256] = {
  0xFF000000, 0xFFAA0000, 0xFF00AA00, 0xFFAAAA00, 0xFF0000AA, 0xFFAA00AA, 0xFF0055AA, 0xFFAAAAAA,
  0xFF555555, 0xFFFF5555, 0xFF55FF55, 0xFFFFFF55, 0xFF5555FF, 0xFFFF55FF, 0xFF55FFFF, 0xFFFFFFFF,
  0xFF000000, 0xFF141414, 0xFF202020, 0xFF2C2C2C, 0xFF383838, 0xFF454545, 0xFF515151, 0xFF616161,
  0xFF717171, 0xFF828282, 0xFF929292, 0xFFA2A2A2, 0xFFB6B6B6, 0xFFCBCBCB, 0xFFE3E3E3, 0xFFFFFFFF,
  0xFFFF0000, 0xFFFF0041, 0xFFFF007D, 0xFFFF00BE, 0xFFFF00FF, 0xFFBE00FF, 0xFF7D00FF, 0xFF4100FF,
  0xFF0000FF, 0xFF0041FF, 0xFF007DFF, 0xFF00BEFF, 0xFF00FFFF, 0xFF00FFBE, 0xFF00FF7D, 0xFF00FF41,
  0xFF00FF00, 0xFF41FF00, 0xFF7DFF00, 0xFFBEFF00, 0xFFFFFF00, 0xFFFFBE00, 0xFFFF7D00, 0xFFFF4100,
  0xFFFF7D7D, 0xFFFF7D9E, 0xFFFF7DBE, 0xFFFF7DDF, 0xFFFF7DFF, 0xFFDF7DFF, 0xFFBE7DFF, 0xFF9E7DFF,
  0xFF7D7DFF, 0xFF7D9EFF, 0xFF7DBEFF, 0xFF7DDFFF, 0xFF7DFFFF, 0xFF7DFFDF, 0xFF7DFFBE, 0xFF7DFF9E,
  0xFF7DFF7D, 0xFF9EFF7D, 0xFFBEFF7D, 0xFFDFFF7D, 0xFFFFFF7D, 0xFFFFDF7D, 0xFFFFBE7D, 0xFFFF9E7D,
  0xFFFFB6B6, 0xFFFFB6C7, 0xFFFFB6DB, 0xFFFFB6EB, 0xFFFFB6FF, 0xFFEBB6FF, 0xFFDBB6FF, 0xFFC7B6FF,
  0xFFB6B6FF, 0xFFB6C7FF, 0xFFB6DBFF, 0xFFB6EBFF, 0xFFB6FFFF, 0xFFB6FFEB, 0xFFB6FFDB, 0xFFB6FFC7,
  0xFFB6FFB6, 0xFFC7FFB6, 0xFFDBFFB6, 0xFFEBFFB6, 0xFFFFFFB6, 0xFFFFEBB6, 0xFFFFDBB6, 0xFFFFC7B6,
  0xFF710000, 0xFF71001C, 0xFF710038, 0xFF710055, 0xFF710071, 0xFF550071, 0xFF380071, 0xFF1C0071,
  0xFF000071, 0xFF001C71, 0xFF003871, 0xFF005571, 0xFF007171, 0xFF007155, 0xFF007138, 0xFF00711C,
  0xFF007100, 0xFF1C7100, 0xFF387100, 0xFF557100, 0xFF717100, 0xFF715500, 0xFF713800, 0xFF711C00,
  0xFF713838, 0xFF713845, 0xFF713855, 0xFF713861, 0xFF713871, 0xFF613871, 0xFF553871, 0xFF453871,
  0xFF383871, 0xFF384571, 0xFF385571, 0xFF386171, 0xFF387171, 0xFF387161, 0xFF387155, 0xFF387145,
  0xFF387138, 0xFF457138, 0xFF557138, 0xFF617138, 0xFF717138, 0xFF716138, 0xFF715538, 0xFF714538,
  0xFF715151, 0xFF715159, 0xFF715161, 0xFF715169, 0xFF715171, 0xFF695171, 0xFF615171, 0xFF595171,
  0xFF515171, 0xFF515971, 0xFF516171, 0xFF516971, 0xFF517171, 0xFF517169, 0xFF517161, 0xFF517159,
  0xFF517151, 0xFF597151, 0xFF617151, 0xFF697151, 0xFF717151, 0xFF716951, 0xFF716151, 0xFF715951,
  0xFF410000, 0xFF410010, 0xFF410020, 0xFF410030, 0xFF410041, 0xFF300041, 0xFF200041, 0xFF100041,
  0xFF000041, 0xFF001041, 0xFF002041, 0xFF003041, 0xFF004141, 0xFF004130, 0xFF004120, 0xFF004110,
  0xFF004100, 0xFF104100, 0xFF204100, 0xFF304100, 0xFF414100, 0xFF413000, 0xFF412000, 0xFF411000,
  0xFF412020, 0xFF412028, 0xFF412030, 0xFF412038, 0xFF412041, 0xFF382041, 0xFF302041, 0xFF282041,
  0xFF202041, 0xFF202841, 0xFF203041, 0xFF203841, 0xFF204141, 0xFF204138, 0xFF204130, 0xFF204128,
  0xFF204120, 0xFF284120, 0xFF304120, 0xFF384120, 0xFF414120, 0xFF413820, 0xFF413020, 0xFF412820,
  0xFF412C2C, 0xFF412C30, 0xFF412C34, 0xFF412C3C, 0xFF412C41, 0xFF3C2C41, 0xFF342C41, 0xFF302C41,
  0xFF2C2C41, 0xFF2C3041, 0xFF2C3441, 0xFF2C3C41, 0xFF2C4141, 0xFF2C413C, 0xFF2C4134, 0xFF2C4130,
  0xFF2C412C, 0xFF30412C, 0xFF34412C, 0xFF3C412C, 0xFF41412C, 0xFF413C2C, 0xFF41342C, 0xFF41302C,
  0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0xFF000000, 0x00000000
};

static void clear_screen(u32 color)
{
  int x;

  if (!init) return;

  u32 *vram = g_vram_base[cur_buf] + (g_vram_offset>>1);

  for(x = 0; x < (PSP_LINE_SIZE * PSP_SCREEN_HEIGHT); x++)
  {
    *vram++ = color;
  }
}

static void clear_line(int Y, u32 color)
{
  int x;

  if (!init) return;

  u32 *vram = g_vram_base[cur_buf] + (g_vram_offset>>1);
  vram += ( Y * PSP_LINE_SIZE) * 8;

  for(x = 0; x < (PSP_LINE_SIZE * 8); x++)
  {
    *vram++ = color;
  }
}

static void put_char(int x, int y, u32 color, u8 ch)
{
  int i,j, l;
  u8  *font;
  u32  pixel;
  u32 *vram_ptr;
  u32 *vram;

  if (!init) return;

  vram = g_vram_base[cur_buf] + (g_vram_offset>>1) + x;
  vram += (y * PSP_LINE_SIZE);

  font = &vgafont[ (int)ch * 8];
  for (i = l = 0; i < 8; i++, l += 8, font++)
  {
    vram_ptr  = vram;
    for (j = 0; j < 8; j++)
    {
      if ((*font & (128 >> j)))
      {
        pixel = color;
      }
      else
      {
        pixel = fb_col;
      }

      *vram_ptr++ = pixel;
    }
    vram += PSP_LINE_SIZE;
  }
}

void VGraphInit(u8 doublebuf)
{
  X = Y = 0;
  /* Place vram in uncached memory */
  g_vram_base[0] = (void *)(0x40000000 | (u32)sceGeEdramGetAddr());
  g_vram_base[1] = (void *)(0x40000000 | ((u32)sceGeEdramGetAddr()+0x100000));
  g_vram_offset = 0;
  sceDisplaySetMode(0, PSP_SCREEN_WIDTH, PSP_SCREEN_HEIGHT);
  sceDisplaySetFrameBuf((void *)g_vram_base[cur_buf], PSP_LINE_SIZE, PSP_PIXEL_FORMAT, 1);
  if (doublebuf)
    cur_buf = 1;
  clear_screen(bg_col);
  init = 1;
}

void VGraphSetBackColor(u32 colour)
{
  bg_col = vga_palette[colour];
}

void VGraphSetTextColor(u32 colour, u32 back)
{
  fg_col = vga_palette[colour];
  fb_col = vga_palette[back];
}


void VGraphClearLine(u32 color)
{
  clear_line(Y, vga_palette[color]);
}

/* Print non-nul terminated strings */
int VGraphPrintData(const char *buff, int size)
{
  int i;
  int j;
  char c;

  for (i = 0; i < size; i++)
  {
    c = buff[i];
    switch (c)
    {
      case '\n':
        X = 0;
        Y++;
        if (Y == MY)
          Y = 0;
        clear_line(Y, bg_col);
        break;
      case '\t':
        for (j = 0; j < 4; j++)
        {
          put_char(X * 8, Y * 8, fg_col, ' ');
          X++;
        }
        break;
      default:
        put_char(X * 8, Y * 8, fg_col, c);
        X++;
        if (X == MX)
        {
          X = 0;
          Y++;
          if (Y == MY)
            Y = 0;
//          clear_line(Y, bg_col);
        }
    }
  }

  return i;
}

void VGraphPutc(u8 ch)
{
  put_char(X * 8, Y * 8, fg_col, ch);
  X++;
  if (X == MX)
  {
    X = 0;
    Y++;
    if (Y == MY)
      Y = 0;
//    clear_line(Y, bg_col);
  }
}

void VGraphPrintf(const char *format, ...)
{
  va_list opt;
  char buff[2048];
  int bufsz;

  va_start(opt, format);
  bufsz = vsnprintf(buff, (size_t)sizeof(buff), format, opt);
  va_end ( opt );

  (void) VGraphPrintData(buff, bufsz);
}


void VGraphGoto(int x, int y)
{
  if (x < MX && x >= 0) X = x;
  if (y < MY && y >= 0) Y = y;
}

void VGraphClear()
{
  VGraphGoto(0,0);
  clear_screen(bg_col);
}

void VGraphSwap()
{
  sceDisplaySetFrameBuf((void *)g_vram_base[cur_buf], PSP_LINE_SIZE, PSP_PIXEL_FORMAT, 1);
  cur_buf = (cur_buf) ? 0 : 1;
}
