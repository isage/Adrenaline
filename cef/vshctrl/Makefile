TARGET = vshctrl
OBJS = main.o vshctrl.o patch_vsh.o patch_io.o virtualsfo.o virtualpbpmgr.o isofs_driver/isofs_driver.o isofs_driver/umd9660_driver.o imports.o exports.o ../bits/iso_common.o

INCDIR = ../include
CFLAGS = -Os -G0 -Wall -Wno-array-bounds -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
PSP_FW_VERSION = 661

ifdef VSHCTRL_DEBUG
CFLAGS += -DDEBUG=$(VSHCTRL_DEBUG)
endif

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
endif

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

LIBDIR = ../lib
LIBS = -lpspsystemctrl_kernel -lpsppower_driver -lpsprtc_driver -lpspusb_driver -lpspreg_driver -lpspsysclib_kernel -lpspsysmem_kernel -lpspinit_kernel

include ../lib/build_prx.mak