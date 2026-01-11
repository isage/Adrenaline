#include <pspmoduleexport.h>
#define NULL ((void *) 0)

extern int module_start;
extern int module_info;
static const unsigned int __syslib_exports[4] __attribute__((section(".rodata.sceResident"))) = {
	0xD3744BE0,
	0xF01D73A7,
	(unsigned int) &module_start,
	(unsigned int) &module_info,
};

extern int sceLflashFatfmtStartFatfmt;
static const unsigned int __LflashFatfmt_exports[2] __attribute__((section(".rodata.sceResident"))) = {
	0xB7A424A4,
	(unsigned int) &sceLflashFatfmtStartFatfmt,
};

static const unsigned int __sceLFatFs_driver_exports[0] __attribute__((section(".rodata.sceResident"))) = {
};

static const unsigned int __sceDve_driver_exports[0] __attribute__((section(".rodata.sceResident"))) = {
};

static const unsigned int __sceLed_driver_exports[0] __attribute__((section(".rodata.sceResident"))) = {
};

static const unsigned int __sceCodec_driver_exports[0] __attribute__((section(".rodata.sceResident"))) = {
};

static const unsigned int __sceClockgen_driver_exports[0] __attribute__((section(".rodata.sceResident"))) = {
};

static const unsigned int __sceNand_driver_exports[0] __attribute__((section(".rodata.sceResident"))) = {
};

static const unsigned int __sceSyscon_driver_exports[0] __attribute__((section(".rodata.sceResident"))) = {
};

extern int kuKernelBootFrom;
extern int kuKernelCall;
extern int kuKernelCallExtendStack;
extern int kuKernelFindModuleByAddress;
extern int kuKernelFindModuleByName;
extern int kuKernelGetModel;
extern int kuKernelGetUmdFile;
extern int kuKernelGetUserLevel;
extern int kuKernelIcacheInvalidateAll;
extern int kuKernelInitApitype;
extern int kuKernelInitFileName;
extern int kuKernelInitKeyConfig;
extern int kuKernelLoadModule;
extern int kuKernelLoadModuleWithApitype2;
extern int kuKernelMemcpy;
extern int kuKernelPeekw;
extern int kuKernelPokew;
extern int kuKernelSetDdrMemoryProtection;
extern int kuKernelInitKeyConfig;
static const unsigned int __KUBridge_exports[38] __attribute__((section(".rodata.sceResident"))) = {
	0x60DDB4AE,
	0x9060F69D,
	0x5C6C3DBA,
	0xCC5F0398,
	0x4B321167,
	0x24331850,
	0xD0D05A5B,
	0xA2ABB6D3,
	0x219DE4D2,
	0x8E5A4057,
	0x1742445F,
	0xB0B8824E,
	0x4C25EA72,
	0x1E9F0498,
	0x6B4B577F,
	0x7A50075E,
	0x0E73A39D,
	0xC4AF12AB,
	0x501E983D,
	(unsigned int) &kuKernelBootFrom,
	(unsigned int) &kuKernelCall,
	(unsigned int) &kuKernelCallExtendStack,
	(unsigned int) &kuKernelFindModuleByAddress,
	(unsigned int) &kuKernelFindModuleByName,
	(unsigned int) &kuKernelGetModel,
	(unsigned int) &kuKernelGetUmdFile,
	(unsigned int) &kuKernelGetUserLevel,
	(unsigned int) &kuKernelIcacheInvalidateAll,
	(unsigned int) &kuKernelInitApitype,
	(unsigned int) &kuKernelInitFileName,
	(unsigned int) &kuKernelInitKeyConfig,
	(unsigned int) &kuKernelLoadModule,
	(unsigned int) &kuKernelLoadModuleWithApitype2,
	(unsigned int) &kuKernelMemcpy,
	(unsigned int) &kuKernelPeekw,
	(unsigned int) &kuKernelPokew,
	(unsigned int) &kuKernelSetDdrMemoryProtection,
	(unsigned int) &kuKernelInitKeyConfig,
};

extern int sctrlHENApplyMemory;
extern int sctrlHENFindDriver;
extern int sctrlHENFindFunction;
extern int sctrlHENFindFunctionInMod;
extern int sctrlHENFindImportInMod;
extern int sctrlHENFindImportLib;
extern int sctrlHENFindFunctionOnSystem;
extern int sctrlHENGetVersion;
extern int sctrlHENGetMinorVersion;
extern int sctrlHENHookFunctionByNID;
extern int sctrlHookImportByNID;
extern int sctrlHENIsDevhook;
extern int sctrlHENIsSE;
extern int sctrlHENIsToolKit;
extern int sctrlHENIsToolKit;
extern int sctrlHENIsSystemBooted;
extern int sctrlHENSetMemory;
extern int sctrlHENSetSpeed;
extern int sctrlHENSetStartModuleHandler;
extern int sctrlHENFakeDevkitVersion;
extern int sctrlKernelExitVSH;
extern int sctrlKernelLoadExecVSHDisc;
extern int sctrlKernelLoadExecVSHDiscUpdater;
extern int sctrlKernelLoadExecVSHMs1;
extern int sctrlKernelLoadExecVSHMs2;
extern int sctrlKernelLoadExecVSHMs3;
extern int sctrlKernelLoadExecVSHMs4;
extern int sctrlKernelLoadExecVSHMs5;
extern int sctrlKernelLoadExecVSHEf1;
extern int sctrlKernelLoadExecVSHEf2;
extern int sctrlKernelLoadExecVSHEf3;
extern int sctrlKernelLoadExecVSHEf4;
extern int sctrlKernelLoadExecVSHEf5;
extern int sctrlKernelLoadExecVSHWithApitype;
extern int sctrlKernelQuerySystemCall;
extern int sctrlKernelRand;
extern int sctrlKernelBootFrom;
extern int sctrlKernelSetUserLevel;
extern int sctrlKernelSetDevkitVersion;
extern int sctrlKernelSetInitApitype;
extern int sctrlKernelSetInitFileName;
extern int sctrlKernelSetInitFileName;
extern int sctrlKernelSetInitKeyConfig;
extern int sctrlKernelSetInitKeyConfig;
extern int sctrlKernelSetInitKeyConfig;
extern int sctrlKernelResolveNid;
extern int sctrlKernelSetNidResolver;
extern int sctrlSEGetConfig;
extern int sctrlSEGetConfigEx;
extern int sctrlSEGetDiscType;
extern int sctrlSEGetVersion;
extern int sctrlSEMountUmdFromFile;
extern int sctrlSESetConfig;
extern int sctrlSESetConfigEx;
extern int sctrlSESetDiscType;
extern int sctrlStartUsb;
extern int sctrlStopUsb;
extern int sctrlSEGetBootConfFileIndex;
extern int sctrlSESetBootConfFileIndex;
extern int sctrlSESetBootConfFileIndex;
extern int sctrlSESetDiscOut;
extern int sctrlSEUmountUmd;
extern int sctrlFlushCache;
extern int sctrlGetInitPARAM;
extern int sctrlGetSfoPARAM;
extern int sctrlGetThreadUIDByName;
extern int sctrlGetUsbState;
extern int sctrlRebootDevice;
extern int sctrlDeflateDecompress;
extern int sctrlGzipDecompress;
extern int LZ4_decompress_fast;
extern int lzo1x_decompress;
extern int sctrlPatchModule;
extern int sctrlModuleTextAddr;
extern int sctrlHENFindImportLib;
static const unsigned int __SystemCtrlForUser_exports[150] __attribute__((section(".rodata.sceResident"))) = {
	0x8B2D99E4,
	0x78E46415,
	0x159AF5CC,
	0x46268461,
	0xDF306CFC,
	0x030C353B,
	0xBC66A655,
	0x1090A2E1,
	0x5328B431,
	0xBD8558C9,
	0x72F47790,
	0x2E2935EF,
	0xD339E2E9,
	0x2552B9A6,
	0x22A45DB7,
	0xC4D88D50,
	0x745286D1,
	0xCC9ADCF8,
	0x1C90BECB,
	0xDA05C1E5,
	0x2794CCF4,
	0x577AF198,
	0x94FE5E4B,
	0x75643FCA,
	0xABA7F1B0,
	0x7B369596,
	0xD690750F,
	0x3D79B0F1,
	0xE338EBB9,
	0xAF22D576,
	0x6D7149C9,
	0x15B5C68F,
	0xF27F9678,
	0x2D10FB28,
	0x56CEAF00,
	0xB364FBB4,
	0x053FAC1D,
	0xEB74FE45,
	0xD8FF9B99,
	0x8D5BE1F0,
	0x128112C3,
	0x2BA770CD,
	0xCB76B778,
	0xCB76B778,
	0xB551220C,
	0x32677DD3,
	0x603EE1D0,
	0x16C3B7EE,
	0x8E426F09,
	0xABEF849B,
	0xB47C9D77,
	0x85B520C6,
	0x1DDDAD0C,
	0xAD4D5EA5,
	0x31C6160D,
	0x80C0ED7B,
	0x5FC12767,
	0x70B92B45,
	0xBC939DC1,
	0x5CB025F0,
	0xFFEFA034,
	0x512E0CD8,
	0x17691875,
	0xFCE44FB8,
	0x0796385F,
	0x23833651,
	0x05D8E209,
	0x053172F8,
	0xF462EE55,
	0x5D665044,
	0x16100529,
	0x76C382FF,
	0x62CAC4CF,
	0x2A868045,
	0xFAC22931,
	(unsigned int) &sctrlHENApplyMemory,
	(unsigned int) &sctrlHENFindDriver,
	(unsigned int) &sctrlHENFindFunction,
	(unsigned int) &sctrlHENFindFunctionInMod,
	(unsigned int) &sctrlHENFindImportInMod,
	(unsigned int) &sctrlHENFindImportLib,
	(unsigned int) &sctrlHENFindFunctionOnSystem,
	(unsigned int) &sctrlHENGetVersion,
	(unsigned int) &sctrlHENGetMinorVersion,
	(unsigned int) &sctrlHENHookFunctionByNID,
	(unsigned int) &sctrlHookImportByNID,
	(unsigned int) &sctrlHENIsDevhook,
	(unsigned int) &sctrlHENIsSE,
	(unsigned int) &sctrlHENIsToolKit,
	(unsigned int) &sctrlHENIsToolKit,
	(unsigned int) &sctrlHENIsSystemBooted,
	(unsigned int) &sctrlHENSetMemory,
	(unsigned int) &sctrlHENSetSpeed,
	(unsigned int) &sctrlHENSetStartModuleHandler,
	(unsigned int) &sctrlHENFakeDevkitVersion,
	(unsigned int) &sctrlKernelExitVSH,
	(unsigned int) &sctrlKernelLoadExecVSHDisc,
	(unsigned int) &sctrlKernelLoadExecVSHDiscUpdater,
	(unsigned int) &sctrlKernelLoadExecVSHMs1,
	(unsigned int) &sctrlKernelLoadExecVSHMs2,
	(unsigned int) &sctrlKernelLoadExecVSHMs3,
	(unsigned int) &sctrlKernelLoadExecVSHMs4,
	(unsigned int) &sctrlKernelLoadExecVSHMs5,
	(unsigned int) &sctrlKernelLoadExecVSHEf1,
	(unsigned int) &sctrlKernelLoadExecVSHEf2,
	(unsigned int) &sctrlKernelLoadExecVSHEf3,
	(unsigned int) &sctrlKernelLoadExecVSHEf4,
	(unsigned int) &sctrlKernelLoadExecVSHEf5,
	(unsigned int) &sctrlKernelLoadExecVSHWithApitype,
	(unsigned int) &sctrlKernelQuerySystemCall,
	(unsigned int) &sctrlKernelRand,
	(unsigned int) &sctrlKernelBootFrom,
	(unsigned int) &sctrlKernelSetUserLevel,
	(unsigned int) &sctrlKernelSetDevkitVersion,
	(unsigned int) &sctrlKernelSetInitApitype,
	(unsigned int) &sctrlKernelSetInitFileName,
	(unsigned int) &sctrlKernelSetInitFileName,
	(unsigned int) &sctrlKernelSetInitKeyConfig,
	(unsigned int) &sctrlKernelSetInitKeyConfig,
	(unsigned int) &sctrlKernelSetInitKeyConfig,
	(unsigned int) &sctrlKernelResolveNid,
	(unsigned int) &sctrlKernelSetNidResolver,
	(unsigned int) &sctrlSEGetConfig,
	(unsigned int) &sctrlSEGetConfigEx,
	(unsigned int) &sctrlSEGetDiscType,
	(unsigned int) &sctrlSEGetVersion,
	(unsigned int) &sctrlSEMountUmdFromFile,
	(unsigned int) &sctrlSESetConfig,
	(unsigned int) &sctrlSESetConfigEx,
	(unsigned int) &sctrlSESetDiscType,
	(unsigned int) &sctrlStartUsb,
	(unsigned int) &sctrlStopUsb,
	(unsigned int) &sctrlSEGetBootConfFileIndex,
	(unsigned int) &sctrlSESetBootConfFileIndex,
	(unsigned int) &sctrlSESetBootConfFileIndex,
	(unsigned int) &sctrlSESetDiscOut,
	(unsigned int) &sctrlSEUmountUmd,
	(unsigned int) &sctrlFlushCache,
	(unsigned int) &sctrlGetInitPARAM,
	(unsigned int) &sctrlGetSfoPARAM,
	(unsigned int) &sctrlGetThreadUIDByName,
	(unsigned int) &sctrlGetUsbState,
	(unsigned int) &sctrlRebootDevice,
	(unsigned int) &sctrlDeflateDecompress,
	(unsigned int) &sctrlGzipDecompress,
	(unsigned int) &LZ4_decompress_fast,
	(unsigned int) &lzo1x_decompress,
	(unsigned int) &sctrlPatchModule,
	(unsigned int) &sctrlModuleTextAddr,
	(unsigned int) &sctrlHENFindImportLib,
};

extern int sctrlHENApplyMemory;
extern int sctrlHENFindDriver;
extern int sctrlHENFindFunction;
extern int sctrlHENFindFunctionInMod;
extern int sctrlHENFindImportInMod;
extern int sctrlHENFindImportLib;
extern int sctrlHENFindFunctionOnSystem;
extern int sctrlHENGetVersion;
extern int sctrlHENGetMinorVersion;
extern int sctrlHENHookFunctionByNID;
extern int sctrlHookImportByNID;
extern int sctrlHENIsDevhook;
extern int sctrlHENIsSE;
extern int sctrlHENIsToolKit;
extern int sctrlHENIsToolKit;
extern int sctrlHENIsSystemBooted;
extern int sctrlHENSetMemory;
extern int sctrlHENSetSpeed;
extern int sctrlHENSetStartModuleHandler;
extern int sctrlHENLoadModuleOnReboot;
extern int sctrlHENPatchSyscall;
extern int sctrlHENPatchSyscall;
extern int sctrlHENPatchSyscall;
extern int sctrlHENRegisterHomebrewLoader;
extern int sctrlHENGetInitControl;
extern int sctrlHENTakeInitControl;
extern int sctrlHENGetRebootexConfig;
extern int sctrlKernelExitVSH;
extern int sctrlKernelLoadExecVSHDisc;
extern int sctrlKernelLoadExecVSHDiscUpdater;
extern int sctrlKernelLoadExecVSHMs1;
extern int sctrlKernelLoadExecVSHMs2;
extern int sctrlKernelLoadExecVSHMs3;
extern int sctrlKernelLoadExecVSHMs4;
extern int sctrlKernelLoadExecVSHMs5;
extern int sctrlKernelLoadExecVSHEf1;
extern int sctrlKernelLoadExecVSHEf2;
extern int sctrlKernelLoadExecVSHEf3;
extern int sctrlKernelLoadExecVSHEf4;
extern int sctrlKernelLoadExecVSHEf5;
extern int sctrlKernelLoadExecVSHWithApitype;
extern int sctrlKernelQuerySystemCall;
extern int sctrlKernelRand;
extern int sctrlKernelBootFrom;
extern int sctrlKernelSetUserLevel;
extern int sctrlKernelSetDevkitVersion;
extern int sctrlKernelSetInitApitype;
extern int sctrlKernelSetInitFileName;
extern int sctrlKernelSetInitFileName;
extern int sctrlKernelSetInitKeyConfig;
extern int sctrlKernelSetInitKeyConfig;
extern int sctrlKernelSetInitKeyConfig;
extern int sctrlKernelResolveNid;
extern int sctrlKernelSetNidResolver;
extern int sctrlSEGetConfig;
extern int sctrlSEGetConfigEx;
extern int sctrlSEGetDiscType;
extern int sctrlSEGetVersion;
extern int sctrlSEMountUmdFromFile;
extern int sctrlSESetConfig;
extern int sctrlSESetConfigEx;
extern int sctrlSESetDiscType;
extern int sctrlSEGetBootConfFileIndex;
extern int sctrlSESetBootConfFileIndex;
extern int sctrlSESetBootConfFileIndex;
extern int sctrlSESetDiscOut;
extern int sctrlSEUmountUmd;
extern int GetUmdFile;
extern int SetUmdFile;
extern int sctrlSEApplyConfig;
extern int sctrlSEApplyConfigEX;
extern int sctrlSEGetUmdFile;
extern int sctrlSEGetUmdFileEx;
extern int sctrlSESetUmdFile;
extern int sctrlSESetUmdFileEx;
extern int sctrlFlushCache;
extern int sctrlGetInitPARAM;
extern int sctrlGetSfoPARAM;
extern int sctrlGetThreadUIDByName;
extern int sctrlGetUsbState;
extern int sctrlRebootDevice;
extern int sctrlStartUsb;
extern int sctrlStopUsb;
extern int ApplyMemory;
extern int SetSpeed;
extern int lowerString;
extern int oe_free;
extern int oe_malloc;
extern int strcasecmp;
extern int strncasecmp;
extern int strncat;
extern int strncat_s;
extern int strncpy_s;
extern int user_free;
extern int user_malloc;
extern int user_memalign;
extern int LZ4_decompress_fast;
extern int lzo1x_decompress;
extern int sctrlPatchModule;
extern int sctrlModuleTextAddr;
extern int sctrlHENFindImportLib;
extern int sctrlSetCustomStartModule;
extern int sctrlSetStartModuleExtra;
extern int sctrlGetInitTextAddr;
extern int sctrlGetInitTextAddr;
static const unsigned int __SystemCtrlForKernel_exports[210] __attribute__((section(".rodata.sceResident"))) = {
	0x8B2D99E4,
	0x78E46415,
	0x159AF5CC,
	0x46268461,
	0xDF306CFC,
	0x030C353B,
	0xBC66A655,
	0x1090A2E1,
	0x5328B431,
	0xBD8558C9,
	0x72F47790,
	0x2E2935EF,
	0xD339E2E9,
	0x2552B9A6,
	0x22A45DB7,
	0xC4D88D50,
	0x745286D1,
	0xCC9ADCF8,
	0x1C90BECB,
	0xCE0A654E,
	0xF988C1DC,
	0x826668E9,
	0x02BFCB5F,
	0x07232EA5,
	0x72F29A6E,
	0x344A971E,
	0x18B687A6,
	0x2794CCF4,
	0x577AF198,
	0x94FE5E4B,
	0x75643FCA,
	0xABA7F1B0,
	0x7B369596,
	0xD690750F,
	0x3D79B0F1,
	0xE338EBB9,
	0xAF22D576,
	0x6D7149C9,
	0x15B5C68F,
	0xF27F9678,
	0x2D10FB28,
	0x56CEAF00,
	0xB364FBB4,
	0x053FAC1D,
	0xEB74FE45,
	0xD8FF9B99,
	0x8D5BE1F0,
	0x128112C3,
	0x2BA770CD,
	0xCB76B778,
	0xCB76B778,
	0xB551220C,
	0x32677DD3,
	0x603EE1D0,
	0x16C3B7EE,
	0x8E426F09,
	0xABEF849B,
	0xB47C9D77,
	0x85B520C6,
	0x1DDDAD0C,
	0xAD4D5EA5,
	0x31C6160D,
	0x70B92B45,
	0xBC939DC1,
	0x5CB025F0,
	0xFFEFA034,
	0x512E0CD8,
	0xAC56B90B,
	0xB64186D0,
	0xFFE1D172,
	0x373D2D2E,
	0xBA21998E,
	0xBBF21F64,
	0x5A35C948,
	0xCF817542,
	0x17691875,
	0xFCE44FB8,
	0x0796385F,
	0x23833651,
	0x05D8E209,
	0x053172F8,
	0x80C0ED7B,
	0x5FC12767,
	0xB86E36D1,
	0x98012538,
	0x983B00FB,
	0xA65E8BC4,
	0xF9584CAD,
	0x3EB35691,
	0x7BA27B01,
	0xD3D1A3B9,
	0xEFB593C9,
	0x5ABF13F5,
	0x12BC667F,
	0x4C0FE24C,
	0xB19C939A,
	0x16100529,
	0x76C382FF,
	0x62CAC4CF,
	0x2A868045,
	0xFAC22931,
	0x259B51CE,
	0x221400A6,
	0x557F0B8C,
	0x72F29A6E,
	(unsigned int) &sctrlHENApplyMemory,
	(unsigned int) &sctrlHENFindDriver,
	(unsigned int) &sctrlHENFindFunction,
	(unsigned int) &sctrlHENFindFunctionInMod,
	(unsigned int) &sctrlHENFindImportInMod,
	(unsigned int) &sctrlHENFindImportLib,
	(unsigned int) &sctrlHENFindFunctionOnSystem,
	(unsigned int) &sctrlHENGetVersion,
	(unsigned int) &sctrlHENGetMinorVersion,
	(unsigned int) &sctrlHENHookFunctionByNID,
	(unsigned int) &sctrlHookImportByNID,
	(unsigned int) &sctrlHENIsDevhook,
	(unsigned int) &sctrlHENIsSE,
	(unsigned int) &sctrlHENIsToolKit,
	(unsigned int) &sctrlHENIsToolKit,
	(unsigned int) &sctrlHENIsSystemBooted,
	(unsigned int) &sctrlHENSetMemory,
	(unsigned int) &sctrlHENSetSpeed,
	(unsigned int) &sctrlHENSetStartModuleHandler,
	(unsigned int) &sctrlHENLoadModuleOnReboot,
	(unsigned int) &sctrlHENPatchSyscall,
	(unsigned int) &sctrlHENPatchSyscall,
	(unsigned int) &sctrlHENPatchSyscall,
	(unsigned int) &sctrlHENRegisterHomebrewLoader,
	(unsigned int) &sctrlHENGetInitControl,
	(unsigned int) &sctrlHENTakeInitControl,
	(unsigned int) &sctrlHENGetRebootexConfig,
	(unsigned int) &sctrlKernelExitVSH,
	(unsigned int) &sctrlKernelLoadExecVSHDisc,
	(unsigned int) &sctrlKernelLoadExecVSHDiscUpdater,
	(unsigned int) &sctrlKernelLoadExecVSHMs1,
	(unsigned int) &sctrlKernelLoadExecVSHMs2,
	(unsigned int) &sctrlKernelLoadExecVSHMs3,
	(unsigned int) &sctrlKernelLoadExecVSHMs4,
	(unsigned int) &sctrlKernelLoadExecVSHMs5,
	(unsigned int) &sctrlKernelLoadExecVSHEf1,
	(unsigned int) &sctrlKernelLoadExecVSHEf2,
	(unsigned int) &sctrlKernelLoadExecVSHEf3,
	(unsigned int) &sctrlKernelLoadExecVSHEf4,
	(unsigned int) &sctrlKernelLoadExecVSHEf5,
	(unsigned int) &sctrlKernelLoadExecVSHWithApitype,
	(unsigned int) &sctrlKernelQuerySystemCall,
	(unsigned int) &sctrlKernelRand,
	(unsigned int) &sctrlKernelBootFrom,
	(unsigned int) &sctrlKernelSetUserLevel,
	(unsigned int) &sctrlKernelSetDevkitVersion,
	(unsigned int) &sctrlKernelSetInitApitype,
	(unsigned int) &sctrlKernelSetInitFileName,
	(unsigned int) &sctrlKernelSetInitFileName,
	(unsigned int) &sctrlKernelSetInitKeyConfig,
	(unsigned int) &sctrlKernelSetInitKeyConfig,
	(unsigned int) &sctrlKernelSetInitKeyConfig,
	(unsigned int) &sctrlKernelResolveNid,
	(unsigned int) &sctrlKernelSetNidResolver,
	(unsigned int) &sctrlSEGetConfig,
	(unsigned int) &sctrlSEGetConfigEx,
	(unsigned int) &sctrlSEGetDiscType,
	(unsigned int) &sctrlSEGetVersion,
	(unsigned int) &sctrlSEMountUmdFromFile,
	(unsigned int) &sctrlSESetConfig,
	(unsigned int) &sctrlSESetConfigEx,
	(unsigned int) &sctrlSESetDiscType,
	(unsigned int) &sctrlSEGetBootConfFileIndex,
	(unsigned int) &sctrlSESetBootConfFileIndex,
	(unsigned int) &sctrlSESetBootConfFileIndex,
	(unsigned int) &sctrlSESetDiscOut,
	(unsigned int) &sctrlSEUmountUmd,
	(unsigned int) &GetUmdFile,
	(unsigned int) &SetUmdFile,
	(unsigned int) &sctrlSEApplyConfig,
	(unsigned int) &sctrlSEApplyConfigEX,
	(unsigned int) &sctrlSEGetUmdFile,
	(unsigned int) &sctrlSEGetUmdFileEx,
	(unsigned int) &sctrlSESetUmdFile,
	(unsigned int) &sctrlSESetUmdFileEx,
	(unsigned int) &sctrlFlushCache,
	(unsigned int) &sctrlGetInitPARAM,
	(unsigned int) &sctrlGetSfoPARAM,
	(unsigned int) &sctrlGetThreadUIDByName,
	(unsigned int) &sctrlGetUsbState,
	(unsigned int) &sctrlRebootDevice,
	(unsigned int) &sctrlStartUsb,
	(unsigned int) &sctrlStopUsb,
	(unsigned int) &ApplyMemory,
	(unsigned int) &SetSpeed,
	(unsigned int) &lowerString,
	(unsigned int) &oe_free,
	(unsigned int) &oe_malloc,
	(unsigned int) &strcasecmp,
	(unsigned int) &strncasecmp,
	(unsigned int) &strncat,
	(unsigned int) &strncat_s,
	(unsigned int) &strncpy_s,
	(unsigned int) &user_free,
	(unsigned int) &user_malloc,
	(unsigned int) &user_memalign,
	(unsigned int) &LZ4_decompress_fast,
	(unsigned int) &lzo1x_decompress,
	(unsigned int) &sctrlPatchModule,
	(unsigned int) &sctrlModuleTextAddr,
	(unsigned int) &sctrlHENFindImportLib,
	(unsigned int) &sctrlSetCustomStartModule,
	(unsigned int) &sctrlSetStartModuleExtra,
	(unsigned int) &sctrlGetInitTextAddr,
	(unsigned int) &sctrlGetInitTextAddr,
};

extern int sceIoOpenDrmPatched;
extern int sceIoOpenAsyncDrmPatched;
extern int moduleLoaderJackass;
extern int vshCheckBootable;
extern int sceWlanGetSwitchStatePatched;
extern int sceIoDreadHidePatched;
extern int sceIoChstatHidePatched;
extern int sceIoGetstatHidePatched;
extern int sceIoRemoveHidePatched;
extern int sceIoRmdirHidePatched;
static const unsigned int __SystemCtrlPrivate_exports[20] __attribute__((section(".rodata.sceResident"))) = {
	0xA79FDC0E,
	0xE4A4B87A,
	0xE17B2908,
	0x06D9C179,
	0xEE9A4E71,
	0x74F62098,
	0xCAB8FF30,
	0xC6F7FE9C,
	0xD87C9F8D,
	0x6AC534EE,
	(unsigned int) &sceIoOpenDrmPatched,
	(unsigned int) &sceIoOpenAsyncDrmPatched,
	(unsigned int) &moduleLoaderJackass,
	(unsigned int) &vshCheckBootable,
	(unsigned int) &sceWlanGetSwitchStatePatched,
	(unsigned int) &sceIoDreadHidePatched,
	(unsigned int) &sceIoChstatHidePatched,
	(unsigned int) &sceIoGetstatHidePatched,
	(unsigned int) &sceIoRemoveHidePatched,
	(unsigned int) &sceIoRmdirHidePatched,
};

extern int user_malloc;
extern int user_free;
extern int oe_free;
extern int user_memalign;
extern int sprintf;
extern int snprintf;
extern int bcopy;
extern int bcmp;
extern int bzero;
extern int atob;
extern int toupper;
extern int tolower;
extern int memset;
extern int wmemset;
extern int memchr;
extern int memcmp;
extern int memmove;
extern int memcpy;
extern int strtoul;
extern int strncmp;
extern int strstr;
extern int strpbrk;
extern int strtok_r;
extern int strcat;
extern int strrchr;
extern int strlen;
extern int strspn;
extern int strcspn;
extern int strnlen;
extern int strchr;
extern int strncpy;
extern int strcmp;
extern int strcpy;
extern int strcasecmp;
extern int strncasecmp;
extern int strcasecmp;
extern int strncasecmp;
extern int lowerString;
extern int strtol10;
extern int strtol;
extern int strtok;
extern int strncat;
extern int strncat_s;
extern int strncpy_s;
extern int __udivdi3;
extern int __umoddi3;
extern int __udivmoddi4;
static const unsigned int __SysclibForUser_exports[94] __attribute__((section(".rodata.sceResident"))) = {
	0x4C0FE24C,
	0x12BC667F,
	0xA65E8BC4,
	0xB19C939A,
	0x7661E728,
	0xC2145E80,
	0x097049BD,
	0x7F8A6F23,
	0x86FEFCE9,
	0x1D83F344,
	0xCE2F7487,
	0x3EC5BBF6,
	0x10F3BB61,
	0x1493EBD9,
	0x68A78817,
	0x81D0D1F7,
	0xA48D2592,
	0xAB7592FF,
	0x6A7900E1,
	0x7AB35214,
	0x0D188658,
	0x0DFB7B6C,
	0x1AB53A58,
	0x476FD94A,
	0x4C0E0274,
	0x52DF196C,
	0x62AE052F,
	0x89B79CB1,
	0x90C5573D,
	0xB1DC2AE8,
	0xB49A7697,
	0xC0AB8932,
	0xEC6F1CF2,
	0xBC7554DF,
	0xAC7554DF,
	0x3EB35691,
	0x7BA27B01,
	0x983B00FB,
	0x37DD934D,
	0x47DD934D,
	0x87F8D2DA,
	0xD3D1A3B9,
	0xEFB593C9,
	0x5ABF13F5,
	0x7DEE14DE,
	0x5E8E5F42,
	0xDF17F4A2,
	(unsigned int) &user_malloc,
	(unsigned int) &user_free,
	(unsigned int) &oe_free,
	(unsigned int) &user_memalign,
	(unsigned int) &sprintf,
	(unsigned int) &snprintf,
	(unsigned int) &bcopy,
	(unsigned int) &bcmp,
	(unsigned int) &bzero,
	(unsigned int) &atob,
	(unsigned int) &toupper,
	(unsigned int) &tolower,
	(unsigned int) &memset,
	(unsigned int) &wmemset,
	(unsigned int) &memchr,
	(unsigned int) &memcmp,
	(unsigned int) &memmove,
	(unsigned int) &memcpy,
	(unsigned int) &strtoul,
	(unsigned int) &strncmp,
	(unsigned int) &strstr,
	(unsigned int) &strpbrk,
	(unsigned int) &strtok_r,
	(unsigned int) &strcat,
	(unsigned int) &strrchr,
	(unsigned int) &strlen,
	(unsigned int) &strspn,
	(unsigned int) &strcspn,
	(unsigned int) &strnlen,
	(unsigned int) &strchr,
	(unsigned int) &strncpy,
	(unsigned int) &strcmp,
	(unsigned int) &strcpy,
	(unsigned int) &strcasecmp,
	(unsigned int) &strncasecmp,
	(unsigned int) &strcasecmp,
	(unsigned int) &strncasecmp,
	(unsigned int) &lowerString,
	(unsigned int) &strtol10,
	(unsigned int) &strtol,
	(unsigned int) &strtok,
	(unsigned int) &strncat,
	(unsigned int) &strncat_s,
	(unsigned int) &strncpy_s,
	(unsigned int) &__udivdi3,
	(unsigned int) &__umoddi3,
	(unsigned int) &__udivmoddi4,
};

const struct _PspLibraryEntry __library_exports[14] __attribute__((section(".lib.ent"), used)) = {
	{ NULL, 0x0000, 0x8000, 4, 1, 1, (unsigned int *) &__syslib_exports },
	{ "LflashFatfmt", 0x0011, 0x0001, 4, 0, 1, (unsigned int *) &__LflashFatfmt_exports },
	{ "sceLFatFs_driver", 0x0011, 0x0001, 4, 0, 0, (unsigned int *) &__sceLFatFs_driver_exports },
	{ "sceDve_driver", 0x0011, 0x0001, 4, 0, 0, (unsigned int *) &__sceDve_driver_exports },
	{ "sceLed_driver", 0x0011, 0x0001, 4, 0, 0, (unsigned int *) &__sceLed_driver_exports },
	{ "sceCodec_driver", 0x0011, 0x0001, 4, 0, 0, (unsigned int *) &__sceCodec_driver_exports },
	{ "sceClockgen_driver", 0x0011, 0x0001, 4, 0, 0, (unsigned int *) &__sceClockgen_driver_exports },
	{ "sceNand_driver", 0x0011, 0x0001, 4, 0, 0, (unsigned int *) &__sceNand_driver_exports },
	{ "sceSyscon_driver", 0x0011, 0x0001, 4, 0, 0, (unsigned int *) &__sceSyscon_driver_exports },
	{ "KUBridge", 0x0000, 0x4001, 4, 0, 19, (unsigned int *) &__KUBridge_exports },
	{ "SystemCtrlForUser", 0x0000, 0x4001, 4, 0, 75, (unsigned int *) &__SystemCtrlForUser_exports },
	{ "SystemCtrlForKernel", 0x0000, 0x0001, 4, 0, 105, (unsigned int *) &__SystemCtrlForKernel_exports },
	{ "SystemCtrlPrivate", 0x0000, 0x4001, 4, 0, 10, (unsigned int *) &__SystemCtrlPrivate_exports },
	{ "SysclibForUser", 0x0000, 0x4001, 4, 0, 47, (unsigned int *) &__SysclibForUser_exports },
};
