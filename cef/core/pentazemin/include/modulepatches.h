#ifndef _PENTAZEMIN_MODPATCHES_H_
#define _PENTAZEMIN_MODPATCHES_H_

#include <pspsdk.h>
#include <psploadcore.h>

////////////////////////////////////////////////////////////////////////////////
// System Module Patchers
////////////////////////////////////////////////////////////////////////////////

void PatchMemlmd(void);
void PatchSysmem(void);
void PatchUtility(void);
void PatchIoFileMgr(void);
void PatchLoaderCore(void);
void PatchVolatileMemBug(void);

void PatchPops(SceModule* mod);
void PatchChkreg(SceModule* mod);
void PatchMesgLed(SceModule* mod);
void PatchPopsMgr(SceModule* mod);
void PatchSasCore(SceModule* mod);
void PatchLoadExec(SceModule* mod);
void PatchUmdDriver(SceModule* mod);
void PatchUSBCamDriver(SceModule* mod);
void PatchImposeDriver(SceModule* mod);
void PatchLowIODriver2(SceModule* mod);
void PatchPowerService(SceModule* mod);
void PatchPowerService2(SceModule* mod);
void PatchSysconfPlugin(SceModule* mod);
void PatchMeCodecWrapper(SceModule* mod);

int AdrenalinePatchInit(int (* module_bootstart)(SceSize, void *), void *argp);

////////////////////////////////////////////////////////////////////////////////
// Other Module Patchers
////////////////////////////////////////////////////////////////////////////////

void PatchMemUnlock(void);
void PatchGameBoot(SceModule* mod);

void PatchVlfLib(SceModule* mod);
void PatchCwCheatPlugin(SceModule* mod);

#endif /* _PENTAZEMIN_MODPATCHES_H_ */