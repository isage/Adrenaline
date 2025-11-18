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

#ifndef __MAIN_H__
#define __MAIN_H__

#include "vgraph.h"

//#define printf rDebugScreenPrintf

extern AdrenalineConfig g_cfw_config;
extern u32 select_color;
extern u8 g_theme;
extern u32 g_button_assign_value;

void MainMenu();
void ToggleUSB();
void Configuration();
void RunRecovery();
void ResetSettings();
void Advanced();
void AdvancedConfiguration();
void CpuSpeed();
void SetButtonAssign(int sel);
void SetWMA(int sel);
void SetFlashPlayer(int sel);
void Setrecovery_color(int sel);
void Exit();

#endif