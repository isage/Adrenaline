release: all
	psp-fixup-imports -m nid_map.txt $(TARGET).prx

TARGET = systemctrl
OBJS =  src/main.o \
	src/pgd.o \
	src/conf.o \
	src/nodrm.o \
	src/utils.o \
	src/malloc.o \
	src/plugin.o \
	src/kubridge.o \
	src/gameinfo.o \
	src/systemctrl.o \
	src/init_patch.o \
	src/game_patch.o \
	src/hide_patch.o \
	src/setjmp_clone.o \
	src/storage_cache.o \
	src/sysmodpatches.o \
	src/sysclibforuser.o \
	src/libraries_patch.o \
	src/executable_patch.o \
	../../bits/ttystdio.o \
	exports.o

INCDIR = ../../include include
CFLAGS = -Os -G0 -Wall -Wno-builtin-declaration-mismatch -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
PSP_FW_VERSION = 661

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

LIBDIR = ../../lib
LIBS = -lmicrolz -ladrenalinectrl -lpspsemaphore -lpspinit_kernel -lpspstdio_kernel -lpsputils_kernel -lpspsysmem_kernel -lpspkermit_driver -lpspchkreg_driver -lpspamctrl_driver -lpspsysclib_kernel -lpspdisplay_driver -lpsploadcore_kernel -lpsploadexec_kernel -lpspthreadman_kernel -lpspmodulemgr_kernel -lpspiofilemgr_kernel -lpspinterruptmanager_kernel

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

ifdef SCTRL_DEBUG
CFLAGS += -DDEBUG=$(SCTRL_DEBUG)
endif

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
endif

ifeq ($(NIGHTLY),1)
CFLAGS += -DNIGHTLY=1
endif

include ../../lib/build_prx.mak
