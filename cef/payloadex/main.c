#include <string.h>

#include <cfwmacros.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <bootloadex.h>
#include <pspbtcnf.h>


RebootexConfigADR *rebootex_config = (RebootexConfigADR *)REBOOTEX_CONFIG;

int pspemuLfatOpenAdrenaline(BootFile* file);

BootLoadExConfig bleconf = {
    .boot_type = TYPE_PAYLOADEX,
    .boot_storage = FLASH_BOOT,
    .extra_io.vita_io = {
        .redirect_flash = 0,
        .pspemuLfatOpenExtra = &pspemuLfatOpenAdrenaline,
    }
};


int pspemuLfatOpenAdrenaline(BootFile* file){
    
    if (strcmp(file->name, "pspbtcnf.bin") == 0){
        switch(rebootex_config->bootfileindex) {
			case MODE_UMD:
				file->name = "/kd/pspbtjnf.bin";
				break;

			case MODE_INFERNO:
				file->name = "/kd/pspbtknf.bin";
				break;

			case MODE_MARCH33:
				file->name = "/kd/pspbtlnf.bin";
				break;

			case MODE_NP9660:
				file->name = "/kd/pspbtmnf.bin";
				break;

			case MODE_RECOVERY:
				file->name = "/kd/pspbtrnf.bin";
				break;
		}

		if (rebootex_config->bootfileindex == MODE_RECOVERY) {
			rebootex_config->bootfileindex = MODE_UMD;
		}
    }

    return -1; // pass through
}


// Entry Point
int cfwBoot(int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7)
{
    // Configure
    configureBoot(&bleconf);

    // scan functions
    findBootFunctions();
    
    // patch sceboot
    patchBootVita();
    
    // Forward Call
    return sceBoot(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
}
