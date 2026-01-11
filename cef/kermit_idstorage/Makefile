TARGET = kermit_idstorage
OBJS = main.o exports.o

INCDIR = ../include
CFLAGS = -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
PSP_FW_VERSION = 661

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

LIBDIR = ../lib
LIBS = -lpspwlandrv_driver -lpspkermitperipheral_driver

ifdef KERMIT_IDSTORAGE_DEBUG
CFLAGS += -DDEBUG=$(KERMIT_IDSTORAGE_DEBUG)
LIBS += -lpspsysclib_kernel
endif

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
LIBS += -lpspsysclib_kernel
endif

include ../lib/build_prx.mak