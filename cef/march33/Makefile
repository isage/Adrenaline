TARGET = march33
OBJS = main.o exports.o umdman.o mediaman.o umd9660.o ../bits/iso_common.o

INCDIR = ../include
CFLAGS = -O2 -G0 -Wall -Wno-array-bounds -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
PSP_FW_VERSION = 661

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

LIBDIR = ../lib
LIBS = -lcisoread -lpspsysmem_kernel  -lpspsystemctrl_kernel -lpspthreadman_kernel -lpspinit_kernel

ifdef MARCH33_DEBUG
CFLAGS += -DDEBUG=$(MARCH33_DEBUG)
LIBS += -lpspsysclib_kernel
endif

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
LIBS += -lpspsysclib_kernel
endif

include ../lib/build_prx.mak