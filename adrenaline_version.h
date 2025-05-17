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

#ifndef __ADRENALINE_VERSION_H__
#define __ADRENALINE_VERSION_H__

#define ADRENALINE_VERSION_MAJOR    7
#define ADRENALINE_VERSION_MINOR    1
#define ADRENALINE_VERSION_MICRO    7
#define ADRENALINE_VERSION_REV		0
#define ADRENALINE_VERSION ((ADRENALINE_VERSION_MAJOR << 24) | (ADRENALINE_VERSION_MINOR << 16) | (ADRENALINE_VERSION_MICRO << 8))

#define xstr(s) #s
#define str(s) xstr(s)
#define ADRENALINE_VERSION_MAJOR_STR str(ADRENALINE_VERSION_MAJOR)
#define ADRENALINE_VERSION_MINOR_STR str(ADRENALINE_VERSION_MINOR)
#define ADRENALINE_VERSION_MICRO_STR str(ADRENALINE_VERSION_MICRO)

#endif
