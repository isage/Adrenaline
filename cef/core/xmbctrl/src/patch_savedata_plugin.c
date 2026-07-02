#include <pspkerneltypes.h>
#include <pspiofilemgr.h>

#include <systemctrl.h>
#include <adrenaline_log.h>
#include <cfwmacros.h>

#include "xmbctrl.h"

static u32 RefDat_is_ef = 0;

static int (*checkOrGetSavedataPath)(void * unk1, int is_ef, u8 unk2) = NULL;

////////////////////////////////////////////////////////////////////////////////
// HELPERS
////////////////////////////////////////////////////////////////////////////////

// static int is_ef_path(const char *path) {
// 	if ((path[0] == 'e' || path[0] == 'E') && (path[1] == 'f' || path[1] == 'F')) {
// 		return 1;
// 	}
// 	return 0;
// }


////////////////////////////////////////////////////////////////////////////////
// PATCHED IMPLEMENTATIONS
////////////////////////////////////////////////////////////////////////////////

int checkOrGetSavedataPathPatched(void * unk1, int is_ef, u8 unk2) {
	if (g_is_ef_drive) {
		is_ef = 1;
	}

	int ret = checkOrGetSavedataPath(unk1, is_ef, unk2);

	logmsg3("[DEBUG]: %s: unk1=%p, is_ef=%d, unk2=%d -> 0x%08X\n", __func__, unk1, is_ef, unk2, ret);
	return ret;
}

////////////////////////////////////////////////////////////////////////////////
// MODULE PATCHERS
////////////////////////////////////////////////////////////////////////////////

void PatchSavedataPlugin(SceModule *mod) {
	u32 text_addr = mod->text_addr;
	u32 text_size = mod->text_size;
	g_savedata_plugin = 1;

	checkOrGetSavedataPath = (void *)(text_addr+0x8f88);
	RefDat_is_ef = text_addr+0x22db4;

	int patches = 2;
	for (u32 addr = text_addr; addr < text_addr + text_size && patches; addr += 4) {
		u32 data = VREAD32(addr);

		if (data == JAL(checkOrGetSavedataPath)) {
			MAKE_CALL(addr, checkOrGetSavedataPathPatched);
			patches--;
		}
	}

}