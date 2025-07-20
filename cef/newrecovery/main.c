/*
	Adrenaline
	Copyright (C) 2016-2018, TheFloW

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <common.h>

#include "main.h"
#include "menu.h"
#include "installer.h"
#include "plugins.h"
#include "utils.h"

#include "options.h"

PSP_MODULE_INFO("Recovery mode", 0, 1, 0);
PSP_DISABLE_NEWLIB();
PSP_DISABLE_AUTOSTART_PTHREAD();

static int usbStatus = 0;

int recovery_exit = 0;

u32 button_assign_value = 0;

AdrenalineConfig config;

void ToggleUSB()
{
  if (!usbStatus)
  {
    sctrlStartUsb();
    usbStatus = 1;
    ShowDialog("USB enabled");
  }
  else
  {
    sctrlStopUsb();
    usbStatus = 0;
    ShowDialog("USB disabled");
  }
}

void RunRecovery()
{
  sctrlStopUsb();

  sctrlSESetConfig(&config);

  static u32 vshmain_args[0x100];
  memset(vshmain_args, 0, sizeof(vshmain_args));

  vshmain_args[0] = sizeof(vshmain_args);
  vshmain_args[1] = 0x20;
  vshmain_args[16] = 1;

  struct SceKernelLoadExecVSHParam param;

  memset(&param, 0, sizeof(param));
  param.size = sizeof(param);
  param.argp = "ms0:/PSP/GAME/RECOVERY/EBOOT.PBP";
  param.args = strlen(param.argp) + 1;
  param.vshmain_args = vshmain_args;
  param.vshmain_args_size = sizeof(vshmain_args);
  param.key = "game";

  sctrlKernelLoadExecVSHMs2(param.argp, &param);
}

void RegistryHacks()
{
  GetRegistryData("/CONFIG/SYSTEM/XMB", "button_assign", REG_TYPE_INT, &button_assign_value, sizeof(u32));
}

void SetButtonAssign(int sel)
{
  SetRegistryData("/CONFIG/SYSTEM/XMB", "button_assign", REG_TYPE_INT, (void *)&button_assign_value, sizeof(u32));
}

void SetWMA(int sel)
{
  u32 value = 0;
  GetRegistryData("/CONFIG/MUSIC", "wma_play", REG_TYPE_INT, &value, sizeof(u32));

  if (value == 1)
  {
    ShowDialog("WMA Activated");
  }
  else
  {
    value = 1;
    SetRegistryData("/CONFIG/MUSIC", "wma_play", REG_TYPE_INT, (void *)&value, sizeof(u32));
    ShowDialog("WMA Activated");
  }
}

void SetFlashPlayer(int sel)
{
  u32 value = 0;
  GetRegistryData("/CONFIG/BROWSER", "flash_activated", REG_TYPE_INT, &value, sizeof(u32));

  if (value == 1)
  {
    ShowDialog("Flash Activated");
  }
  else
  {
    value = 1;
    SetRegistryData("/CONFIG/BROWSER", "flash_activated", REG_TYPE_INT, (void *)&value, sizeof(u32));
    SetRegistryData("/CONFIG/BROWSER", "flash_play", REG_TYPE_INT, (void *)&value, sizeof(u32));
    ShowDialog("Flash Activated");
  }
}

void SetRecoveryColor(int sel)
{
  theme = config.recoverycolor;
}

void Exit()
{
  recovery_exit = 1;
}

int main(int argc, char *argv[])
{

  SceIoStat stat;
  memset(&stat, 0, sizeof(SceIoStat));
  if (sceIoGetstat("ms0:/__ADRENALINE__/flash0", &stat) < 0)
  {
    VGraphInit(0);
    Installer();
  }

  VGraphInit(1);
  sctrlSEGetConfig(&config);
  SetRecoveryColor(config.recoverycolor);

  RegistryHacks();
  UpdatePluginCount(Plugins());

  while (!recovery_exit)
  {
    MenuLoop();
  }

  sctrlStopUsb();

  sctrlSESetConfig(&config);

  sctrlKernelExitVSH(NULL);

  return 0;
}
