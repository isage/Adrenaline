/*
	Adrenaline
	Copyright (C) 2016-2018, TheFloW

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

#include <common.h>
#include <pspintrman.h>

#include <adrenaline_log.h>

#include "main.h"
#include "adrenaline.h"

int lzo1x_decompress(void* source, unsigned src_len, void* dest, unsigned* dst_len, void*);
int LZ4_decompress_fast(const char* source, char* dest, int outputSize);


int sctrlKernelSetUserLevel(int level) {
	int k1 = pspSdkSetK1(0);
	int res = sceKernelGetUserLevel();

	SceModule2 *mod = sceKernelFindModuleByName("sceThreadManager");
	u32 text_addr = mod->text_addr;

	u32 high = (((u32)VREAD16(text_addr + 0x358)) << 16);
	u32 low = ((u32)VREAD16(text_addr + 0x35C));

	if (low & 0x8000)
		high -= 0x10000;

	u32 *thstruct = (u32 *)VREAD32(high | low);
	thstruct[0x14/4] = (level ^ 8) << 28;

	pspSdkSetK1(k1);
	return res;
}

int sctrlKernelSetDevkitVersion(int version) {
	u32 k1 = pspSdkSetK1(0);

	int prev_ver = sceKernelDevkitVersion();

	// Overwrite version
	u32 devkit_version = sctrlHENFindFunction("sceSystemMemoryManager", "SysMemForKernel", 0xC886B169);
	VWRITE16(devkit_version, (version >> 16));
	VWRITE16(devkit_version+8, (version & 0xFFFF));

	sctrlFlushCache();
	pspSdkSetK1(k1);

	return prev_ver;
}

int sctrlKernelSetInitApitype(int apitype) {
	int k1 = pspSdkSetK1(0);
	SceInit* init = sceKernelQueryInitCB();

	if (init == NULL) {
		return SCE_EFAULT;
	}

	int prev_value = init->apitype;
	init->apitype = apitype;

	pspSdkSetK1(k1);
	return prev_value;
}

char g_init_filename[255] = {0};
// The same as sctrlKernelSetUMDEmuFile
int sctrlKernelSetInitFileName(char * filename) {
	if(filename == NULL) {
		return SCE_EINVAL;
	}

	int k1 = pspSdkSetK1(0);

	SceInit* init = sceKernelQueryInitCB();

	if (init == NULL) {
		return SCE_EFAULT;
	}

	if (init->file_mod_addr != NULL) {
		logmsg4("%s: [DEBUG]: Previous filename=%s\n", __func__, *(const char**)(init->file_mod_addr));
	} else {
		logmsg4("%s: [DEBUG]: Previous filename=<NUL>\n", __func__);
	}

	strncpy_s(g_init_filename, 255, filename, strlen(filename));
	init->file_mod_addr = &g_init_filename;

	pspSdkSetK1(k1);
	return 0;
}

int sctrlKernelSetInitKeyConfig(int key) {
	int k1 = pspSdkSetK1(0);

	SceInit* init = sceKernelQueryInitCB();
	int prev_value = init->application_type;

	init->application_type = key;

	pspSdkSetK1(k1);
	return prev_value;
}

int sctrlHENIsSE() {
	return 1;
}

int sctrlHENIsDevhook() {
	return 0;
}

int sctrlHENGetVersion() {
	return 0x00001000;
}

int sctrlSEGetVersion() {
	return ADRENALINE_VERSION;
}

PspIoDrv *sctrlHENFindDriver(char *drvname) {
	int k1 = pspSdkSetK1(0);

	SceModule2 *mod = sceKernelFindModuleByName("sceIOFileManager");
	u32 text_addr = mod->text_addr;

	u32 *(* GetDevice)(char *) = NULL;

	for (int i = 0; i < mod->text_size; i += 4) {
		u32 addr = text_addr + i;
		u32 data = VREAD32(addr);

		if (data == 0xA2200000) {
			GetDevice = (void *)K_EXTRACT_CALL(addr + 4);
			break;
		}
	}

	if (!GetDevice) {
		pspSdkSetK1(k1);
		return 0;
	}

	u32 *u = GetDevice(drvname);
	if (!u) {
		pspSdkSetK1(k1);
		return 0;
	}

	pspSdkSetK1(k1);

	return (PspIoDrv *)u[1];
}

int (*_sceKernelExitVSH)(void*) = (void*)sceKernelExitVSHVSH;
int sctrlKernelExitVSH(SceKernelLoadExecVSHParam *param) {
	int k1 = pspSdkSetK1(0);
	int res = _sceKernelExitVSH(param);
	pspSdkSetK1(k1);

	logmsg3("%s: param=0x%p -> 0x%08X\n", __func__, param, res);
	return res;
}

int (* _sceLoadExecVSHWithApitype)(int, const char*, SceKernelLoadExecVSHParam*, unsigned int) = NULL;
int sctrlKernelLoadExecVSHWithApitype(int apitype, const char *file, SceKernelLoadExecVSHParam *param) {
	logmsg3("%s: [INFO]: apitype=0x%04X, file=%s, param=0x%p\n", __func__, apitype, file, param);

	int k1 = pspSdkSetK1(0);

	if (!_sceLoadExecVSHWithApitype) {
		SceModule2 *mod = sceKernelFindModuleByName("sceLoadExec");
		u32 text_addr = mod->text_addr;
		_sceLoadExecVSHWithApitype = (void *)text_addr + 0x23D0;
	}

	int res = _sceLoadExecVSHWithApitype(apitype, file, param, 0x10000);
	pspSdkSetK1(k1);

	logmsg3("%s: apitype=0x%04X, file=%s, param=0x%p -> 0x%08X\n", __func__, apitype, file, param, res);
	return res;
}

int sctrlKernelLoadExecVSHMs1(const char *file, SceKernelLoadExecVSHParam *param) {
	return sctrlKernelLoadExecVSHWithApitype(SCE_EXEC_APITYPE_MS1, file, param);
}

int sctrlKernelLoadExecVSHMs2(const char *file, SceKernelLoadExecVSHParam *param) {
	return sctrlKernelLoadExecVSHWithApitype(SCE_EXEC_APITYPE_MS2, file, param);
}

int sctrlKernelLoadExecVSHMs3(const char *file, SceKernelLoadExecVSHParam *param) {
	return sctrlKernelLoadExecVSHWithApitype(SCE_EXEC_APITYPE_MS3, file, param);
}

int sctrlKernelLoadExecVSHMs4(const char *file, SceKernelLoadExecVSHParam *param) {
	return sctrlKernelLoadExecVSHWithApitype(SCE_EXEC_APITYPE_MS4, file, param);
}

int sctrlKernelLoadExecVSHMs5(const char *file, SceKernelLoadExecVSHParam *param) {
	return sctrlKernelLoadExecVSHWithApitype(SCE_EXEC_APITYPE_MS5, file, param);
}

int sctrlKernelLoadExecVSHEf1(const char *file, SceKernelLoadExecVSHParam *param) {
	return sctrlKernelLoadExecVSHWithApitype(SCE_EXEC_APITYPE_EF1, file, param);
}

int sctrlKernelLoadExecVSHEf2(const char *file, SceKernelLoadExecVSHParam *param) {
	return sctrlKernelLoadExecVSHWithApitype(SCE_EXEC_APITYPE_EF2, file, param);
}

int sctrlKernelLoadExecVSHEf3(const char *file, SceKernelLoadExecVSHParam *param) {
	return sctrlKernelLoadExecVSHWithApitype(SCE_EXEC_APITYPE_EF3, file, param);
}

int sctrlKernelLoadExecVSHEf4(const char *file, SceKernelLoadExecVSHParam *param) {
	return sctrlKernelLoadExecVSHWithApitype(SCE_EXEC_APITYPE_EF4, file, param);
}

int sctrlKernelLoadExecVSHEf5(const char *file, SceKernelLoadExecVSHParam *param) {
	return sctrlKernelLoadExecVSHWithApitype(SCE_EXEC_APITYPE_EF5, file, param);
}

int sctrlKernelLoadExecVSHDisc(const char *file, SceKernelLoadExecVSHParam *param) {
	return sctrlKernelLoadExecVSHWithApitype(SCE_EXEC_APITYPE_DISC, file, param);
}

int sctrlKernelLoadExecVSHDiscUpdater(const char *file, SceKernelLoadExecVSHParam *param) {
	return sctrlKernelLoadExecVSHWithApitype(SCE_EXEC_APITYPE_DISC_UPDATER, file, param);
}

int sctrlKernelQuerySystemCall(void *function) {
	int k1 = pspSdkSetK1(0);
	int res = sceKernelQuerySystemCall(function);
	pspSdkSetK1(k1);
	return res;
}

void sctrlHENPatchSyscall(u32 addr, void *newaddr) {
	void *ptr = NULL;
	asm("cfc0 %0, $12\n" : "=r"(ptr));

	if (NULL == ptr) {
		return;
	}

	u32 *syscalls = (u32 *)(ptr + 0x10);

	for (int i = 0; i < 0x1000; i++) {
		if ((syscalls[i] & 0x0FFFFFFF) == (addr & 0x0FFFFFFF)) {
			syscalls[i] = (u32)newaddr;
		}
	}
}

void SetUmdFile(char *file) __attribute__((alias("sctrlSESetUmdFile")));
void sctrlSESetUmdFile(char *file) {
	strncpy(rebootex_config.umdfilename, file, 255);
}


char *GetUmdFile(void) __attribute__((alias("sctrlSEGetUmdFile")));
char *sctrlSEGetUmdFile() {
	return rebootex_config.umdfilename;
}

int sctrlSEMountUmdFromFile(char *file, int noumd, int isofs) {
	int k1 = pspSdkSetK1(0);

	SetUmdFile(file);

	pspSdkSetK1(k1);
	return 0;
}

int sctrlSEGetBootConfBootFileIndex() {
	return rebootex_config.bootfileindex;
}

void sctrlSESetBootConfFileIndex(int index) {
	rebootex_config.bootfileindex = index;
}

void sctrlSESetDiscType(int type) {
	rebootex_config.iso_disc_type = type;
}

int sctrlSEGetDiscType(void) {
	return rebootex_config.iso_disc_type;
}

void sctrlHENLoadModuleOnReboot(char *module_after, void *buf, int size, int flags) {
	rebootex_config.module_after = module_after;
	rebootex_config.buf = buf;
	rebootex_config.size = size;
	rebootex_config.flags = flags;
}

int sctrlGetUsbState() {
	return SendAdrenalineCmd(ADRENALINE_VITA_CMD_GET_USB_STATE);
}

int sctrlStartUsb() {
	return SendAdrenalineCmd(ADRENALINE_VITA_CMD_START_USB);
}

int sctrlStopUsb() {
	return SendAdrenalineCmd(ADRENALINE_VITA_CMD_STOP_USB);
}

int sctrlRebootDevice() {
	// can't do it separately, because user might have old systemctrl
	// but this is used only by updater, so that's ok
	SendAdrenalineCmd(ADRENALINE_VITA_CMD_UPDATE);
	return SendAdrenalineCmd(ADRENALINE_VITA_CMD_POWER_REBOOT);
}

u32 sctrlKernelRand(void) {
	u32 k1 = pspSdkSetK1(0);

	unsigned char * alloc = oe_malloc(20 + 4);

	// Allocation Error
	if(alloc == NULL) __asm__ volatile ("break");

	// Align Buffer to 4 Bytes
	unsigned char * buffer = (void *)(((u32)alloc & (~(4-1))) + 4);

	// KIRK Random Generator Opcode
	enum {
		KIRK_PRNG_CMD=0xE,
	};

	// Create 20 Random Bytes
	sceUtilsBufferCopyWithRange(buffer, 20, NULL, 0, KIRK_PRNG_CMD);

	u32 random = *(u32 *)buffer;

	oe_free(alloc);
	pspSdkSetK1(k1);

	return random;
}


int sctrlDeflateDecompress(void* dest, void* src, int size){
	u32 k1 = pspSdkSetK1(0);
	int ret = sceKernelDeflateDecompress(dest, size, src, 0);
	pspSdkSetK1(k1);
	return ret;
}

int sctrlGzipDecompress(void* dest, void* src, int size){
	u32 k1 = pspSdkSetK1(0);
	int ret = sceKernelGzipDecompress(dest, size, src, 0);
	pspSdkSetK1(k1);
	return ret;
}

int sctrlLZ4Decompress(void* dest, const void* src, int size) {
	return LZ4_decompress_fast(src, dest, size);
}

int sctrlLzoDecompress(void* dest, unsigned* dst_size, void* src, unsigned src_size) {
	return lzo1x_decompress(src, src_size, dest, dst_size, 0);
}

// init.prx Custom sceKernelStartModule Handler
extern int (* custom_start_module_handler)(int modid, SceSize argsize, void * argp, int * modstatus, SceKernelSMOption * opt);

void sctrlSetCustomStartModule(int (* func)(int modid, SceSize argsize, void * argp, int * modstatus, SceKernelSMOption * opt)) {
	custom_start_module_handler = func;
}

void* sctrlSetStartModuleExtra(int (* func)(int modid, SceSize argsize, void * argp, int * modstatus, SceKernelSMOption * opt)) {
	void* ret = custom_start_module_handler;
	custom_start_module_handler = func;
	return ret;
}