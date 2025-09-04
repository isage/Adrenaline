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

#ifndef __MENU_H__
#define __MENU_H__

typedef struct {
	char *name;
	void (* function)(int sel);
	char **options;
	int size_options;
	u8 *value;
} Entry;

typedef struct {
  u8 accent_color; // arrows, first letters
  u8 text_color; // unselected text in main area
  u8 main_bg; // main area bg

  u8 select_text_color; // selected menu item
  u8 select_text_bg; // selected menu item bg

  u8 panel_text; // top/bottom panels
  u8 panel_bg; // top/bottom panels
  u8 panel_select_text; // top panel
  u8 panel_select_bg; // top panel
  u8 dialog_text; // message dialog
  u8 dialog_bg; // message dialog

  u8 default_value_color;
  u8 changed_value_color;
} Theme;

void MenuLoop();
void UpdatePluginCount(int count);
void ShowDialog(char* message);

#endif