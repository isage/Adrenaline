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

#define _ADRENALINE_LOG_IMPL_
#include <adrenaline_log.h>

#include "main.h"

PSP_MODULE_INFO("M33PopcornManager", 0x1007, 1, 0);

int (* scePopsManExitVSHKernel)(int error);
int (* SetVersionKeyContentId)(char *file, u8 *version_key, char *content_id);

u8 pgd_buf[0x80];
int original = 0;

STMOD_HANDLER previous;

int scePopsManExitVSHKernelPatched(u32 destSize, u8 *src, u8 *dest) {
	if (destSize & 0x80000000) {
		return scePopsManExitVSHKernel(destSize);
	}

	int size = sceKernelDeflateDecompress(dest, destSize, src, 0);
	return (size == 0x9300) ? 0x92FF : size;
}

SceUID sceIoOpenPatched(const char *file, int flags, SceMode mode) {
	// Remove drm flag
	SceUID res = sceIoOpen(file, flags & ~0x40000000, mode);

	logmsg2("%s: %s 0x%08X -> 0x%08X\r\n", __func__, file, flags, res);
	return res;
}

int sceIoIoctlPatched(SceUID fd, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen) {
	// Seek
	if (cmd == 0x04100002) {
		sceIoLseek(fd, *(u32 *)indata, PSP_SEEK_SET);
	}

	return 0;
}

int sceIoReadPatched(SceUID fd, void *data, SceSize size) {
	int res = sceIoRead(fd, data, size);

	// Fake ~PSP magic to avoid crash
	if (size == sizeof(u32)) {
		u32 magic = ELF_MAGIC;
		if (memcmp(data, &magic, sizeof(u32)) == 0) {
			magic = PSP_MAGIC;
			memcpy(data, &magic, sizeof(u32));
			logmsg3("%s: Faked PSP magic\n");
		}
	}

	logmsg2("%s: fd=0x%08X data=0x%08X size=%d -> 0x%08X\n", __func__, fd, data, size, res);
	return res;
}

// PGD decryption by Hykem
// https://github.com/Hykem/psxtract/blob/master/Linux/crypto.c

int sceUtilsBufferCopyWithRange(void *inbuf, SceSize insize, void *outbuf, int outsize, int cmd);

int kirk7(u8 *buf, int size, int type) {
	u32 *header = (u32 *)buf;

	header[0] = 5;
	header[1] = 0;
	header[2] = 0;
	header[3] = type;
	header[4] = size;

	return sceUtilsBufferCopyWithRange(buf, size + KIRK7_HEADER_SIZE, buf, size, 7);
}

int GetVersionKeyContentIdPatched(char *file, u8 *version_key, char *content_id) {
	u8 dummy_version_key[VERSION_KEY_SIZE];
	char dummy_content_id[CONTENT_ID_SIZE];

	if (!version_key) {
		version_key = dummy_version_key;
	}

	if (!content_id) {
		content_id = dummy_content_id;
	}

	memset(version_key, 0, VERSION_KEY_SIZE);
	memset(content_id, 0, CONTENT_ID_SIZE);

	if (original) {
		// Set mac type
		int mac_type = 0;

		if (((u32 *)pgd_buf)[2] == 1) {
			mac_type = 1;

			if (((u32 *)pgd_buf)[1] > 1) {
				mac_type = 3;
			}
		} else {
			mac_type = 2;
		}

		// Generate the key from MAC 0x70
		MAC_KEY mac_key;
		sceDrmBBMacInit(&mac_key, mac_type);
		sceDrmBBMacUpdate(&mac_key, pgd_buf, 0x70);

		u8 xor_keys[VERSION_KEY_SIZE];
		sceDrmBBMacFinal(&mac_key, xor_keys, NULL);

		u8 kirk_buf[VERSION_KEY_SIZE + KIRK7_HEADER_SIZE];

		if (mac_key.type == 3) {
			memcpy(kirk_buf + KIRK7_HEADER_SIZE, pgd_buf + 0x70, VERSION_KEY_SIZE);
			kirk7(kirk_buf, VERSION_KEY_SIZE, 0x63);
		} else {
			memcpy(kirk_buf, pgd_buf + 0x70, VERSION_KEY_SIZE);
		}

		memcpy(kirk_buf + KIRK7_HEADER_SIZE, kirk_buf, VERSION_KEY_SIZE);
		kirk7(kirk_buf, VERSION_KEY_SIZE, (mac_key.type == 2) ? 0x3A : 0x38);

		// Get version key
		for (int i = 0; i < VERSION_KEY_SIZE; i++) {
			version_key[i] = xor_keys[i] ^ kirk_buf[i];
		}
	}

	return SetVersionKeyContentId(file, version_key, content_id);
}

int (* setCompiledSdkVersion)(u32 ver) = NULL;
static void setPsxCompiledFwVersion(u32 version) {
	if (NULL == setCompiledSdkVersion) {
		setCompiledSdkVersion = (void*)sctrlHENFindFunction("sceSystemMemoryManager", "SysMemUserForUser", 0x358CA1BB);
	}

	setCompiledSdkVersion(version);

}

static void patchScePopsMgr(void) {
	SceModule2 *mod = sceKernelFindModuleByName("scePops_Manager");
	u32 text_addr = mod->text_addr;

	// Use different mode for SceKermitPocs
	VWRITE32(text_addr + 0x2030, 0x2405000E);
	VWRITE32(text_addr + 0x20F0, 0x2405000E);
	VWRITE32(text_addr + 0x21A0, 0x2405000E);

	// Use different pops register location
	VWRITE32(text_addr + 0x11B4, 0x3C014BCD);

	// Patch key function. With this, KEYS.BIN or license files are not required anymore.
	// Also this gives support to custom PSone games.
	SetVersionKeyContentId = (void *)text_addr + 0x124; // Is this `sceNpDrmSetLicenseeKey`?
	REDIRECT_FUNCTION(text_addr + 0x14FC, GetVersionKeyContentIdPatched);
	// sctrlHENHookImportByNID(mod, "scePspNpDrm_driver",0x0F9547E6, GetVersionKeyContentIdPatched, 0);

	if (!original) {
		// Patch syscall to use it as deflate decompress
		scePopsManExitVSHKernel = (void *)sctrlHENFindFunction("scePops_Manager", "scePopsMan", 0x0090B2C8);
		sctrlHENPatchSyscall((u32)scePopsManExitVSHKernel, scePopsManExitVSHKernelPatched);

		// Fake dnas drm
		sctrlHENHookImportByNID(mod, "IoFileMgrForKernel", 0x109F50BC, sceIoOpenPatched, 0);
		sctrlHENHookImportByNID(mod, "IoFileMgrForKernel", 0x63632449, sceIoIoctlPatched, 0);
		sctrlHENHookImportByNID(mod, "IoFileMgrForKernel", 0x6A638D83, sceIoReadPatched, 0);

		// Dummying amctrl decryption functions
		MAKE_DUMMY_FUNCTION(text_addr + 0xA90, 1);
		MAKE_NOP(text_addr + 0x53C)
	}

	sctrlFlushCache();
}

int OnModuleStart(SceModule2 *mod) {
	if (strcmp(mod->modname, "pops") == 0) {
		// Use different pops register location
		for (u32 i = 0; i < mod->text_size; i += 4) {
			if ((VREAD32(mod->text_addr+i) & 0xFFE0FFFF) == 0x3C0049FE) {
				VWRITE16(mod->text_addr+i, 0x4BCD);
			}
		}

		if (!original) {
			// Use our decompression function
			VWRITE32(mod->text_addr + 0xC99C, VREAD32(mod->text_addr + 0xC69C));

			// Fix index length. This enables CDDA support
			VWRITE32(mod->text_addr + 0x164E4, 0x10000014);
		}

		sctrlFlushCache();
	}

	if (!previous){
		return 0;
	}

	return previous(mod);
}

int module_start(SceSize args, void *argp) {
	logInit("ms0:/log_popcorn.txt");
	logmsg("Popcorn started...\n");

	SceUID fd = sceIoOpen(sceKernelInitFileName(), PSP_O_RDONLY, 0);
	if (fd < 0) {
		return fd;
	}

	// Read header
	PBPHeader header;
	sceIoRead(fd, &header, sizeof(PBPHeader));

	// Get magic
	char magic[16];
	sceIoLseek(fd, header.psar_offset, PSP_SEEK_SET);
	sceIoRead(fd, magic, sizeof(magic));

	if (memcmp(magic, "PSISOIMG0000", 12) == 0) { // Single-Disc
		sceIoLseek(fd, header.psar_offset + 0x400, PSP_SEEK_SET);
	} else if (memcmp(magic, "PSTITLEIMG000000", 16) == 0) { // Multi-Disc
		sceIoLseek(fd, header.psar_offset + 0x200, PSP_SEEK_SET);
	} else {
		sceIoClose(fd);
		return 1;
	}

	// Read PGD buffer
	sceIoRead(fd, pgd_buf, sizeof(pgd_buf));

	// Close fd
	sceIoClose(fd);

	// Check PGD magic
	if (((u32 *)pgd_buf)[0] == PGD_MAGIC) {
		original = 1;
	}

	setPsxCompiledFwVersion(0x06060110);

	previous = sctrlHENSetStartModuleHandler(OnModuleStart);

	patchScePopsMgr();

	return 0;
}