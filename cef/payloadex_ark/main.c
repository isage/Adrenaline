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


int pspemuLfatOpenArkVPSP(BootFile* file){

    int ret = -1;
    char* p = file->name;

    if (strcmp(p, "pspbtcnf.bin") == 0){
        p[2] = 'v'; // custom btcnf for PS Vita
        p[5] = 'k'; // use np9660 ISO mode (psvbtknf.bin)
        ret = findArkFlashFile(file, p);
        if (ret == 0){
            relocateFlashFile(file);
        }
    }
    else if (strncmp(p, "/kd/ark_", 8) == 0){ // ARK module
        ret = findArkFlashFile(file, p);
        if (ret == 0){
            relocateFlashFile(file);
        }
    }

    return ret;
}


BootLoadExConfig g_bleconf = {
    .boot_type = TYPE_PAYLOADEX,
    .boot_storage = FLASH_BOOT,
    .extra_io.vita_io = {
        .redirect_flash = 0,
        .pspemuLfatOpenExtra = &pspemuLfatOpenArkVPSP
    }
};


// Entry Point
int cfwBoot(int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7) {
    RebootexConfigEPI *rebootex_config_adr = (RebootexConfigEPI *)REBOOTEX_CONFIG;
    if (rebootex_config_adr->bootfileindex == MODE_RECOVERY) arkconf.recovery = 1;

    memcpy(ark_config, &arkconf, sizeof(ARKConfig));


    // Configure
    configureBoot(&g_bleconf);

    // scan for reboot functions
    findBootFunctions();

    // patch reboot buffer
    patchBootVita();

    // Forward Call
    return sceBoot(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
}
