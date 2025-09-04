#ifndef __VGRAPH_H__
#define __VGRAPH_H__

#include <psptypes.h>
#include <pspmoduleinfo.h>

#ifdef __cplusplus
extern "C" {
#endif

void VGraphInit(u8 doublebuf);

void VGraphPrintf(const char *fmt, ...) __attribute__((format(printf,1,2)));
void VGraphPutc(u8 ch);

void VGraphSetBackColor(u32 color);
void VGraphSetTextColor(u32 color, u32 back);

//void VGraphPutChar(int x, int y, u32 color, u8 ch);

void VGraphGoto(int x, int y);

void VGraphClear(void);
void VGraphClearLine(u32 color);

void VGraphSwap();

#ifdef __cplusplus
}
#endif

#endif
