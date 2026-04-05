#ifndef __ME_DRIVER_EXTERNS_H_
#define __ME_DRIVER_EXTERNS_H_

#include <pspkerneltypes.h>

extern SceUID g_umd_sema;

int sceUmd_Init();
int march_init();
void pspUmdCallback(int a0);

#endif /* __ME_DRIVER_EXTERNS_H_ */