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

PSP_MODULE_INFO("EPIPopcornManager", 0x1007, 1, 0);

int (* _scePopsManExitVSHKernel)(int error);
int (* SetVersionKeyContentId)(char *file, u8 *version_key, char *content_id);

static u8 pgd_buf[0x80];
// If the launched software is official (0 = CUSTOM, 1 = OFFICIAL)
static int is_official = 0;

// Custom emulator config
static u8 custom_config[0x400];
// Custom emulator config size;
static int config_size = 0;
// pops supports up to 5 discs, but config is the same for all of them even if
// it doesn't have to
static int psiso_offsets[5] = {0, 0, 0, 0, 0};


STMOD_HANDLER previous;

int scePopsManExitVSHKernelPatched(u32 destSize, u8 *src, u8 *dest) {
	if (destSize & 0x80000000) {
		logmsg3("%s: error=0x%08lX", __func__, destSize);
		return _scePopsManExitVSHKernel(destSize);
	}

	int size = sceKernelDeflateDecompress(dest, destSize, src, 0);

	int ret;
	if (size == 0x9300) {
		ret = 0x92FF;
		logmsg4("%s: [FAKE] return value -> 0x%08lX\n", __func__, ret);
	} else {
		ret = size;
	}

	logmsg3("%s: DeflateDecompress destSize=0x%08X, src=0x%08X, dest=0x%08X -> 0x%08X\n",__func__, (uint)destSize, (uint)src, (uint)dest, ret);
	return ret;
}

static int (*_sceMeAudio_2AB4FE43)(void *buf, int size) = NULL;
int sceMeAudio_2AB4FE43_Patched(void *buf, int size) {
	if (NULL == _sceMeAudio_2AB4FE43) {
		logmsg("%s: [ERROR]: Pointer to original function was not set\n", __func__);
		// Illegal addr error
		return 0x800200d3;
	}

	u32 k1 = pspSdkSetK1(0);
	int ret = _sceMeAudio_2AB4FE43(buf, size);
	pspSdkSetK1(k1);

	logmsg3("%s: buf=0x%p, size=0x%08X -> 0x%08X", __func__, buf, size, ret);
	return ret;
}

SceUID sceIoOpenPatched(const char *file, int flags, SceMode mode) {
	// Remove drm flag
	SceUID res = sceIoOpen(file, flags & ~0x40000000, mode);

	logmsg3("%s: file=%s, flags=0x%08X -> 0x%08X\n", __func__, file, flags, res);
	return res;
}

int sceIoIoctlPatched(SceUID fd, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen) {
	int ret = 0;

	if (cmd == 0x04100002) { // Seek
		ret = sceIoLseek(fd, *(u32 *)indata, PSP_SEEK_SET);

		if (ret < 0) {
			logmsg("%s: [ERROR] sceIoLseek -> 0x%08X\n", __func__, ret);
		}
		ret = 0;

		logmsg4("%s: [FAKE] fd=0x%08X, cmd=0x%08X -> 0x%08X\n", __func__, fd, cmd, ret);
	} else {
		// ret = sceIoIoctl(fd, cmd, indata, inlen, outdata, outlen);
		ret = 0;
		logmsg4("%s: [FAKE] fd=0x%08X, cmd=0x%08X -> 0x%08X\n", __func__, fd, cmd, ret);
	}

	logmsg3("%s: fd=0x%08X, cmd=0x%08X -> 0x%08X\n", __func__, fd, cmd, ret);
	return ret;
}

int sceIoReadPatched(SceUID fd, u8 *data, SceSize size) {
	u32 k1 = pspSdkSetK1(0);
	u32 pos = sceIoLseek32(fd, 0, SEEK_CUR);
	int res = sceIoRead(fd, data, size);

	// Inject custom config and apply anti-libcrypt patch
	for (int i = 0; i < NELEMS(psiso_offsets); i++) {
		if (psiso_offsets[i] == 0) {
			break;
		}

		// Emulator reads a huge chunk of data starting at PSISOIMG+0x400
		// More information about PSISOIMG: https://www.psdevwiki.com/psp/PSISOIMG0000
		u32 huge_chunk_read_pos = psiso_offsets[i]+0x400;

		// If not the read we expect, go to the next possibility
		if (pos != huge_chunk_read_pos) {
			continue;
		}

		// Seek to where PSISOIMG0000 magic is expected to appear and read it
		char magic[12];
		sceIoLseek(fd, psiso_offsets[i], PSP_SEEK_SET);
		sceIoRead(fd, magic, sizeof(magic));
		// Seek back into where file cursor should be.
		sceIoLseek(fd, pos+size, PSP_SEEK_SET);

		// Magic is correct
		if (memcmp(magic, "PSISOIMG0000", 12) == 0) {
			// Copy custom config if it exists
			if (config_size > 0) {
				// It is located at 0x420 after PSISOIMG, thus 0x20 after given buffer
				memcpy(data+0x20, custom_config, config_size);
				logmsg2("%s: [INFO]: Custom config was set.\n", __func__);
			}

			// anti-libcrypt patch, calculate libcrypt magic and inject at 0x12B0 after PSISOIMG, 0xEB0 after given buffer
			// buf points to PSISOIMG+0x0400, which conviniently starts with the disc_id
			u32 libcrypt_magic = searchLibCryptMagicWord(data);

			// A magic word for this title was found
			if (libcrypt_magic != 0) {
				// It needs to be xored with this constant
				libcrypt_magic ^= LIBCRYPT_XOR_MAGIC;
				memcpy(data+0xeb0, &libcrypt_magic, sizeof(libcrypt_magic));
				logmsg2("%s: [INFO]: Anti-libcrypt patch was applied.\n", __func__);
			}
		}
	}

	if (res != size) {
		goto exit;
	}

	if (!is_official && size >= 0x420 && data[0x41B] == 0x27 && data[0x41C] == 0x19 && data[0x41D] == 0x22 && data[0x41E] == 0x41 && data[0x41A] == data[0x41F]) {
		data[0x41B] = 0x55;
		logmsg3("%s: [INFO]: Unknown patch loc_6c\n", __func__);
	}

	// Fake ~PSP magic to avoid crash
	if (size == sizeof(u32)) {
		u32 magic = ELF_MAGIC;
		if (memcmp(data, &magic, sizeof(u32)) == 0) {
			magic = PSP_MAGIC;
			memcpy(data, &magic, sizeof(u32));
			logmsg3("%s: [FAKE] PSP magic\n", __func__);
		}
	}

exit:
	pspSdkSetK1(k1);
	logmsg3("%s: fd=0x%08X, data=0x%p, size=0x%08X -> 0x%08X\n", __func__, fd, data, size, res);
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

	if (is_official) {
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

/// Init `is_official`, `psiso_offsets`, `pgd_buf`, `custom_config`, `config_size`
static int initGlobals() {
	const char * filename = sceKernelInitFileName();

	if (NULL == filename) {
		return -1;
	}

	SceUID fd = sceIoOpen(filename, PSP_O_RDONLY, 0);
	if (fd < 0) {
		logmsg("%s: [ERROR]: sceIoOpen %s -> 0x%08X\n", __func__, filename, fd);
		return fd;
	}

	// Read header
	PBPHeader header;
	int io_ret = sceIoRead(fd, &header, sizeof(PBPHeader));

	if (io_ret < 0) {
		logmsg("%s: [ERROR]: sceIoRead PBP header -> 0x%08X\n", __func__, io_ret);
		sceIoClose(fd);
		return io_ret;
	}

	// Get magic
	char magic[16];
	sceIoLseek(fd, header.psar_offset, PSP_SEEK_SET);
	io_ret = sceIoRead(fd, magic, sizeof(magic));

	if (io_ret < 0) {
		logmsg("%s: [ERROR]: sceIoRead PSISOIMG magic -> 0x%08X\n", __func__, io_ret);
		sceIoClose(fd);
		return io_ret;
	}

	// Reset psiso_offset
	memset(psiso_offsets, 0, sizeof(psiso_offsets));

	if (memcmp(magic, "PSISOIMG0000", 12) == 0) { // Single-Disc
		// Start at psar_offset
		psiso_offsets[0] = header.psar_offset;

		// Prepare for reading PGD
		sceIoLseek(fd, header.psar_offset + 0x400, PSP_SEEK_SET);
	} else if (memcmp(magic, "PSTITLEIMG000000", 16) == 0) { // Multi-Disc
		// Multi disc, offsets are stored at psar+0x200
		sceIoLseek(fd, header.psar_offset + 0x200, PSP_SEEK_SET);
		sceIoRead(fd, psiso_offsets, sizeof(psiso_offsets));

		// Adjust to make offsets absolute.
		for (int i = 0; i < NELEMS(psiso_offsets) && psiso_offsets[i] != 0; i++) {
			// Offsets are relative to psar
			psiso_offsets[i] += header.psar_offset;
		}

		// Prepare for reading PGD
		sceIoLseek(fd, header.psar_offset + 0x200, PSP_SEEK_SET);
	} else {
		sceIoClose(fd);
		logmsg("%s: [ERROR]: Failed to find PSISOIMG magic\n", __func__);
		return -1;
	}

	// Read PGD buffer
	io_ret = sceIoRead(fd, pgd_buf, sizeof(pgd_buf));

	if (io_ret < 0) {
		logmsg("%s: [ERROR]: sceIoRead PGD -> 0x%08X\n", __func__, io_ret);
		sceIoClose(fd);
		return io_ret;
	}
	logmsg3("%s: `pgd_buf` set\n", __func__);

	// Close fd
	sceIoClose(fd);

	// Must have at least one disc.
	if (psiso_offsets[0] == 0) {
		logmsg("%s: [ERROR]: Zero discs\n", __func__);
		return -1;
	}
	logmsg3("%s: `psiso_offsets` set\n", __func__);

	// Check PGD magic
	if (((u32 *)pgd_buf)[0] == PGD_MAGIC) {
		is_official = 1;
	} else {
		is_official = 0;
	}
	logmsg3("%s: `is_official` set\n", __func__);

	// Check and read config.bin
	//
	// From now on, errors should be ignored since custom config are optional.
	// 1. Set filename
	char config_filename[256] = {0};
	strcpy(config_filename, filename);
	char* slash = strrchr(config_filename, '/');
	if (!slash) {
		logmsg("%s: [ERROR]: Ignoring custom config: Invalid filename to find custom config: %s\n", __func__, filename);
	}
	strcpy(slash+1, "CONFIG.BIN");

	// 2. Open file
	fd = -1;
	fd = sceIoOpen(config_filename, PSP_O_RDONLY, 0777);
	if (fd < 0) {
		logmsg("%s: [ERROR]: sceIoOpen %s -> 0x%08X\n", __func__, config_filename, fd);
	}

	// 3. Read it if it exists
	if (fd > 0) {
		config_size = sceIoLseek(fd, 0, PSP_SEEK_END);
		if (config_size <= 0) {
			logmsg("%s: [ERROR]: Ignoring custom config: Fail to get custom config size\n", __func__);
			sceIoClose(fd);
			return 0;
		}

		sceIoLseek(fd, 0, PSP_SEEK_SET);
		io_ret = sceIoRead(fd, custom_config, config_size);

		if (io_ret < 0) {
			logmsg("%s: [ERROR]: Ignoring custom config: Fail to read custom config size\n", __func__);
			sceIoClose(fd);
			return 0;
		}

		logmsg3("%s: `custom_config` set: 0x%08X bytes\n", __func__, config_size);
		sceIoClose(fd);
	}

	return 0;
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

	// Patch permission issues with audio function
	_sceMeAudio_2AB4FE43 = (void*)sctrlHENFindFunctionInMod(mod, "sceMeAudio", 0x2AB4FE43);
	sctrlHENHookImportByNID(mod, "sceMeAudio", 0x2AB4FE43, sceMeAudio_2AB4FE43_Patched, 1);

	sctrlHENHookImportByNID(mod, "IoFileMgrForKernel", 0x6A638D83, sceIoReadPatched, 0);

	if (!is_official) {
		// Fake dnas drm
		sctrlHENHookImportByNID(mod, "IoFileMgrForKernel", 0x109F50BC, sceIoOpenPatched, 0);
		sctrlHENHookImportByNID(mod, "IoFileMgrForKernel", 0x63632449, sceIoIoctlPatched, 0);


		// Dummying amctrl decryption functions
		MAKE_DUMMY_FUNCTION(text_addr + 0xA90, 1);
		MAKE_NOP(text_addr + 0x53C)

		// Removes checks in scePopsManLoadModule that only allows loading modules below FW 3.XX
		MAKE_NOP(text_addr + 0x10D0);
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

		if (!is_official) {
			// Patch syscall to use it as deflate decompress
			_scePopsManExitVSHKernel = (void *)sctrlHENFindImportInMod(mod, "scePopsMan", 0x0090B2C8);
			sctrlHENHookImportByNID(mod, "scePopsMan", 0x0090B2C8, scePopsManExitVSHKernelPatched, 0);

			// Use our decompression function
			MAKE_CALL(mod->text_addr + 0xC99C, _scePopsManExitVSHKernel);

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

	int res = initGlobals();
	if (res < 0) {
		return res;
	}

	setPsxCompiledFwVersion(0x06060110);

	previous = sctrlHENSetStartModuleHandler(OnModuleStart);

	patchScePopsMgr();

	return 0;
}