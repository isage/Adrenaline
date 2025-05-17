#ifndef PATCHED_IO
#define PATCHED_IO

#include <systemctrl.h>
#include <vshctrl.h>

#include "utils.h"
#include "isoreader.h"
#include "virtual_pbp.h"
#include "virtual_mp4.h"

SceUID sceIoDopenPatched(const char *dirname);
int sceIoDreadPatched(SceUID fd, SceIoDirent *dir);
int sceIoDclosePatched(SceUID fd);

SceUID sceIoOpenPatched(const char *file, int flags, SceMode mode);
int sceIoReadPatched(SceUID fd, void *data, SceSize size);
int sceIoClosePatched(SceUID fd);
SceOff sceIoLseekPatched(SceUID fd, SceOff offset, int whence);
int sceIoLseek32Patched(SceUID fd, int offset, int whence);
int sceIoGetstatPatched(const char *file, SceIoStat *stat);
int sceIoRemovePatched(const char *file);
int sceIoRmdirPatched(const char *path);
int sceIoMkdirPatched(const char *dir, SceMode mode);
int sceIoRenamePatched(const char *oldname, const char *name);
int sceIoChstatPatched(const char *file, SceIoStat *stat, int bits);

int homebrewloadexec(char * file, struct SceKernelLoadExecVSHParam * param);
int umdemuloadexec(char *file, struct SceKernelLoadExecVSHParam *param);
int umdLoadExec(char *file, struct SceKernelLoadExecVSHParam *param);
int umdLoadExecUpdater(char *file, struct SceKernelLoadExecVSHParam *param);

int LoadExecVSHCommonPatched(int apitype, char *file, struct SceKernelLoadExecVSHParam *param, int unk2);
int homebrewLoadExec(int apitype, char *file, struct SceKernelLoadExecVSHParam *param, int unk2);
int umdemuLoadExec(int apitype, char *file, struct SceKernelLoadExecVSHParam *param, int unk2);
int umdLoadExec2(int apitype, char *file, struct SceKernelLoadExecVSHParam *param, int unk2);

#endif