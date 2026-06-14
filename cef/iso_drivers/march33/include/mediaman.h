#ifndef __MEDIAMAN_H__
#define __MEDIAMAN_H__

#include <pspumd.h>

enum SceUmdMode {
	SCE_UMD_MODE_POWERON = 0x01,
	SCE_UMD_MODE_POWERCUR = 0x02,
};

typedef pspUmdInfo SceUmdDiscInfo;

int InitMediaMan();
void UmdNotifyCallback(int stat);

#endif // __MEDIAMAN_H__



