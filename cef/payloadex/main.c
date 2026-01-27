#include <string.h>

#include <cfwmacros.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <bootloadex.h>
#include <bootloadex_ark.h>

RebootexConfigEPI *g_rebootex_config = (RebootexConfigEPI *)REBOOTEX_CONFIG;


int pspemuLfatOpenEPI(BootFile* file) {
    int ret = -1;
    char* p = file->name;

    if (strcmp(p, "pspbtcnf.bin") == 0){
		if (g_rebootex_config->bootfileindex == MODE_RECOVERY) {
			g_rebootex_config->bootfileindex = MODE_UMD;
            file->name = "/kd/pspbtrnf.bin";
		}
        else {
            file->name = "/kd/pspbtjnf.bin";
        }
    }

    return ret;
}


BootLoadExConfig g_bleconf = {
    .boot_type = TYPE_PAYLOADEX,
    .boot_storage = FLASH_BOOT,
    .extra_io.vita_io = {
        .redirect_flash = 0,
        .pspemuLfatOpenExtra = &pspemuLfatOpenEPI
    }
};


// Entry Point
int cfwBoot(int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7) {
	g_bleconf.rtm_mod.before = g_rebootex_config->module_after;
	g_bleconf.rtm_mod.buffer = g_rebootex_config->buf;
	g_bleconf.rtm_mod.size = g_rebootex_config->size;
	g_bleconf.rtm_mod.flags = g_rebootex_config->flags;

    // Configure
    configureBoot(&g_bleconf);

    // scan for reboot functions
    findBootFunctions();

    // patch reboot buffer
    patchBootVita();

    // Forward Call
    return sceBoot(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
}
