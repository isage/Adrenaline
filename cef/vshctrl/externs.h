#ifndef _EXTERNS_H
#define _EXTERNS_H

#include <pspctrl.h>

#include <systemctrl_se.h>

extern SEConfigEPI g_cfw_config;

extern int (* g_vshmenu_ctrl)(SceCtrlData *pad_data, int count);

extern u8 g_set;
extern int g_cpu_list[9];
extern int g_bus_list[9];
#define N_CPU (sizeof(g_cpu_list) / sizeof(int))

extern u32 g_firsttick;


#endif