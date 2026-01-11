TARGET = inferno
OBJS = main.o iodrv_funcs.o umd.o isoread.o isocache.o exports.o ../bits/iso_common.o

INCDIR = ../include
CFLAGS = -Os -G0 -Wall -Wno-array-bounds -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
PSP_FW_VERSION = 661

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC=1
USE_KERNEL_LIBS=1

LIBDIR = ../lib
LIBS = -lpspsystemctrl_kernel -lpspsemaphore -lpspsysmem_kernel -lpspinit_kernel

ifdef INFERNO_DEBUG
CFLAGS += -DDEBUG=$(INFERNO_DEBUG)
LIBS += -lpspsysclib_kernel
endif

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
LIBS += -lpspsysclib_kernel
endif

include ../lib/build_prx.mak