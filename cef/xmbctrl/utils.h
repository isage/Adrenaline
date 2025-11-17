#ifndef __XMBCTRL_UTILS_H__
#define __XMBCTRL_UTILS_H__

#include <stddef.h>

#include <pspkerneltypes.h>

int utf8_to_unicode(wchar_t *dest, char *src);
int ReadLine(SceUID fd, char *str);
int GetPlugin(char *buf, int size, char *str, int *activated);

#endif