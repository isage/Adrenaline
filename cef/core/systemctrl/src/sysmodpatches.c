/*
    Adrenaline
    Copyright (C) 2016-2018, TheFloW
    Copyright (C) 2025, GrayJack

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

/**
 * Module patches for system modules
 */

#include <string.h>

#include <pspctrl.h>
#include <psperror.h>
#include <pspextratypes.h>
#include <pspsysmem_kernel.h>
#include <pspumd.h>

#include <bootloadex.h>
#include <systemctrl.h>
#include <systemctrl_epi.h>
#include <systemctrl_se.h>

#include <adrenaline_log.h>

#include "externs.h"
#include <systemctrl_epi.h>

#include "plugin.h"
#include "rebootex.h"

#define EXIT_TO_VSH_MASK (PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER | PSP_CTRL_SELECT | PSP_CTRL_DOWN)
#define EXIT_TO_VSH2_MASK (PSP_CTRL_LTRIGGER | PSP_CTRL_RTRIGGER | PSP_CTRL_START | PSP_CTRL_DOWN)

#define EXTRA_RAM_SETTABLE (0)

STMOD_HANDLER g_module_handler = NULL;
SYSBOOT_HANDLER g_on_system_booted_handler = NULL;

static int (*RunReboot)(u32 *params) = NULL;
static int (*DecodeKL4E)(void *dest, u32 size_dest, void *src, u32 size_src) = NULL;

static int (*_sceChkregGetPsCode)(u8 *pscode) = NULL;
static int (*_sceSystemFileGetIndex)(void *sfo, void *a1, void *a2) = NULL;
static int (*_sceCtrlPeekBufferPositive)(SceCtrlData *pad_data, int count) = NULL;
static int (*_sceCtrlPeekBufferNegative)(SceCtrlData *pad_data, int count) = NULL;
static int (*_sceCtrlReadBufferPositive)(SceCtrlData *pad_data, int count) = NULL;
static int (*_sceCtrlReadBufferNegative)(SceCtrlData *pad_data, int count) = NULL;
static int (*_scePowerSetClockFrequency_k)(int cpufreq, int ramfreq, int busfreq) = NULL;
static int (*_scePowerGetClockFrequency_k)() = NULL;

static int (*_memlmd_StartScrambleKey)(u32 unk, void *hash_addr) = NULL;
static int (*_memlmd_unsigner)(u8 *prx, u32 size, int use_polling) = NULL;
static KDEC_HANDLER _memlmd_decrypt = NULL;
static KDEC_HANDLER g_extend_memlmd_decryption = NULL;

static MDEC_HANDLER _mesgled_decrypt = NULL;
static MDEC_HANDLER g_extend_mesgled_decrypt = NULL;

// Known GZIP Compression Tags
static u32 g_gzip_compress_tag[] = {
    0x28796DAA, 0x7316308C, 0x3EAD0AEE, 0x8555ABF2, 0xC6BA41D3, 0x55668D96, 0xC01DB15D,
};

////////////////////////////////////////////////////////////////////////////////
// HELPERS
////////////////////////////////////////////////////////////////////////////////

static u32 FindPowerFunction(u32 nid) {
    return sctrlHENFindFunction("scePower_Service", "scePower", nid);
}

static int protect_pspemu_mem() {
    u32 ram2 = g_rebootex_config.ram2;
    u32 ram11 = g_rebootex_config.ram11;

    // Needs to protect FLASH0 VRAM
    if (ram2 + ram11 >= 49) {
        int partition = (ram2 >= 49) ? PSP_MEMORY_PARTITION_USER : 11;
        int res = sceKernelAllocPartitionMemory(
            partition, "SCE_PSPEMU_FLASH0", PSP_SMEM_Addr, 1024 * 1024,
            (void *)SCE_PSPEMU_FLASH0_RAMFS
        );

        if (res < 0) {
            return res;
        }
    }

    return 0;
}

int ApplyMemory() {
    if (g_rebootex_config.ram2 != EXTRA_RAM_SETTABLE
        && (g_rebootex_config.ram2 + g_rebootex_config.ram11) <= 52) {
        PspSysMemPartition *partition;
        u32 user_size;

        user_size = (g_rebootex_config.ram2 * 1024 * 1024);
        partition = sctrlGetMemoryPartition(PSP_MEMORY_PARTITION_USER);
        partition->size = user_size;
        partition->data->size = (((user_size >> 8) << 9) | 0xFC);

        partition = sctrlGetMemoryPartition(11);
        partition->size = (g_rebootex_config.ram11 * 1024 * 1024);
        partition->address = 0x88800000 + user_size;
        partition->data->size = (((partition->size >> 8) << 9) | 0xFC);

        // Protects the regions with pspemu addresses
        protect_pspemu_mem();
    }

    return 0;
}

void ApplyAndResetMemory() {
    ApplyMemory();
    g_rebootex_config.ram2 = EXTRA_RAM_SETTABLE;
}

void UnprotectExtraMemory() {
    u32 *prot = (u32 *)0xBC000040;

    // Allow user code to read/write to extra ram
    for (int i = 0; i < 0x10; i++) {
        prot[i] = 0xFFFFFFFF;
    }

    // Allow user to allocate on extra ram
    for (int i = 8; i < 12; i++) {
        PspSysMemPartition *partition = (void *)sctrlGetMemoryPartition(i);
        if (partition) {
            partition->address &= 0x7FFFFFFF;
        }
    }
}

static int exitToVsh(SceSize args, void *argp) {
    int k1 = pspSdkSetK1(0);

    // Refuse operation in Save dialog
    if (sceKernelFindModuleByName("sceVshSDUtility_Module") != NULL) {
        return 0;
    }

    // Refuse operation in Dialog
    if (sceKernelFindModuleByName("sceDialogmain_Module") != NULL) {
        return 0;
    }

    int (*_sceDisplaySetHoldMode)(int) =
        (void *)sctrlHENFindFunction("sceDisplay_Service", "sceDisplay", 0x7ED59BC4);
    if (_sceDisplaySetHoldMode) {
        _sceDisplaySetHoldMode(0);
    }

    // reset some flags
    SetUmdFile("");
    sctrlSESetBootConfFileIndex(MODE_UMD);

    int res = sctrlKernelExitVSH(NULL);

    pspSdkSetK1(k1);
    return res;
}

static void startExitThread() {
    int k1 = pspSdkSetK1(0);
    int intc = pspSdkDisableInterrupts();
    if (sctrlGetThreadUIDByName("ExitGamePollThread") >= 0) {
        pspSdkEnableInterrupts(intc);
        return; // already exiting
    }
    int uid = sceKernelCreateThread("ExitGamePollThread", exitToVsh, 1, 4096, 0, NULL);
    pspSdkEnableInterrupts(intc);
    sceKernelStartThread(uid, 0, NULL);
    sceKernelWaitThreadEnd(uid, NULL);
    sceKernelDeleteThread(uid);
    pspSdkSetK1(k1);
}

/** Checks controller input and modifies CFW context when certain buttons are hold at the start of
 * an application */
void CheckControllerInput() {
    SceCtrlData pad_data;
    _sceCtrlPeekBufferPositive(&pad_data, 1);
    if ((pad_data.Buttons & PSP_CTRL_LTRIGGER) == PSP_CTRL_LTRIGGER) {
        g_disable_plugins = 1;
        logmsg("[INFO]: Plugins disabled by holding `L` at the application start\n");
    }
}

static int is_tag_compressed(u32 tag) {
    for (int i = 0; i < NELEMS(g_gzip_compress_tag); i++) {
        if (g_gzip_compress_tag[i] == tag) {
            return 1;
        }
    }
    return 0;
}

static int is_prx_compressed(u8 *prx, u32 size) {
    // Minimum size check
    if (size < 0x160) {
        return 0;
    }

    // Check GZIP magic and with supported compression tag
    if ((*(u16 *)(prx + 0x150) == 0x8B1F) && is_tag_compressed(*(u32 *)(prx + 0x130))) {
        // GZIP PRX
        return 1;
    }

    // Encrypted PRX
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
// CFW API (That depends on this file globals/statics)
////////////////////////////////////////////////////////////////////////////////

int sctrlHENSetMemory(u32 p2, u32 p11) {
    if ((p2 == 0) || ((p2 + p11) > 52)) {
        return SCE_ERR_INMODE;
    }

    // Disallow setting after game boot
    if (sctrlHENIsSystemBooted()) {
        return -3;
    };

    // Do not allow in pops and vsh
    int apitype = sceKernelInitApitype();
    if (apitype == PSP_INIT_APITYPE_MS5 || apitype == PSP_INIT_APITYPE_EF5
        || apitype >= PSP_INIT_APITYPE_VSH_KERNEL) {
        return -1;
    }

    // P11 must be at least as big as the memory used to hold user-level plugins loaded to P11
    // This should never be true with `Force High Memory Layout`.
    // But could happen if a user plugin uses `sctrlHENApplyMemory`
    if ((p11 * 1024 * 1024) < g_plugins_loaded_mem) {
        return -4;
    }

    // Checks for unlock state
    if (p2 > 24) {
        // already enabled
        if (g_rebootex_config.ram2 > 24) {
            return -2;
        }
    } else if (p2 == 24) { // Checks for default state
        // already enabled
        if (g_rebootex_config.ram2 == 24) {
            return -2;
        }
    }

    int k1 = pspSdkSetK1(0);

    g_rebootex_config.ram2 = p2;
    g_rebootex_config.ram11 = p11;

    pspSdkSetK1(k1);
    return 0;
}

int sctrlHENApplyMemory(u32 p2) {
    int p11 = 40 - p2;
    int res = sctrlHENSetMemory(p2, (p11 < 0) ? 0 : p11);

    // It can't unlock
    if (res < 0) {
        return res;
    }

    return 0;
}

void sctrlHENSetSpeed(int cpu, int bus) {
    int k1 = pspSdkSetK1(0);
    if (cpu == 20 || cpu == 75 || cpu == 100 || cpu == 133 || cpu == 333 || cpu == 300 || cpu == 266
        || cpu == 222) {
        _scePowerSetClockFrequency_k = (void *)FindPowerFunction(0x737486F2);
        _scePowerSetClockFrequency_k(cpu, cpu, bus);

        if (sceKernelApplicationType() != PSP_INIT_KEYCONFIG_VSH) {
            MAKE_DUMMY_FUNCTION((u32)_scePowerSetClockFrequency_k, 0);
            MAKE_DUMMY_FUNCTION((u32)FindPowerFunction(0x545A7F3C), 0);
            MAKE_DUMMY_FUNCTION((u32)FindPowerFunction(0xB8D7B3FB), 0);
            MAKE_DUMMY_FUNCTION((u32)FindPowerFunction(0x843FBF43), 0);
            MAKE_DUMMY_FUNCTION((u32)FindPowerFunction(0xEBD177D6), 0);
            sctrlFlushCache();
        }
    }
    pspSdkSetK1(k1);
}

u32 sctrlHENGetSpeed() {
    u32 res = 222;
    int k1 = pspSdkSetK1(0);

    _scePowerGetClockFrequency_k = (void *)FindPowerFunction(0xFDB5BFE9);

    if (_scePowerGetClockFrequency_k != NULL) {
        res = _scePowerGetClockFrequency_k();
    }

    pspSdkSetK1(k1);
    return res;
}

KDEC_HANDLER sctrlHENRegisterKDecryptHandler(KDEC_HANDLER func) {
    KDEC_HANDLER r = g_extend_memlmd_decryption;
    g_extend_memlmd_decryption = func;
    return r;
}

MDEC_HANDLER sctrlHENRegisterMDecryptHandler(MDEC_HANDLER func) {
    MDEC_HANDLER ret = g_extend_mesgled_decrypt;
    g_extend_mesgled_decrypt = func;
    return ret;
}

////////////////////////////////////////////////////////////////////////////////
// PATCHED IMPLEMENTATIONS
////////////////////////////////////////////////////////////////////////////////

int sceSystemFileGetIndexPatched(void *sfo, void *a1, void *a2) {
    int largememory = 0;

    if (g_rebootex_config.ram2 == EXTRA_RAM_SETTABLE) {
        SFOHeader *header = (SFOHeader *)sfo;
        SFODir *entries = (SFODir *)(sfo + sizeof(SFOHeader));

        for (int i = 0; i < header->nitems; i++) {
            if (strcmp(sfo + header->fields_table_offs + entries[i].field_offs, "MEMSIZE") == 0) {
                memcpy(&largememory, sfo + header->values_table_offs + entries[i].val_offs, 4);
            }
        }

        if (g_cfw_config.high_memory == HIGHMEM_OPT_FORCE_OFF) {
            sctrlHENSetMemory(24, 16);
            ApplyMemory();
        }
        if (largememory == 1) {
            sctrlHENSetMemory(52, 0);
            ApplyMemory();
        } else if (largememory == 2) {
            sctrlHENSetMemory(40, 0);
            ApplyMemory();
        } else if (largememory > 2) {
            // Invalid value, but just to be safe, unlock stable extra RAM, but
            // left in a state that can be re-set
            sctrlHENSetMemory(40, 0);
            ApplyAndResetMemory();
        }
    } else {
        ApplyAndResetMemory();
    }

    return _sceSystemFileGetIndex(sfo, a1, a2);
}

int RunRebootPatched(u32 *params) {
    if ((char *)params[2] == NULL) {
        if (g_rebootex_config.bootfileindex != MODE_RECOVERY) {
            g_rebootex_config.bootfileindex = MODE_UMD;
        }
        memset(g_rebootex_config.umdfilename, 0, sizeof(g_rebootex_config.umdfilename));
    }

    return RunReboot(params);
}

int DecodeKL4EPatched(void *dest, u32 size_dest, void *src, u32 size_src) {
    memset((void *)REBOOTEX_TEXT, 0, REBOOTEX_MAX_SIZE);

    if (g_custom_rebootex == NULL) {
        // GZIP rebootex
        if (rebootex[0] == 0x1F && rebootex[1] == 0x8B) {
            sctrlGzipDecompress((unsigned char *)REBOOTEX_TEXT, rebootex, REBOOTEX_MAX_SIZE);
        } else { // Plain rebootex
            memcpy((void *)REBOOTEX_TEXT, rebootex, size_rebootex);
        }
        memcpy((void *)REBOOTEX_CONFIG, &g_rebootex_config, sizeof(RebootexConfigEPI));
        // Backup the CFW config across reboot
        memcpy((void *)EPI_CONFIG_ADDR, &g_cfw_config, sizeof(SEConfigEPI));
    } else {
        // GZIP rebootex
        if (((u8 *)g_custom_rebootex)[0] == 0x1F && ((u8 *)g_custom_rebootex)[1] == 0x8B) {
            sctrlGzipDecompress(
                (unsigned char *)REBOOTEX_TEXT, g_custom_rebootex, REBOOTEX_MAX_SIZE
            );
        } else { // Plain rebootex
            memcpy((void *)REBOOTEX_TEXT, g_custom_rebootex, g_custom_rebootex_size);
        }
        memset((void *)REBOOTEX_CONFIG, 0, sizeof(RebootexConfigEPI));
        if (g_custom_rebootex_config != NULL) {
            memcpy(
                (void *)REBOOTEX_CONFIG, g_custom_rebootex_config, g_custom_rebootex_config_size
            );
        }
        memset((void *)EPI_CONFIG_ADDR, 0, sizeof(SEConfigEPI));
    }
    return DecodeKL4E(dest, size_dest, src, size_src);
}

int sceChkregGetPsCodePatched(u8 *pscode) {
    int res = _sceChkregGetPsCode(pscode);

    pscode[0] = 0x01;
    pscode[1] = 0x00;

    if (g_cfw_config.fake_region) {
        pscode[2] = g_cfw_config.fake_region < 12 ? g_cfw_config.fake_region + 2
                                                  : g_cfw_config.fake_region - 11;
        if (pscode[2] == 2) {
            pscode[2] = 3;
        }
    }

    pscode[3] = 0x00;
    pscode[4] = 0x01;
    pscode[5] = 0x00;
    pscode[6] = 0x01;
    pscode[7] = 0x00;

    return res;
}

int sceCtrlPeekBufferPositivePatched(SceCtrlData *pad_data, int count) {
    if (_sceCtrlPeekBufferPositive == NULL) {
        return SCE_KERR_ILLEGAL_ADDR;
    }

    count = _sceCtrlPeekBufferPositive(pad_data, count);

    if ((pad_data->Buttons & EXIT_TO_VSH_MASK) == EXIT_TO_VSH_MASK
        || (pad_data->Buttons & EXIT_TO_VSH2_MASK) == EXIT_TO_VSH2_MASK) {
        startExitThread();
    }

    return count;
}

int sceCtrlPeekBufferNegativePatched(SceCtrlData *pad_data, int count) {
    if (_sceCtrlPeekBufferNegative == NULL) {
        return SCE_KERR_ILLEGAL_ADDR;
    }

    count = _sceCtrlPeekBufferNegative(pad_data, count);

    if ((pad_data->Buttons & EXIT_TO_VSH_MASK) == 0
        || (pad_data->Buttons & EXIT_TO_VSH2_MASK) == 0) {
        startExitThread();
    }

    return count;
}

int sceCtrlReadBufferPositivePatched(SceCtrlData *pad_data, int count) {
    if (_sceCtrlReadBufferPositive == NULL) {
        return SCE_KERR_ILLEGAL_ADDR;
    }

    count = _sceCtrlReadBufferPositive(pad_data, count);

    if ((pad_data->Buttons & EXIT_TO_VSH_MASK) == EXIT_TO_VSH_MASK
        || (pad_data->Buttons & EXIT_TO_VSH2_MASK) == EXIT_TO_VSH2_MASK) {
        startExitThread();
    }

    return count;
}

int sceCtrlReadBufferNegativePatched(SceCtrlData *pad_data, int count) {
    if (_sceCtrlReadBufferNegative == NULL) {
        return SCE_KERR_ILLEGAL_ADDR;
    }

    count = _sceCtrlReadBufferNegative(pad_data, count);

    if ((pad_data->Buttons & EXIT_TO_VSH_MASK) == 0
        || (pad_data->Buttons & EXIT_TO_VSH2_MASK) == 0) {
        startExitThread();
    }

    return count;
}

SceUID sceKernelAllocPartitionMemoryPluginPatched(
    SceUID partitionid, const char *name, int type, SceSize size, void *addr
) {
    if (partitionid == 11) {
        partitionid = 2;
        logmsg2("[INFO]: %s: `partition_id` 11 -> 2\n", __func__);
    }

    u32 k1 = pspSdkSetK1(0);

    SceUID res = sceKernelAllocPartitionMemory(partitionid, name, type, size, addr);

    pspSdkSetK1(k1);

    return res;
}

SceUID sceKernelCreateThreadPluginPatched(
    const char *name, SceKernelThreadEntry entry, int init_priority, int stack_size, SceUInt attr,
    SceKernelThreadOptParam *option
) {
    if (option != NULL && option->stackMpid == 11) {
        option->stackMpid = 2;
        logmsg2("[INFO]: %s: `stack_mpid` 11 -> 2\n", __func__);
    }

    u32 k1 = pspSdkSetK1(0);

    SceUID res = sceKernelCreateThread(name, entry, init_priority, stack_size, attr, option);

    pspSdkSetK1(k1);

    return res;
}

int memlmd_unsigner_patched(u8 *prx, u32 size, int use_polling) {
    // ~PSP
    if (prx != NULL && *(u32 *)prx == 0x5053507E) {
        u32 i;
        int result = 0;

        // 6.60 kernel modules use type 9 PRX... 0xd4~0x104 is zero padded
        for (i = 0; i < 0x30; ++i) {
            if (prx[i + 0xd4]) {
                result = 1;
                break;
            }
        }

        if (result == 0) {
            return 0;
        }

        result = 0;

        // Updater prx ... 0xB8~0x18 is zero padded
        for (i = 0; i < 0x18; ++i) {
            if (prx[i + 0xB8]) {
                result = 1;
                break;
            }
        }

        if (result == 0) {
            return 0;
        }
    }

    return (*_memlmd_unsigner)(prx, size, use_polling);
}

int memlmd_decrypt_patched(u8 *prx, u32 size, u32 *newsize, int use_polling) {
    int ret = 0;
    if (g_extend_memlmd_decryption) {
        ret = g_extend_memlmd_decryption(prx, size, newsize, use_polling);
        if (ret >= 0) {
            return ret;
        }
    }

    // Besides the call to the original function, the things below here are not
    // really needed for PSPemu version of `memlmd_01g.prx`, but can be useful
    // if we ever change to PSP version.

    if (prx != NULL && newsize != NULL) {
        u32 compsize = *(u32 *)(prx + 0xB0);

        if (is_prx_compressed(prx, size)) {
            // Remove PRX header
            memcpy(prx, prx + 0x150, compsize);

            // Write GZIP payload size
            *newsize = compsize;

            return 0;
        }
    }

    ret = _memlmd_decrypt(prx, size, newsize, use_polling);

    if (ret < 0 && _memlmd_unsigner != NULL && _memlmd_StartScrambleKey != NULL) {
        if (_memlmd_StartScrambleKey != NULL
            && _memlmd_StartScrambleKey(0, (void *)0xBFC00200) < 0) {
            return ret;
        }

        // Enable user modules to load kernel signchecked modules
        if (_memlmd_unsigner != NULL && memlmd_unsigner_patched(prx, size, use_polling) < 0) {
            return ret;
        }

        ret = _memlmd_decrypt(prx, size, newsize, use_polling);
    }

    return ret;
}

int mesgled_decrypt_patched(
    u32 *tag, u8 *key, u32 code, u8 *prx, u32 size, u32 *newsize, u32 use_polling, u8 *blacklist,
    u32 blacklistsize, u32 type, u8 *xor_key1, u8 *xor_key2
) {
    int ret = 0;
    if (prx != NULL && newsize != NULL) {
        if (g_extend_mesgled_decrypt != NULL) {
            ret = g_extend_mesgled_decrypt(
                tag, key, code, prx, size, newsize, use_polling, blacklist, blacklistsize, type,
                xor_key1, xor_key2
            );

            if (ret >= 0) {
                return ret;
            }
        }

        u32 compsize = *(u32 *)(prx + 0xB0);

        if (is_prx_compressed(prx, size)) {
            memcpy(prx, prx + 0x150, compsize);

            *newsize = compsize;

            return 0;
        }
    }

    ret = _mesgled_decrypt(
        tag, key, code, prx, size, newsize, use_polling, blacklist, blacklistsize, type, xor_key1,
        xor_key2
    );

    return ret;
}

////////////////////////////////////////////////////////////////////////////////
// MODULE PATCHERS
////////////////////////////////////////////////////////////////////////////////

void PatchLoadExec(SceModule *mod) {
    u32 text_addr = mod->text_addr;
    u32 text_size = mod->text_size;

    // Allow loadexec in whatever user level. Ignore K1 Check
    VWRITE16(text_addr + 0x16A6, 0x1000);
    VWRITE16(text_addr + 0x241E, 0x1000);
    VWRITE16(text_addr + 0x2622, 0x1000);

    for (int i = 0; i < text_size; i += 4) {
        u32 addr = text_addr + i;
        u32 data = VREAD32(addr);

        // Patch to do things before reboot
        if (data == 0x02202021 && VREAD32(addr + 4) == 0x00401821) {
            K_HIJACK_CALL(addr - 4, RunRebootPatched, RunReboot);
            continue;
        }

        // Redirect pointer to 0x88FC0000
        if (data == 0x04400020) {
            K_HIJACK_CALL(addr - 8, DecodeKL4EPatched, DecodeKL4E);
            VWRITE8(addr + 0x44, 0xFC);
            continue;
        }
    }

    sctrlFlushCache();
}

void PatchChkreg() {
    HIJACK_FUNCTION(
        K_EXTRACT_IMPORT(&sceChkregGetPsCode), sceChkregGetPsCodePatched, _sceChkregGetPsCode
    );
    sctrlFlushCache();
}

void PatchMediaSync(SceModule *mod) {
    u32 text_addr = mod->text_addr;

    // Dummy function that checks flash0 files
    MAKE_INSTRUCTION(text_addr + 0xC8, 0x00001021);

    // Fixes: ELF boot, boot not from /PSP/GAME/
    MAKE_INSTRUCTION(text_addr + 0x864, 0x00008821);
    MAKE_INSTRUCTION(text_addr + 0x988, 0x00008821);

    // Avoid SCE_MEDIASYNC_ERROR_INVALID_MEDIA
    VWRITE16(text_addr + 0x3C6, 0x5000);
    VWRITE16(text_addr + 0xDCA, 0x1000);

    K_HIJACK_CALL(text_addr + 0x97C, sceSystemFileGetIndexPatched, _sceSystemFileGetIndex);

    sctrlFlushCache();
}

void PatchController(SceModule *mod) {
    _sceCtrlPeekBufferPositive =
        (void *)sctrlHENFindFunctionInMod(mod, "sceCtrl_driver", 0x3A622550);
    _sceCtrlPeekBufferNegative =
        (void *)sctrlHENFindFunctionInMod(mod, "sceCtrl_driver", 0xC152080A);
    _sceCtrlReadBufferPositive =
        (void *)sctrlHENFindFunctionInMod(mod, "sceCtrl_driver", 0x1F803938);
    _sceCtrlReadBufferNegative =
        (void *)sctrlHENFindFunctionInMod(mod, "sceCtrl_driver", 0x60B81F86);

    HIJACK_FUNCTION(
        _sceCtrlPeekBufferPositive, sceCtrlPeekBufferPositivePatched, _sceCtrlPeekBufferPositive
    );
    HIJACK_FUNCTION(
        _sceCtrlPeekBufferNegative, sceCtrlPeekBufferNegativePatched, _sceCtrlPeekBufferNegative
    );
    HIJACK_FUNCTION(
        _sceCtrlReadBufferPositive, sceCtrlReadBufferPositivePatched, _sceCtrlReadBufferPositive
    );
    HIJACK_FUNCTION(
        _sceCtrlReadBufferNegative, sceCtrlReadBufferNegativePatched, _sceCtrlReadBufferNegative
    );

    sctrlFlushCache();
}

void PatchInterruptMgr() {
    SceModule *mod = sceKernelFindModuleByName("sceInterruptManager");
    u32 text_addr = mod->text_addr;

    // Allow execution of syscalls in kernel mode
    MAKE_INSTRUCTION(text_addr + 0xE98, 0x408F7000);
    MAKE_NOP(text_addr + 0xE9C);
}

void PatchSysmem() {
    u32 nids[] = {0x7591C7DB, 0x342061E5, 0x315AD3A0, 0xEBD5C3E6, 0x057E7380,
                  0x91DE343C, 0x7893F79A, 0x35669D4C, 0x1B4217BC, 0x358CA1BB};

    SceModule *mod = sceKernelFindModuleByName("sceSystemMemoryManager");
    for (int i = 0; i < sizeof(nids) / sizeof(u32); i++) {
        u32 addr =
            sctrlHENFindFirstBEQ(sctrlHENFindFunctionInMod(mod, "SysMemUserForUser", nids[i]));
        if (addr) {
            VWRITE16(addr + 2, 0x1000);
        }
    }

    extern SceSize (*_sceKernelMaxFreeMemSize)(void);
    extern SceSize (*_sceKernelTotalFreeMemSize)(void);
    extern SceSize (*_sceKernelTotalMemSize)(void);

    _sceKernelMaxFreeMemSize =
        (void *)sctrlHENFindFunctionInMod(mod, "SysMemUserForUser", 0xA291F107);
    _sceKernelTotalFreeMemSize =
        (void *)sctrlHENFindFunctionInMod(mod, "SysMemUserForUser", 0xF919F628);
    _sceKernelTotalMemSize =
        (void *)sctrlHENFindFunctionInMod(mod, "SysMemUserForUser", 0xACBD88CA);
}

void PatchMemlmd(void) {
    SceModule *mod = sceKernelFindModuleByName("sceMemlmd");

    u32 text_addr = mod->text_addr;
    u32 top_addr = mod->text_addr + mod->text_size;

    int patches = 5;
    for (u32 addr = text_addr; addr < top_addr && patches; addr += 4) {
        u32 data = VREAD32(addr);

        if (data == JAL(_memlmd_decrypt)) {
            MAKE_CALL(addr, memlmd_decrypt_patched);
            patches -= 1; // 2
        } else if (data == 0x27BDFF80) {
            _memlmd_decrypt = (void *)(addr - 8);
        } else if (data == JAL(_memlmd_unsigner)) {
            MAKE_CALL(addr, memlmd_unsigner_patched);
            patches -= 1; // 2
        } else if (data == 0x3C02F009) {
            // Allow to load packed usermode module.
            VWRITE16(addr, 0xF005);
            patches--;
        } else if (data == 0x3222003F) {
            u32 a = addr;
            do {
                a -= 4;
            } while (VREAD32(a) != 0x27BDFFF0);

            // The inner function which unsigns a PRX module
            _memlmd_unsigner = (void *)a;
        } else if (data == 0x2403FF31 && _memlmd_StartScrambleKey == NULL) {
            _memlmd_StartScrambleKey = (void *)(addr - 8);
        }
    }

    sctrlFlushCache();
}

void PatchMesgLed(SceModule *mod) {
    u32 topaddr = mod->text_addr + mod->text_size;

    for (u32 addr = mod->text_addr; addr < topaddr; addr += 4) {
        u32 data = VREAD32(addr);

        if (data == 0x2CE30001 && _mesgled_decrypt == NULL) {
            _mesgled_decrypt = (void *)addr;
        } else if (data == JAL(_mesgled_decrypt)) {
            MAKE_CALL(addr, mesgled_decrypt_patched);
        }
    }
}

void PatchPluginModule(SceModule *mod) {

    // Fix 6.60 plugins on 6.61
    const char *sysmemlib =
        (mod->mod_state & SCE_MODULE_USER_MODULE) ? "SysMemUserForUser" : "SysMemForKernel";
    sctrlHookImportByNID(mod, sysmemlib, 0x3FC9AE6A, &sctrlHENFakeDevkitVersion);

    // If p11 is too small and module is a user module, patch function to use p2 instead of p11

    if ((mod->mod_state & SCE_MODULE_USER_MODULE) && g_cfw_config.high_memory == HIGHMEM_OPT_DEFAULT
        && ((g_rebootex_config.ram11 * 1024 * 1024) <= g_plugins_loaded_mem
            || g_rebootex_config.ram11 <= 2)) {
        sctrlHookImportByNID(
            mod, "ThreadManForUser", 0x446D8DE6, sceKernelCreateThreadPluginPatched
        );
        sctrlHookImportByNID(
            mod, "SysMemUserForUser", 0x237DBD4F, sceKernelAllocPartitionMemoryPluginPatched
        );
    }
}
