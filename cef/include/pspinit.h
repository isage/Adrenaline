#ifndef __PSPINIT_H__
#define __PSPINIT_H__

enum PSPBootFrom {
	PSP_BOOT_FLASH = 0, /* ? */
	PSP_BOOT_DISC = 0x20,
	PSP_BOOT_USBWLAN = 0x30,
	PSP_BOOT_MS = 0x40,
	PSP_BOOT_EF = 0x50,
	PSP_BOOT_FLASH3 = 0x80,
};

enum PSPInitApitype {
	PSP_INIT_APITYPE_DISC = 0x120,
	PSP_INIT_APITYPE_DISC_UPDATER = 0x121,
	PSP_INIT_APITYPE_UMDEMU_MS1 = 0x123, /* np9660 game , x-reader */
	PSP_INIT_APITYPE_UMDEMU_MS2 = 0x124,
	PSP_INIT_APITYPE_UMDEMU_EF1 = 0x125,
	PSP_INIT_APITYPE_UMDEMU_EF2 = 0x126,
	PSP_INIT_APITYPE_USBWLAN = 0x130, /* Game Shareing */
	PSP_INIT_APITYPE_MS1 = 0x140,
	PSP_INIT_APITYPE_MS2 = 0x141,
	PSP_INIT_APITYPE_MS3 = 0x142,
	PSP_INIT_APITYPE_MS4 = 0x143, /* comic reader */
	PSP_INIT_APITYPE_MS5 = 0x144, /* pops */
	PSP_INIT_APITYPE_MS6 = 0x145,
	PSP_INIT_APITYPE_EF1 = 0x151,
	PSP_INIT_APITYPE_EF2 = 0x152,
	PSP_INIT_APITYPE_EF3 = 0x153,
	PSP_INIT_APITYPE_EF4 = 0x154,
	PSP_INIT_APITYPE_EF5 = 0x155,
	PSP_INIT_APITYPE_EF6 = 0x156,
	PSP_INIT_APITYPE_DISC_PBOOT = 0x160, /* Physical UMD with PBOOT */
	PSP_INIT_APITYPE_MLNAPP_MS = 0x170,
	PSP_INIT_APITYPE_MLNAPP_EF = 0x171,
	PSP_INIT_APITYPE_VSH1 = 0x210, /* ExitGame */
	PSP_INIT_APITYPE_VSH2 = 0x220, /* ExitVSH */
};

enum PSPKeyConfig {
	PSP_INIT_KEYCONFIG_VSH		= 0x100,
	PSP_INIT_KEYCONFIG_UPDATER	= 0x110,
	PSP_INIT_KEYCONFIG_GAME		= 0x200,
	PSP_INIT_KEYCONFIG_POPS		= 0x300,
	PSP_INIT_KEYCONFIG_APP		= 0x400,
};

/**
 * This structure represents an Init control block. It holds information about the
 * currently booted module by Init.
 */
typedef struct SceInit {
    /** The API type of the currently loaded module. One of ::SceInitApiType. */
    s32 apitype; //0
    /** The address of a memory protection block of type ::SCE_PROTECT_INFO_TYPE_FILE_NAME. */
    void *file_mod_addr; //4
    /** The address of a memory protection block of type ::SCE_PROTECT_INFO_TYPE_DISC_IMAGE. */
    void *disc_mod_addr; //8
    /** VSH parameters. Used to reboot the kernel. */
    SceKernelLoadExecVSHParam vsh_param; //12
    /** Unknown. */
    s32 unk60;
    /** Unknown. */
    s32 unk64;
    /** Unknown. */
    s32 unk68;
    /** Unknown. */
    s32 unk72;
    /** Unknown. */
    s32 unk76;
    /** Unknown. */
    s32 unk80;
    /** Unknown. */
    s32 unk84;
    /** Unknown. */
    s32 unk88;
    /** The application type of the currently loaded module. One of ::SceApplicationType. */
    u32 application_type; //92
    /** The number of power locks used by Init. */
    s32 num_power_locks; //96
    /** The address of a memory protection block of type ::SCE_PROTECT_INFO_TYPE_PARAM_SFO. */
    void *param_sfo_base; //100
    /** The size of of the memory block pointed to by ::paramSfoBase. */
    SceSize param_sfo_size; //104
    /** Unknown. */
    s32 lpt_summary; //108
    /** Pointer to boot callbacks of modules. */
    SceBootCallback *boot_callbacks1; //112
    /** The current boot callback 1 slot used to hold the registered boot callback. */
    SceBootCallback *cur_boot_callback1; //116
    /** Pointer to boot callbacks of modules. */
    SceBootCallback *boot_callbacks2; //120
    /** The current boot callback 2 slot used to hold the registered boot callback. */
    SceBootCallback *cur_boot_callback2; //124
} SceInit;

#ifdef __KERNEL__
/**
 * Gets the api type,
 *
 * @returns the api type in which the system has booted (one of `SceFileExecApiType`)
 *
 * @attention Needs to link to `lpspinit_kernel`.
*/
int sceKernelInitApitype();

/**
 * Gets the filename of the executable to be launched after all modules of the api.
 *
 * @returns filename of executable or NULL if no executable found.
 *
 * @attention Needs to link to `lpspinit_kernel`.
*/
char *sceKernelInitFileName();

/**
 *
 * Gets the device in which the application was launched.
 *
 * @returns the device code, one of `SceBootMediumType` values.
 *
 * @attention Needs to link to `lpspinit_kernel`.
*/
int sceKernelBootFrom();

/**
 * Get the key configuration in which the system has booted.
 *
 * @returns the key configuration code, one of `SceApplicationType` values
 *
 * @attention Needs to link to `lpspinit_kernel`.
*/
int sceKernelApplicationType();

#define sceKernelInitKeyConfig sceKernelApplicationType

/**
 * Retrieve Init's internal control block. This control block manages execution details of an
 * executable, like its API type, its boot medium and its application type.
 *
 * @return A pointer to Init's internal control block.
 */
SceInit *sceKernelQueryInitCB(void);

#endif // __KERNEL__

#endif // __PSPINIT_H__

