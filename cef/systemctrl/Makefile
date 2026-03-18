release: all
	psp-fixup-imports -m nid_map.txt $(TARGET).prx

TARGET = systemctrl
OBJS =  main.o executable_patch.o libraries_patch.o init_patch.o \
	game_patch.o hide_patch.o sysmodpatches.o \
	nodrm.o conf.o malloc.o kubridge.o systemctrl.o setjmp_clone.o \
	sysclibforuser.o gameinfo.o plugin.o pgd.o utils.o storage_cache.o \
	../bits/ttystdio.o exports.o

INCDIR = ../include
CFLAGS = -Os -G0 -Wall -Wno-builtin-declaration-mismatch -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
PSP_FW_VERSION = 661

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

LIBDIR = ../lib
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

include ../lib/build_prx.mak
