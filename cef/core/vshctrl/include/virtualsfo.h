/*
	Adrenaline
	Copyright (C) 2016-2018, TheFloW
	Copyright (C) 2024-2025, isage
	Copyright (C) 2025, GrayJack

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

#ifndef __VIRTUALSFO__
#define __VIRTUALSFO__

void virtual_sfo_init();
int virtual_sfo_size();
unsigned char* virtual_sfo_get();
void sfo_set_int_param(char *key, int value);
void sfo_set_string_param(char *key, char* value);
int sfo_get_int_param(char *key);
char* sfo_get_string_param(char *key);

#endif
