#ifndef REGISTRY_H
#define REGISTRY_H

#include <pspsdk.h>
#include <psptypes.h>
#include <pspreg.h>
#include <string.h>

int vctrlGetRegistryValue(const char *dir, const char *name, u32 *val);
int vctrlSetRegistryValue(const char *dir, const char *name, u32 val);

#endif