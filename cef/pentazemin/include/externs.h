#ifndef __PENTAZEMIN_EXTERNS_H__
#define __PENTAZEMIN_EXTERNS_H__

#include <systemctrl.h>
#include <systemctrl_se.h>
#include <systemctrl_adrenaline.h>

extern STMOD_HANDLER g_module_handler;
extern SceAdrenaline *g_adrenaline;

extern PentazeminConfig g_config;

extern int (*_sctrlHENApplyMemory)(u32);
int memoryHandlerVita(u32 p2);

#endif