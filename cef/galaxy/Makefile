TARGET = galaxy
OBJS = main.o exports.o ../bits/iso_common.o

INCDIR = ../include
CFLAGS = -Os -G0 -Wall -Wno-array-bounds -fshort-wchar -fno-pic -mno-check-zero-division -D__KERNEL__
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
PSP_FW_VERSION = 661

ifdef GALAXY_DEBUG
CFLAGS += -DDEBUG=$(GALAXY_DEBUG)
endif

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
endif

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC=1
USE_KERNEL_LIBS=1

LIBDIR = ../lib
LIBS = -lpspsystemctrl_kernel -lpspsysmem_kernel -lpspsysclib_kernel -lpspinit_kernel

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak
