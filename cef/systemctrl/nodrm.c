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

#include "main.h"
#include "pgd.h"

int (* do_open)(char *file, int flags, SceMode mode, int async, int retAddr, int oldK1);
int (* do_ioctl)(SceUID fd, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen, int async);

SceUID (* _sceKernelLoadModuleNpDrm)(const char *path, int flags, SceKernelLMOption *option);

int (* _sceNpDrmRenameCheck)(const char *file_name);
int (* _sceNpDrmEdataSetupKey)(SceUID fd);
int (* _sceNpDrmEdataGetDataSize)(SceUID fd);

static char ebootpath[256];
static char g_pgd_path[256];
static u8 pgdbuf[0x90];
static u8 g_eboot_key[16];
static int g_licensed_eboot = 0;
static int g_is_key = 0;

int IsPlainDrmFd(SceUID fd) {
	int k1 = pspSdkSetK1(0);

	int pos = sceIoLseek32(fd, 0, PSP_SEEK_CUR);
	sceIoLseek32(fd, 0, PSP_SEEK_SET);

	char buf[8];
	if (sceIoRead(fd, buf, sizeof(buf)) == sizeof(buf)) {
		sceIoLseek32(fd, pos, PSP_SEEK_SET);

		// Encrypted DRM file
		if (memcmp(buf, "\0PSPEDAT", 8) == 0 || memcmp(buf, "\0PGD", 4) == 0) {
			pspSdkSetK1(k1);
			return 0;
		}
	} else {
		// Read error
		pspSdkSetK1(k1);
		return -1;
	}

	// Plain DRM file
	pspSdkSetK1(k1);
	return 1;
}

int IsPlainDrmPath(const char *path) {
	int k1 = pspSdkSetK1(0);

	SceUID fd = sceIoOpen(path, PSP_O_RDONLY, 0);
	if (fd >= 0) {
		if (IsPlainDrmFd(fd)) {
			sceIoClose(fd);
			pspSdkSetK1(k1);
			return 1;
		}

		sceIoClose(fd);
	}

	pspSdkSetK1(k1);
	return 0;
}

int do_open_patched(char *file, int flags, SceMode mode, int async, int retAddr, int oldK1) {
	if (flags & 0x40000000) {
		strcpy(g_pgd_path, file);
		if (IsPlainDrmPath(file)) {
			flags &= ~0x40000000;
		}
	}

	return do_open(file, flags, mode, async, retAddr, oldK1);
}

int do_ioctl_patched(SceUID fd, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen, int async) {
	int res = do_ioctl(fd, cmd, indata, inlen, outdata, outlen, async);

	if (res < 0) {
		if (cmd == 0x04100001 || cmd == 0x04100002) {
			if (IsPlainDrmFd(fd)) {
				return 0;
			}
		}
	}

	return res;
}

SceUID sceKernelLoadModuleNpDrmPatched(const char *path, int flags, SceKernelLMOption *option) {
	int res = _sceKernelLoadModuleNpDrm(path, flags, option);

	if (res < 0) {
		if (IsPlainDrmPath(path)) {
			return sceKernelLoadModule(path, flags, option);
		}
	}

	return res;
}

int sceNpDrmRenameCheckPatched(const char *file_name) {
	int res = _sceNpDrmRenameCheck(file_name);

	if (res < 0) {
		if (IsPlainDrmPath(file_name)) {
			return 0;
		}
	}

	return res;
}

int sceNpDrmEdataSetupKeyPatched(SceUID fd) {
	int res = _sceNpDrmEdataSetupKey(fd);

	if (res < 0) {
		if (IsPlainDrmFd(fd)) {
			return 0;
		}
	}

	return res;
}

int sceNpDrmEdataGetDataSizePatched(SceUID fd) {
	int res = _sceNpDrmEdataGetDataSize(fd);

	if (res < 0) {
		if (IsPlainDrmFd(fd)) {
			int pos = sceIoLseek32(fd, 0, PSP_SEEK_CUR);
			int size = sceIoLseek32(fd, 0, PSP_SEEK_END);
			sceIoLseek32(fd, pos, PSP_SEEK_SET);
			return size;
		}
	}

	return res;
}

static u32 tou32(u8 *buf) {
	return (u32)(buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24));
}

static int is_licensed_eboot(const char *path) {
	int ret = 0;
	u8 buf[0x28];
	SceUID fd = sceIoOpen(path, PSP_O_RDONLY, 0);
	sceIoRead(fd, buf, sizeof(buf));

	if (!memcmp(buf, "\x00PBP", 4)) {
		sceIoLseek(fd, tou32(buf + 0x24), 0);
		sceIoRead(fd, buf, 0xC);

		if (!memcmp(buf, "NPUMDIMG", 8))
			ret = memcmp(buf + 8, "\x03\x00\x00\x01", 4); //disable patch if fixed key version is detected.
	}

	sceIoClose(fd);

	return ret;
}

//patches sceNpDrmEdataSetupKey and sceNpDrmGetModuleKey
int (* _setupEdatVersionKey)(u8 *vkey, u8 *edat, int size);
// variable EDAT/SPRX vkey per game, do not backup vkey.
int setupEdatVersionKeyPatched(u8 *vkey, u8 *edat, int size) {
	int ret = _setupEdatVersionKey(vkey, edat, size);

	if (ret < 0) { //generate key from mac if official method fails.
		ret = sceIoOpen(g_pgd_path, 1, 0);
		sceIoRead(ret, pgdbuf, 16);
		sceIoLseek(ret, tou32(pgdbuf + 0xC) & 0xFFFF, 0);
		sceIoRead(ret, pgdbuf, 0x90);
		sceIoClose(ret);

		ret = get_edat_key(vkey, pgdbuf);
	}

	return ret;
}

int (* _setupEbootVersionKey)(u8 *vkey, u8 *cid, u32 type, u8 *act);
int setupEbootVersionKeyPatched(u8 *vkey, u8 *cid, u32 type, u8 *act) {
	// Prevent SceNpUmdMount thread from crashing during suspend/resume process if rif/act.dat fails.
	if (g_is_key) {
		// Copy generated key from first call since there is only one version key per eboot.
		memcpy(vkey, g_eboot_key, 16);
		return 0;
	}

	int ret = _setupEbootVersionKey(vkey, cid, type, act);

	// Generate key from mac if official method fails.
	if (ret < 0) {
		ret = get_version_key(vkey, ebootpath);
	}

	if (ret >= 0) {
		// Backup eboot vkey for later calls.
		memcpy(g_eboot_key, vkey, 16);
		g_is_key = 1;
	}

	return ret;
}

SceModule2* g_np9660_mod = NULL;
void patch_drm() {
	u32 addr, data;

	for (addr = g_np9660_mod->text_addr; addr < (g_np9660_mod->text_addr + g_np9660_mod->text_size); addr += 4) {
		data = VREAD32(addr);

		if (data == 0x3C118021) { //lui        $s1, 0x8021
			// Patch memcmp, ensures the kernel continues to decrypt the eboot.
			MAKE_INSTRUCTION(addr - 4, 0x1021);
		} else if (data == 0x3C098055) { //lui        $t1, 0x8055
			HIJACK_FUNCTION(addr - 4, setupEbootVersionKeyPatched, _setupEbootVersionKey);
			break;
		}
	}

	SceModule2 *mod = sceKernelFindModuleByName("scePspNpDrm_Driver");

	for (addr = mod->text_addr; addr < (mod->text_addr + mod->text_size); addr += 4) {
		if (_lw(addr) == 0x2CC60080) { //sltiu      $a2, $a2, 128
			HIJACK_FUNCTION(addr - 8, setupEdatVersionKeyPatched, _setupEdatVersionKey);
			break;
		}
	}

	sctrlFlushCache();
}

int (* _initEboot)(const char *eboot, u32 a1) = NULL;
int initEbootPatched(const char *eboot, u32 a1) {
	if ((g_licensed_eboot = is_licensed_eboot(eboot))) {
		strcpy(ebootpath, eboot);
		patch_drm();
	}

	return _initEboot(eboot, a1);
}

void PatchNp9660Driver(SceModule2* mod) {
	// Do not patch if configured to not patch it
	if (config.no_nodrm_engine) {
		return;
	}

	g_np9660_mod = mod;

	// Do not patch in pops mode
	if (sceKernelApplicationType() == PSP_INIT_KEYCONFIG_POPS) {
		return;
	}

	// Prevent from patching again, should fix suspend issue in most cases.
	if (_initEboot != NULL) {
		return;
	}

	u32 jalcount = 0;
	for (u32 addr = mod->text_addr; addr < (mod->text_addr + mod->text_size); addr += 4) {
		u32 data = VREAD32(addr);

		if (IS_JAL(data) && (jalcount < 2)) {
			jalcount++;
		}

		if (jalcount == 2) {
			_initEboot = (void *)KERNELIFY(JUMP_TARGET(data));
			MAKE_CALL(addr, initEbootPatched);
			break;
		}
	}
}

void PatchNpDrmDriver(SceModule2* mod) {
	// Do not patch if configured to not patch it
	if (config.no_nodrm_engine) {
		return;
	}

	u32 text_addr = mod->text_addr;
	if (sceKernelBootFrom() == PSP_BOOT_DISC) {
		SceModule2 *mod = sceKernelFindModuleByName("sceIOFileManager");

		for (int i = 0; i < mod->text_size; i += 4) {
			u32 addr = mod->text_addr + i;
			u32 data = VREAD32(addr);

			if (data == 0x03641824) {
				HIJACK_FUNCTION(addr - 4, do_open_patched, do_open);
				continue;
			}

			if (data == 0x00C75821) {
				HIJACK_FUNCTION(addr - 4, do_ioctl_patched, do_ioctl);
				continue;
			}
		}

		HIJACK_FUNCTION(text_addr + 0x1590, sceNpDrmRenameCheckPatched, _sceNpDrmRenameCheck);
		HIJACK_FUNCTION(text_addr + 0x1714, sceNpDrmEdataSetupKeyPatched, _sceNpDrmEdataSetupKey);
		HIJACK_FUNCTION(text_addr + 0x1514, sceNpDrmEdataGetDataSizePatched, _sceNpDrmEdataGetDataSize);

		HIJACK_FUNCTION(FindProc("sceModuleManager", "ModuleMgrForUser", 0xF2D8D1B4), sceKernelLoadModuleNpDrmPatched, _sceKernelLoadModuleNpDrm);

		sctrlFlushCache();
	}
}

SceUID sceIoOpenDrmPatched(const char *path, int flags, SceMode mode) {
	if (flags & 0x40000000) {
		strcpy(g_pgd_path, path);
	}

	return sceIoOpen(path, flags, mode);
}

SceUID sceIoOpenAsyncDrmPatched(const char *path, int flags, SceMode mode) {
	if (flags & 0x40000000) {
		strcpy(g_pgd_path, path);
	}

	return sceIoOpenAsync(path, flags, mode);
}

void PatchDrmGameModule(SceModule2* mod) {
	// Do not patch if configured to not patch it
	if (config.no_nodrm_engine) {
		return;
	}

	if (!g_licensed_eboot) {
		return;
	}


	sctrlHENHookImportByNID(mod, "IoFileMgrForUser", 0x109F50BC, sceIoOpenDrmPatched,0);
	sctrlHENHookImportByNID(mod, "IoFileMgrForUser", 0x89AA9906, sceIoOpenAsyncDrmPatched,0);
}