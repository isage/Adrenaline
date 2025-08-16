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
#include "gameinfo.h"

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

int sctrlKernelBootFrom() {
	int k1 = pspSdkSetK1(0);
	int res = sceKernelBootFrom();
	pspSdkSetK1(k1);
	return res;
}

int sctrlHENIsSE() {
	return 1;
}

int sctrlHENIsToolKit() {
	return 0;
}

int sctrlHENIsDevhook() {
	return 0;
}

int sctrlHENGetVersion() {
	return 0x00001000;
}

int sctrlHENGetMinorVersion() {
	return ADRENALINE_VERSION;
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
	memset(rebootex_config.game_id, 0, 10);
	int res = _sceKernelExitVSH(param);
	pspSdkSetK1(k1);

	logmsg3("%s: param=0x%p -> 0x%08X\n", __func__, param, res);
	return res;
}

#if defined(DEBUG) && DEBUG >= 3
int (* _runExec)(RunExecParams* args) = NULL;
int runExecPatched(RunExecParams* args) {
	if (args->args == 0) {
		logmsg3("%s: [INFO]: apitype=0x%04lX, file=%s, param=0x%p\n", __func__, args->api_type, (char *)args->argp, args->vsh_param);
	} else {
		logmsg3("%s: [INFO]: apitype=0x%04lX, param=0x%p\n", __func__, args->api_type, args->vsh_param);
	}

	u32 k1 = pspSdkSetK1(0);

	if (!_runExec) {
		SceModule2 *mod = sceKernelFindModuleByName("sceLoadExec");
		_runExec = (void*) mod->text_addr + 0x2148;
	}

	int res = _runExec(args);

	pspSdkSetK1(k1);
	return res;
}
#endif // defined(DEBUG) && DEBUG >= 3

int (* _sceLoadExecVSHWithApitype)(int, const char*, SceKernelLoadExecVSHParam*, unsigned int) = NULL;
int sctrlKernelLoadExecVSHWithApitype(int apitype, const char *file, SceKernelLoadExecVSHParam *param) {
	logmsg3("%s: [INFO]: apitype=0x%04X, file=%s, param=0x%p\n", __func__, apitype, file, param);

	int k1 = pspSdkSetK1(0);

	if (!_sceLoadExecVSHWithApitype) {
		SceModule2 *mod = sceKernelFindModuleByName("sceLoadExec");
		u32 text_addr = mod->text_addr;
		_sceLoadExecVSHWithApitype = (void *)text_addr + 0x23D0;
	}

	// obtain game id
    u32 gameid_size = sizeof(rebootex_config.game_id);
    memset(rebootex_config.game_id, 0, gameid_size);
    if (apitype == SCE_EXEC_APITYPE_DISC || apitype == SCE_EXEC_APITYPE_DISC2){
        readGameIdFromDisc();
    } else {
        sctrlGetSfoPARAM(file, "DISC_ID", NULL, &gameid_size, rebootex_config.game_id);
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

void SetUmdFile(const char *file) __attribute__((alias("sctrlSESetUmdFile")));
void sctrlSESetUmdFile(const char *file) {
	strncpy(rebootex_config.umdfilename, file, 255);
}

void sctrlSESetUmdFileEx(const char *file, char *input) {
	if (input != NULL) {
		strncpy(input, rebootex_config.umdfilename, 255);
	}
	sctrlSESetUmdFile(file);
}


char *GetUmdFile(void) __attribute__((alias("sctrlSEGetUmdFile")));
char *sctrlSEGetUmdFile() {
	return rebootex_config.umdfilename;
}

char *sctrlSEGetUmdFileEx(char *input) {
	char* umdfilename = sctrlSEGetUmdFile();
	if (input != NULL) {
		sctrlSESetUmdFile(input);
	}
	return umdfilename;
}

int sctrlSEMountUmdFromFile(char *file, int noumd, int isofs) {
	int k1 = pspSdkSetK1(0);

	SetUmdFile(file);

	pspSdkSetK1(k1);
	return 0;
}

int sctrlSEUmountUmd() {
	return 0;
}

void sctrlSESetDiscOut(int out){
	return;
}

int sctrlSEGetBootConfBootFileIndex() {
	return rebootex_config.bootfileindex;
}

void sctrlSESetBootConfFileIndex(int index) {
	rebootex_config.bootfileindex = index;
}

int sctrlSEGetBootConfFileIndex() {
	return rebootex_config.bootfileindex;
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

extern u32 init_addr;
u32 sctrlGetInitTextAddr() {
	return init_addr;
}

void* sctrlHENGetInitControl() {
	return NULL;
}

void sctrlHENTakeInitControl(void* ictrl) {
	return;
}

int sctrlGetSfoPARAM(const char* sfo_path, const char* param_name, u16* param_type, u32* param_length, void* param_buf) {
	u32 k1 = pspSdkSetK1(0);

	if (param_name == NULL || param_name[0] == 0 || param_length == NULL) {
		pspSdkSetK1(k1);
		return SCE_ERR_INARG;
	}

	if (*param_length <= 0) {
		pspSdkSetK1(k1);
		return SCE_ERR_INSIZE;
	}

	u32 param_offset = 0;
	if (sfo_path == NULL) {
		sfo_path = sceKernelInitFileName();

		if (sfo_path == NULL) {
			pspSdkSetK1(k1);
			return SCE_ERR_NOENT;
		}

		if (strncmp(sfo_path, "disc0", 5) == 0) {
			sfo_path = "disc0:/PSP_GAME/PARAM.SFO";
		}
	}

	int fd = sceIoOpen(sfo_path, PSP_O_RDONLY, 0);

	if (fd < 0) {
		pspSdkSetK1(k1);
		return SCE_ERR_NOENT;
	}

	int magic = 0;
	sceIoRead(fd, &magic, sizeof(magic));

	if (magic == PBP_MAGIC){
		sceIoLseek(fd, 0x08, PSP_SEEK_SET);
		sceIoRead(fd, &param_offset, sizeof(u32));
	} else if (magic != SFO_MAGIC){ // Invalid Format - FSP
		sceIoClose(fd);
		pspSdkSetK1(k1);
		return SCE_ERR_NOENT;
	}

	// seek to PARAM.SFO offset
	sceIoLseek(fd, param_offset, PSP_SEEK_SET);

	// seek to key table offset variable
	sceIoLseek(fd, 0x08, PSP_SEEK_CUR);

	// read variables of interest
	u32 key_table_offset = 0;
	u32 data_table_offset = 0;
	u32 entry_count = 0;
	sceIoRead(fd, &key_table_offset, sizeof(key_table_offset));
	sceIoRead(fd, &data_table_offset, sizeof(data_table_offset));
	sceIoRead(fd, &entry_count, sizeof(entry_count));

	// iterate entries
	u32 entry;
	for (entry = 0; entry < entry_count; entry++) {
		// read variables of interest
		u16 entry_key_offset = 0;
		u16 entry_format = 0;
		u32 entry_used_length = 0;
		u32 entry_full_length = 0;
		u32 entry_data_offset = 0;
		sceIoRead(fd, &entry_key_offset, sizeof(entry_key_offset));
		sceIoRead(fd, &entry_format, sizeof(entry_format));
		sceIoRead(fd, &entry_used_length, sizeof(entry_used_length));
		sceIoRead(fd, &entry_full_length, sizeof(entry_full_length));
		sceIoRead(fd, &entry_data_offset, sizeof(entry_data_offset));

		// save offset for next entry
		SceOff next_entry_offset = sceIoLseek(fd, 0, PSP_SEEK_CUR);

		// move to key name
		sceIoLseek(fd, param_offset + key_table_offset + entry_key_offset, PSP_SEEK_SET);

		// read key name
		char key_name[128];
		memset(key_name, 0, sizeof(key_name));
		char symbol = 0;
		while (sceIoRead(fd, &symbol, sizeof(symbol)) > 0) {
			key_name[strlen(key_name)] = symbol;
			if (symbol == 0) {
				break;
			}
		}

		if (strcmp(key_name, param_name) == 0) {
			u32 required_length = entry_used_length;

			if (entry_format == 0x0004) {
				required_length = entry_used_length + 1;
			}

			if (param_buf != NULL && *param_length < required_length) {
				sceIoClose(fd);
				pspSdkSetK1(k1);
				return SCE_ERR_INSIZE;
			}

			if (param_length != NULL) {
				*param_length = required_length;
			}

			if (param_type != NULL) {
				*param_type = entry_format;
			}

			// move to entry data
			sceIoLseek(fd, param_offset + data_table_offset + entry_data_offset, PSP_SEEK_SET);

			if (param_buf != NULL) {
				// Reset buffer (also serves as termination of strings)
				memset(param_buf, 0, *param_length);

				sceIoRead(fd, param_buf, entry_used_length);
			}

			sceIoClose(fd);
			pspSdkSetK1(k1);
			return 0;
		}

		// Resume processing at next entry
		sceIoLseek(fd, next_entry_offset, PSP_SEEK_SET);
	}

	sceIoClose(fd);
	pspSdkSetK1(k1);

	// Return Error Code (we just treat a missing parameter as file not found, it should work)
	return SCE_ERR_NOENT;
}

int sctrlGetInitPARAM(const char* param_name, u16* param_type, u32* param_length, void* param_buf) {
	return sctrlGetSfoPARAM(NULL, param_name, param_type, param_length, param_buf);
}

SceUID sctrlGetThreadUIDByName(const char* name) {
	if (name == NULL) {
		return SCE_EINVAL;
	}

	SceUID ids[100];
	memset(ids, 0, sizeof(ids));

	int count = 0;

	int res = sceKernelGetThreadmanIdList(SCE_KERNEL_TMID_Thread, ids, NELEMS(ids), &count);

	if (res < 0) {
		return res;
	}

	for (int i = 0; i < count; i++) {
		SceKernelThreadInfo info = {0};
		info.size = sizeof(info);

		res = sceKernelReferThreadStatus(ids[i], &info);
		if (res != 0) {
			return res;
		}

		// Check if name match
		if (strcmp(info.name, name) == 0) {
			return ids[i];
		}
	}

	return SCE_ERR_NOTFOUND;
}