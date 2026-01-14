#include <string.h>

#include <cfwmacros.h>
#include <systemctrl.h>
#include <systemctrl_se.h>
#include <bootloadex.h>
#include <bootloadex_ark.h>

ARKConfig arkconf = {
    .magic = ARK_CONFIG_MAGIC,
    .arkpath = "ms0:/PSP/SAVEDATA/ARK_01234/", // default path for ARK files
    .exploit_id = "Adrenaline",
    .launcher = {0},
    .exec_mode = PSV_ADR, // run ARK in PSP mode
    .recovery = 0,
};

BootLoadExConfig bleconf = {
    .boot_type = TYPE_PAYLOADEX,
    .boot_storage = FLASH_BOOT,
    .extra_io = {
        .vita_io = {
            .redirect_flash = 0,
            .pspemuLfatOpenExtra = &pspemuLfatOpenArkVPSP
        }
    }
};


// Entry Point
int cfwBoot(int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7)
{
    #ifdef DEBUG
    _sw(0x44000000, 0xBC800100);
    colorDebug(0xFF00);
    #endif

    RebootexConfigADR *rebootex_config_adr = (RebootexConfigADR *)REBOOTEX_CONFIG;
    if (rebootex_config_adr->bootfileindex == MODE_RECOVERY) arkconf.recovery = 1;

    memcpy(ark_config, &arkconf, sizeof(ARKConfig));

    // Configure
    configureBoot(&bleconf);

    // scan for reboot functions
    findBootFunctions();
    
    // patch reboot buffer
    patchBootVita();
    
    // Forward Call
    return sceBoot(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
}
