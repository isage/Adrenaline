#ifndef __XMBCTRL_UTILS_H__
#define __XMBCTRL_UTILS_H__

int utf8_to_unicode(wchar_t *dest, char *src);
int ReadLine(SceUID fd, char *str);
void trim(char *str);
int GetPlugin(char *buf, int size, char *str, int *activated);

#endif