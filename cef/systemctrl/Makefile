release: all
	psp-fixup-imports -m nid_map.txt $(TARGET).prx

TARGET = systemctrl
OBJS = main.o adrenaline.o executable_patch.o libraries_patch.o io_patch.o init_patch.o power_patch.o usbcam_patch.o utility_patch.o game_patch.o hide_patch.o sysmodpatches.o modpatches.o \
	   nodrm.o conf.o malloc.o kubridge.o systemctrl.o ttystdio.o LflashFatfmt.o setjmp_clone.o exports.o \
	   lz4.o minilzo.o sysclibforuser.o gameinfo.o plugin.o pgd.o utils.o storage_cache.o

INCDIR = ../include
CFLAGS = -Os -G0 -Wall -Wno-builtin-declaration-mismatch -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
PSP_FW_VERSION = 661

ifdef SCTRL_DEBUG
CFLAGS += -DDEBUG=$(SCTRL_DEBUG)
endif

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
endif

ifeq ($(NIGHTLY),1)
CFLAGS += -DNIGHTLY=1
endif

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

LIBDIR = ../lib
LIBS = -lpspsemaphore -lpspamctrl_driver -lpspinit_kernel -lpspthreadman_kernel -lpspinterruptmanager_kernel -lpsploadexec_kernel -lpspsysclib_kernel -lpsploadcore_kernel -lpspmodulemgr_kernel -lpspusbcam_driver -lpspusb_driver -lpspchkreg_driver -lpsppower_driver -lpspdisplay_driver -lpspkermit_driver -lpspsysmem_kernel

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

include ../lib/build_prx.mak