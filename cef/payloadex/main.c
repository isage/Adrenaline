#include <string.h>

#include <cfwmacros.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <bootloadex.h>
#include <bootloadex_ark.h>

RebootexConfigEPI *rebootex_config = (RebootexConfigEPI *)REBOOTEX_CONFIG;


int pspemuLfatOpenEPI(BootFile* file){

    int ret = -1;
    char* p = file->name;

    if (strcmp(p, "pspbtcnf.bin") == 0){
		if (rebootex_config->bootfileindex == MODE_RECOVERY) {
			rebootex_config->bootfileindex = MODE_UMD;
            file->name = "/kd/pspbtrnf.bin";
		}
        else {
            file->name = "/kd/pspbtjnf.bin";
        }
    }

    return ret;
}


BootLoadExConfig bleconf = {
    .boot_type = TYPE_PAYLOADEX,
    .boot_storage = FLASH_BOOT,
    .extra_io.vita_io = {
        .redirect_flash = 0,
        .pspemuLfatOpenExtra = &pspemuLfatOpenEPI
    }
};


// Entry Point
int cfwBoot(int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7)
{
    // Configure
    configureBoot(&bleconf);

    // scan for reboot functions
    findBootFunctions();

    // patch reboot buffer
    patchBootVita();

    // Forward Call
    return sceBoot(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
}
