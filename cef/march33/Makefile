TARGET = march33
OBJS = main.o exports.o umdman.o mediaman.o umd9660.o ../bits/iso_common.o

INCDIR = ../include
CFLAGS = -O2 -G0 -Wall -Wno-array-bounds -fshort-wchar -fno-pic -mno-check-zero-division -D__KERNEL__
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
PSP_FW_VERSION = 661

ifdef MARCH33_DEBUG
CFLAGS += -DDEBUG=$(MARCH33_DEBUG)
endif

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
endif

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

LIBDIR = ../lib
LIBS = -lpspsysmem_kernel  -lpspsystemctrl_kernel -lpspthreadman_kernel -lpspinit_kernel

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak

