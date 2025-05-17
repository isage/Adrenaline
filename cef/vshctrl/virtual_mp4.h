#ifndef VIRTUAL_MP4
#define VIRTUAL_MP4

#include <systemctrl.h>

SceUID videoIoDopen(const char *dir);
int videoIoDread(SceUID fd, SceIoDirent *dir);
int videoIoDclose(SceUID fd);

SceUID videoIoOpen(const char *file, u32 flags, u32 mode);
int videoIoClose(SceUID fd);
int videoIoGetstat(const char *path, SceIoStat *stat);
int videoIoRead(SceUID fd, void *buf, u32 size);
SceOff videoIoLseek(SceUID fd, SceOff offset, int whence);
int videoIoRemove(const char *file);

int is_video_path(const char *path);
int is_video_file(SceUID fd);
int is_video_folder(SceUID dd);
#endif