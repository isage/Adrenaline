#include <stdio.h>
#include <string.h>
#include <pspkernel.h>
#include <psputility.h>
#include <pspdisplay.h>
#include <psprtc.h>
#include <psppower.h>

#include <systemctrl_ark.h>
#include <cfwmacros.h>
#include <vshctrl.h>
#include <kubridge.h>
#include <systemctrl.h>

#include "xmbctrl.h"
#include "utils.h"

#include "../../adrenaline_version.h"

static u32 g_patch_addr=0;
static char g_info_string[128] = {0};
static int g_menu_mode = 0;
static u32 g_button_on = 0;
static u32 g_cur_buttons = 0xFFFFFFFF;

int g_vshmenu_running  = 0;
void (*g_vshmenu_draw)(void* frame) = NULL;

int (*scePafAddClockOrig)(ScePspDateTime*, wchar_t*, int, wchar_t*) = NULL;

int scePafAddClockPatched(ScePspDateTime* time, wchar_t* str, int max_len, wchar_t* format) {
    if (g_vshmenu_running){
        return utf8_to_unicode(str, g_info_string);
    }
    else {
        return scePafAddClockOrig(time, str, max_len, format);
    }
}

void patchVshClock(u32 addr) {
    u32 fw = sceKernelDevkitVersion();
    u32 major = fw>>24;
    u32 minor = (fw>>16)&0xF;
    u32 micro = (fw>>8)&0xF;

	#ifdef NIGHTLY
	paf_snprintf(g_info_string, 127, "\n\n\n"
        "CFW: Epinephrine v" ADRENALINE_VERSION_MAJOR_STR "." ADRENALINE_VERSION_MINOR_STR "." ADRENALINE_VERSION_MICRO_STR "-"EPI_NIGHTLY_VER"\n"
        "Built: %s-%s-%s " __TIME__ "\n"
        "Console: PS Vita (01g) FW%d%d%d\n"
        "Bootloader: Adrenaline v" ADRENALINE_VERSION_MAJOR_STR,
		DAY_STR, MONTH_STR, YEAR_STR,
        major, minor, micro
    );
	#else
    paf_snprintf(g_info_string, 127, "\n\n\n"
        "CFW: Epinephrine v" ADRENALINE_VERSION_MAJOR_STR "." ADRENALINE_VERSION_MINOR_STR "." ADRENALINE_VERSION_MICRO_STR "\n"
        "Built: %c%c-%s-%s " __TIME__ "\n"
        "Console: PS Vita (01g) FW%d%d%d\n"
        "Bootloader: Adrenaline v" ADRENALINE_VERSION_MAJOR_STR,
        DAY_CHAR1, DAY_CHAR2, MONTH_STR, YEAR_STR,
        major, minor, micro
    );
	#endif

	VWRITE16(addr - 0x48, 0);
    g_patch_addr = addr;

    sceKernelDcacheWritebackAll();
    kuKernelIcacheInvalidateAll();
}

static int EatKey(SceCtrlData *pad_data, int count) {
    g_button_on   = ~g_cur_buttons & pad_data[0].Buttons;
    g_cur_buttons = pad_data[0].Buttons;

    // menu control
    switch (g_menu_mode) {
        case 0:
            if ((g_cur_buttons & ALL_CTRL) == 0)
                g_menu_mode = 1;
            break;
        case 1:
            if ((g_button_on & PSP_CTRL_SELECT) || (g_button_on & PSP_CTRL_HOME))
				g_menu_mode = 2;
            break;
		case 2:
			if ((g_cur_buttons & ALL_CTRL) == 0)
				vctrlVSHExitVSHMenu(NULL, NULL, 0);
			break;
    }

    // mask buttons for LOCK VSH control
    for (int i = 0; i < count; i++) {
        pad_data[i].Buttons = 0;
    }

    return 0;
}

int xmbctrlEnterVshMenuMode() {
    g_menu_mode = 0;
    g_button_on = 0;
    g_cur_buttons = 0xFFFFFFFF;
    g_vshmenu_draw = NULL;

    vctrlVSHRegisterVshMenu(EatKey);

    if (scePafAddClockOrig == NULL){
        scePafAddClockOrig = (void*)U_EXTRACT_CALL(g_patch_addr + 4);
        MAKE_CALL(g_patch_addr + 4, (u32)&scePafAddClockPatched);
        sctrlFlushCache();
    }

    g_vshmenu_running = 1;

	return 0;
}

int xmbctrlExitVshMenuMode() {
    g_vshmenu_running = 0;

	return 0;
}

int xmbctrlRegisterVshMenu(void (*draw_func)(void*)) {
    g_vshmenu_draw = draw_func;

	return 0;
}
