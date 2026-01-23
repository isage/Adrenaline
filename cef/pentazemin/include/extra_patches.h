#ifndef EXTRA_PATCHES_H
#define EXTRA_PATCHES_H

#include <pspsdk.h>

extern void PatchUSBCamDriver(SceModule*);
extern void PatchVlfLib(SceModule*);
extern void PatchImposeDriver(SceModule*);
extern void PatchUtility();
extern void PatchPowerService(SceModule*);
extern void PatchLoadExec(SceModule*);

#endif