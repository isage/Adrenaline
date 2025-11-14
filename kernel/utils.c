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

#include <psp2kern/kernel/modulemgr.h>
#include <psp2kern/kernel/sysmem.h>
#include <psp2kern/io/fcntl.h>
#include <psp2kern/io/dirent.h>
#include <psp2kern/io/stat.h>

#include <stdio.h>
#include <string.h>

#define SCE_ERROR_ERRNO_EEXIST      0x80010011

void debug_printf(char *msg) {
	SceUID fd = ksceIoOpen("ux0:data/adrenaline_kernel_log.txt", SCE_O_WRONLY | SCE_O_CREAT | SCE_O_APPEND, 0777);
	if (fd >= 0) {
		ksceIoWrite(fd, msg, strlen(msg));
		ksceIoClose(fd);
	}
}

int ReadFile(char *file, void *buf, int size) {
	SceUID fd = ksceIoOpen(file, SCE_O_RDONLY, 0);
	if (fd < 0) {
		return fd;
	}

	int read = ksceIoRead(fd, buf, size);

	ksceIoClose(fd);
	return read;
}

int WriteFile(char *file, void *buf, int size) {
	SceUID fd = ksceIoOpen(file, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
	if (fd < 0) {
		return fd;
	}

	int written = ksceIoWrite(fd, buf, size);

	ksceIoClose(fd);
	return written;
}

SceUID g_heap_id = 0;
// 0 - success, !=0 - error
int my_mallocinit() {
	if (g_heap_id != 0) {
		// already init
		return 0;
	}

	SceKernelHeapCreateOpt opt;
	opt.size = sizeof(SceKernelHeapCreateOpt);
	opt.attr = SCE_KERNEL_HEAP_ATTR_HAS_AUTO_EXTEND;
	g_heap_id = ksceKernelCreateHeap("KernelAdrenalineHeap", 0x16000, &opt);
	ksceKernelPrintf("[DEBUG]: HEAP: 0x%08X\n", g_heap_id);

	if (g_heap_id < 0) {
		return g_heap_id;
	}
	return 0;
}

void* my_malloc(SceSize size) {
	return ksceKernelAllocHeapMemory(g_heap_id, size);
}

void my_free(void* ptr) {
	ksceKernelFreeHeapMemory(g_heap_id, ptr);
}

int strncasecmp(const char *s1, const char *s2, SceSize n) {
	const unsigned char *p1 = (const unsigned char *) s1;
	const unsigned char *p2 = (const unsigned char *) s2;
	unsigned char c1, c2;

	if (p1 == p2 || n == 0) {
		return 0;
	}

	if (s1 == NULL || s2 == NULL) {
		return (int)s1 - (int)s2;
	}

	do {
		c1 = tolower(*p1);
		c2 = tolower(*p2);

		if (--n == 0 || c1 == '\0') {
			break;
		}

		++p1;
		++p2;
	} while (c1 == c2);

	return c1 - c2;
}

int strcasecmp(const char *s1, const char *s2) {
	return strncasecmp(s1, s2, 2147483647);
}

#define MAX_PATH_LENGTH 1024
#define MAX_NAME_LENGTH 256
#define MAX_SHORT_NAME_LENGTH 64
#define MAX_MOUNT_POINT_LENGTH 16

#define DIRECTORY_SIZE (4 * 1024)
#define TRANSFER_SIZE (128 * 1024)
#define ALIGN(x, align) (((x) + ((align) - 1)) & ~((align) - 1))

static int hasEndSlash(const char *path) {
	return path[strlen(path) - 1] == '/';
}

int copyFile(const char *src_path, const char *dst_path) {
	// The source and destination paths are identical
	if (strcasecmp(src_path, dst_path) == 0) {
		return -1;
	}

	// The destination is a subfolder of the source folder
	int len = strlen(src_path);
	if (strncasecmp(src_path, dst_path, len) == 0 && (dst_path[len] == '/' || dst_path[len - 1] == '/')) {
		return -2;
	}
	SceUID fdsrc = ksceIoOpen(src_path, SCE_O_RDONLY, 0);
	if (fdsrc < 0) {
		return fdsrc;
	}

	SceUID fddst = ksceIoOpen(dst_path, SCE_O_WRONLY | SCE_O_CREAT | SCE_O_TRUNC, 0777);
	if (fddst < 0) {
		ksceIoClose(fdsrc);
		return fddst;
	}

	// void *buf = memalign(4096, TRANSFER_SIZE);
	// void *og_buf = my_memalign(4096, TRANSFER_SIZE);
	void *og_buf = my_malloc(TRANSFER_SIZE + 4096);
	if (og_buf == NULL) {
		return -3;
	}
	void *buf = (void*)ALIGN((uint32_t)buf, 4096);

	while (1) {
		int read = ksceIoRead(fdsrc, buf, TRANSFER_SIZE);

		if (read < 0) {
			my_free(og_buf);

			ksceIoClose(fddst);
			ksceIoClose(fdsrc);

			ksceIoRemove(dst_path);

			return read;
		}

		if (read == 0) {
			break;
		}

		int written = ksceIoWrite(fddst, buf, read);

		if (written < 0) {
			my_free(og_buf);

			ksceIoClose(fddst);
			ksceIoClose(fdsrc);

			ksceIoRemove(dst_path);

			return written;
		}
	}

	my_free(og_buf);

	// Inherit file stat
	SceIoStat stat;
	memset(&stat, 0, sizeof(SceIoStat));
	ksceIoGetstatByFd(fdsrc, &stat);
	ksceIoChstatByFd(fddst, &stat, 0x3B);

	ksceIoClose(fddst);
	ksceIoClose(fdsrc);

	return 1;
}

int copyPath(const char *src_path, const char *dst_path) {
	// The source and destination paths are identical
	if (strcasecmp(src_path, dst_path) == 0) {
		return -1;
	}

	// The destination is a subfolder of the source folder
	int len = strlen(src_path);
	if (strncasecmp(src_path, dst_path, len) == 0 && (dst_path[len] == '/' || dst_path[len - 1] == '/')) {
		return -2;
	}

	SceUID dfd = ksceIoDopen(src_path);
	if (dfd >= 0) {
		SceIoStat stat;
		memset(&stat, 0, sizeof(SceIoStat));
		ksceIoGetstatByFd(dfd, &stat);

		stat.st_mode |= SCE_S_IWUSR;

		int ret = ksceIoMkdir(dst_path, stat.st_mode & 0xFFF);

		if (ret < 0 && ret != SCE_ERROR_ERRNO_EEXIST) {
			ksceIoDclose(dfd);
			return ret;
		}

		if (ret == SCE_ERROR_ERRNO_EEXIST) {
			ksceIoChstat(dst_path, &stat, 0x3B);
		}

		int res = 0;

		do {
			SceIoDirent dir;
			memset(&dir, 0, sizeof(SceIoDirent));

			res = ksceIoDread(dfd, &dir);
			if (res > 0) {
				char *new_src_path = my_malloc(strlen(src_path) + strlen(dir.d_name) + 2);
				snprintf(new_src_path, MAX_PATH_LENGTH, "%s%s%s", src_path, hasEndSlash(src_path) ? "" : "/", dir.d_name);

				char *new_dst_path = my_malloc(strlen(dst_path) + strlen(dir.d_name) + 2);
				snprintf(new_dst_path, MAX_PATH_LENGTH, "%s%s%s", dst_path, hasEndSlash(dst_path) ? "" : "/", dir.d_name);

				int ret = 0;

				if (SCE_S_ISDIR(dir.d_stat.st_mode)) {
					ret = copyPath(new_src_path, new_dst_path);
				} else {
					ret = copyFile(new_src_path, new_dst_path);
				}

				my_free(new_dst_path);
				my_free(new_src_path);

				if (ret <= 0) {
					ksceIoDclose(dfd);
					return ret;
				}
			}
		} while (res > 0);

		ksceIoDclose(dfd);
	} else {
		return copyFile(src_path, dst_path);
	}

	return 1;
}