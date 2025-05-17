TARGET = vshctrl
OBJS = main.o registry.o exports.o imports.o \
	utils.o isoreader.o dirent_track.o disctype.o virtual_pbp.o virtual_mp4.o patched_io.o patch_vsh.o printk.o

INCDIR = ../include
CFLAGS = -O2 -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
PSP_FW_VERSION=661

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

LIBDIR = ../lib
LIBS = -lpspsystemctrl_kernel -lpsppower_driver -lpsprtc_driver -lpspusb_driver -lpspreg_driver

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak