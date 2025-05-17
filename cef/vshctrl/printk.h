#ifndef __PRINTK_H__
#define __PRINTK_H__

// FIXME: This probably should be exported by systemctrl

int printk(char *fmt, ...);
int printkInit(const char *output);
int printkSync(void);

#endif