#ifndef _EXTERNS_H
#define _EXTERNS_H

#include <pspctrl.h>

extern int (* vshmenu_ctrl)(SceCtrlData *pad_data, int count);

extern u8 set;
extern int cpu_list[9];
extern int bus_list[9];
#define N_CPU (sizeof(cpu_list) / sizeof(int))

extern u32 firsttick;


#endif