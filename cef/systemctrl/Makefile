release: all
	psp-fixup-imports -m nid_map.txt $(TARGET).prx

TARGET = systemctrl
OBJS = main.o adrenaline.o executable_patch.o libraries_patch.o io_patch.o init_patch.o power_patch.o usbcam_patch.o utility_patch.o sysmodpatches.o \
	   nodrm.o conf.o malloc.o kubridge.o systemctrl.o ttystdio.o LflashFatfmt.o string_clone.o setjmp_clone.o imports.o exports.o \
	   lz4.o minilzo.o sysclibforuser.o

INCDIR = ../include
CFLAGS = -O2 -Os -G0 -Wall -Wno-builtin-declaration-mismatch -fshort-wchar -fno-pic -mno-check-zero-division -D__KERNEL__
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
PSP_FW_VERSION = 661

ifdef SCTRL_DEBUG
CFLAGS += -DDEBUG=$(SCTRL_DEBUG)
endif

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
endif

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

LIBDIR = ../lib
LIBS = -lpspsemaphore -lpspinit_kernel -lpspinterruptmanager_kernel -lpsploadexec_kernel -lpspsysclib_kernel -lpsploadcore_kernel -lpspmodulemgr_kernel -lpspusbcam_driver -lpspusb_driver -lpspchkreg_driver -lpsppower_driver -lpspdisplay_driver -lpspkermit_driver

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak