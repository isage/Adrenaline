#include <pspkerneltypes.h>
#include <pspiofilemgr.h>

#include <systemctrl.h>
#include <adrenaline_log.h>
#include <cfwmacros.h>

#include "xmbctrl.h"

static u32 RefDat_2e91c = 0;
static u32 RefDat_2f2a4 = 0;
static u32 RefDat_2f2b4 = 0;
static char * (*ReturnBasePath)(int unk) = NULL;
// drive_mode: 1 - `ms0:` 2 - `ef0:`
static int (*GetDriveInfo)(char *unk1, u32 *unk2, int drive_mode, int *unk3, u32 unk4) = NULL;

int g_game_plugin = 0;
int g_savedata_plugin = 0;

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

char *ReturnBasePathPatched(int unk) {
	if (g_is_ef_drive) {
		VWRITE32(RefDat_2e91c, 2);
	} else {
		VWRITE32(RefDat_2e91c, 1);
	}

	char* ret = ReturnBasePath(unk);
	logmsg3("[DEBUG]: %s: unk=%d -> %s\n", __func__, unk, ret);
	return ret;
}

int GetDriveInfoPatched(char *unk1, u32 *unk2, int drive_mode, int *unk3, u32 unk4) {
	logmsg4("[DEBUG]: %s: og drive_mode:%d\n", __func__, drive_mode);
	if (g_is_ef_drive) {
		drive_mode = 2;
	}
	logmsg4("[DEBUG]: %s: using drive_mode:%d\n", __func__, drive_mode);

	int ret = GetDriveInfo(unk1, unk2, drive_mode, unk3, unk4);

	logmsg3("[DEBUG]: %s: unk1=%s, unk2=%ld, drive_mode=%d, unk4=%ld -> 0x%08X\n", __func__, unk1, *unk2, drive_mode, unk4, ret);
	return ret;
}

int sce_paf_private_snprintf_patched(char *a0, int a1, const char *a2, void *a3, void *t0) {
    sce_paf_private_strcpy((char *)a1, (char *)t0);
    return sce_paf_private_snprintf(a0, 291, a2, a3, t0);
}

////////////////////////////////////////////////////////////////////////////////
// MODULE PATCHERS
////////////////////////////////////////////////////////////////////////////////

void PatchGamePlugin(SceModule *mod) {
	u32 text_addr = mod->text_addr;
	u32 text_size = mod->text_size;
	g_game_plugin = 1;

	ReturnBasePath = (void *)(text_addr+0x21584);
	GetDriveInfo = (void *)(text_addr+0x21fb4);
	RefDat_2e91c = text_addr+0x2e91c;
	RefDat_2f2a4 = text_addr+0x2f2a4;
	RefDat_2f2b4 = text_addr+0x2f2b4;

	int patches = 20;
	for (u32 addr = text_addr; addr < text_addr + text_size && patches; addr += 4) {
		u32 data = VREAD32(addr);

		if (data == JAL(ReturnBasePath)) {
			MAKE_CALL(addr, ReturnBasePathPatched);
			patches--;
		}
	}

	MAKE_CALL(text_addr+0x5d58, GetDriveInfoPatched);

	MAKE_CALL(text_addr+0x1E6A8, sce_paf_private_snprintf_patched);
	MAKE_INSTRUCTION(text_addr+0x1E69C, 0x02402821);

	MAKE_CALL(text_addr+0x21340, sce_paf_private_snprintf_patched);
	MAKE_INSTRUCTION(text_addr+0x21304, 0x02402821);

}