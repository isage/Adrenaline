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

#include <psputility.h>

#include <cfwmacros.h>
#include <systemctrl.h>
#include <systemctrl_adrenaline.h>

#include "externs.h"

static int (* _sceUtilityLoadModule)(int id);
static int (* _sceUtilityUnloadModule)(int id);

static int (* _sceUtilityGetSystemParamInt)(int id, int *value);
static int (* _kermitUtilityOskGetStatus)();
static int (* _kermitUtilityOskInitStart)(SceUtilityOskParams *params);

////////////////////////////////////////////////////////////////////////////////
// PATCHED IMPLEMENTATIONS
////////////////////////////////////////////////////////////////////////////////

int sceUtilityLoadModulePatched(int id) {
	int res = _sceUtilityLoadModule(id);
	return (id != PSP_MODULE_NP_DRM) ? res : 0;
}

int sceUtilityUnloadModulePatched(int id) {
	int res = _sceUtilityUnloadModule(id);
	return (id != PSP_MODULE_NP_DRM) ? res : 0;
}

int kermitUtilityOskInitStartPatched(SceUtilityOskParams *params) {
	int k1 = pspSdkSetK1(0);

	if (params->data->language == PSP_UTILITY_OSK_LANGUAGE_DEFAULT) {
		_sceUtilityGetSystemParamInt(PSP_SYSTEMPARAM_ID_INT_LANGUAGE, &params->data->language);
		params->data->language++;
	}

	pspSdkSetK1(k1);
	return _kermitUtilityOskInitStart(params);
}

////////////////////////////////////////////////////////////////////////////////
// MODULE PATCHERS
////////////////////////////////////////////////////////////////////////////////

void PatchUtility() {
	HIJACK_FUNCTION(sctrlHENFindFunction("sceUtility_Driver", "sceUtility", 0x2A2B3DE0), sceUtilityLoadModulePatched, _sceUtilityLoadModule);
	HIJACK_FUNCTION(sctrlHENFindFunction("sceUtility_Driver", "sceUtility", 0xE49BFE92), sceUtilityUnloadModulePatched, _sceUtilityUnloadModule);

	if (g_config.osk_type == OSK_TYPE_VITA) {
		_sceUtilityGetSystemParamInt = (void *)sctrlHENFindFunction("sceUtility_Driver", "sceUtility", 0xA5DA2406);
		_kermitUtilityOskGetStatus = (void *)sctrlHENFindFunction("sceUtility_Driver", "sceUtility_private", 0xB08B2B48);

		HIJACK_FUNCTION(sctrlHENFindFunction("sceUtility_Driver", "sceUtility_private", 0x3B6D7CED), kermitUtilityOskInitStartPatched, _kermitUtilityOskInitStart);
	}

	sctrlFlushCache();
}