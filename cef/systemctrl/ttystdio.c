#include <string.h>
#include <unistd.h>

#include <psperror.h>
#include <pspdebug.h>
#include <pspkernel.h>

#include <systemctrl_adrenaline.h>

#include "main.h"
#include "adrenaline.h"
#include "../../adrenaline_compat.h"

static SceUID g_out_sema = 0;

static int io_init(PspIoDrvArg *arg) {
	return 0;
}

static int io_exit(PspIoDrvArg *arg) {
	return 0;
}

static int io_open(PspIoDrvFileArg *arg, char *file, int mode, SceMode mask) {
	if (arg->fs_num < 3) {
		return 0;
	}

	return SCE_ERR_DEVICE_IO;
}

static int io_close(PspIoDrvFileArg *arg) {
	if (arg->fs_num < 3) {
		return 0;
	}
	return SCE_ERR_DEVICE_IO;
}

static int io_read(PspIoDrvFileArg *arg, char *data, int len) {
	return 0;
}

static int io_write(PspIoDrvFileArg *arg, const char *data, int len) {
	int ret = 0;

	(void) sceKernelWaitSema(g_out_sema, 1, 0);
	if ((arg->fs_num == STDOUT_FILENO) || (arg->fs_num == STDERR_FILENO)) {
		if (len < 1023) {
			memset(g_adrenaline->printbuf, 0, 1024);
			strncpy(g_adrenaline->printbuf, data, len);
			ret = SendAdrenalineCmd(ADRENALINE_VITA_CMD_PRINT, 0);
		}
	}
	(void) sceKernelSignalSema(g_out_sema, 1);

	return ret;
}

static SceOff io_lseek(PspIoDrvFileArg *arg, SceOff ofs, int whence) {
	return SCE_KERR_BADF;
}

static int io_ioctl(PspIoDrvFileArg *arg, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen) {
	return SCE_KERR_INVAL;
}

static int io_remove(PspIoDrvFileArg *arg, const char *name) {
	return 0;
}

static int io_mkdir(PspIoDrvFileArg *arg, const char *name, SceMode mode) {
	return 0;
}

static int io_rmdir(PspIoDrvFileArg *arg, const char *name) {
	return 0;
}

static int io_dopen(PspIoDrvFileArg *arg, const char *dir) {
	return 0;
}

static int io_dclose(PspIoDrvFileArg *arg) {
	return 0;
}

static int io_dread(PspIoDrvFileArg *arg, SceIoDirent *dir) {
	return 0;
}

static int io_getstat(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat) {
	return 0;
}

static int io_chstat(PspIoDrvFileArg *arg, const char *file, SceIoStat *stat, int bits) {
	return 0;
}

static int io_rename(PspIoDrvFileArg *arg, const char *oldname, const char *newname) {
	return 0;
}

static int io_chdir(PspIoDrvFileArg *arg, const char *dir) {
	return 0;
}

static int io_mount(PspIoDrvFileArg *arg) {
	return 0;
}

static int io_umount(PspIoDrvFileArg *arg) {
	return 0;
}

static int io_devctl(PspIoDrvFileArg *arg, const char *devname, unsigned int cmd, void *indata, int inlen, void *outdata, int outlen) {
	return 0;
}

static int io_unknown(PspIoDrvFileArg *arg) {
	return 0;
}

static PspIoDrvFuncs tty_funcs = {
	io_init,
	io_exit,
	io_open,
	io_close,
	io_read,
	io_write,
	io_lseek,
	io_ioctl,
	io_remove,
	io_mkdir,
	io_rmdir,
	io_dopen,
	io_dclose,
	io_dread,
	io_getstat,
	io_chstat,
	io_rename,
	io_chdir,
	io_mount,
	io_umount,
	io_devctl,
	io_unknown,
};

static PspIoDrv tty_driver = {
	"tty", 0x10, 0x800, "TTY", &tty_funcs
};

int tty_init(void) {
	int ret;
	(void) sceIoDelDrv("tty"); /* Ignore error */
	ret = sceIoAddDrv(&tty_driver);
	if (ret < 0) {
		return ret;
	}

	g_out_sema = sceKernelCreateSema("TtyOutMutex", 0, 1, 1, NULL);
	if (g_out_sema < 0) {
		return g_out_sema;
	}

	ret = sceIoReopen("tty0:", PSP_O_RDONLY, 0777, sceKernelStdin());
	if (ret < 0) {
		return ret;
	}

	ret = sceKernelStdoutReopen("tty1:", PSP_O_WRONLY, 0777);
	if (ret < 0) {
		return ret;
	}

	ret = sceKernelStderrReopen("tty2:", PSP_O_WRONLY, 0777);
	if (ret < 0) {
		return ret;
	}

    // TODO: install kputchar handler

	return 0;
}
