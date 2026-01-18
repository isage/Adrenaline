TARGET = vshctrl
OBJS = main.o vshctrl.o patch_vsh.o patch_io.o virtualsfo.o virtualpbpmgr.o isofs_driver/isofs_driver.o isofs_driver/umd9660_driver.o exports.o imports.o ../bits/iso_common.o

INCDIR = ../include
CFLAGS = -Os -G0 -Wall -Wno-array-bounds -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
PSP_FW_VERSION = 661

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

LIBDIR = ../lib
LIBS = -lcisoread -lpspsystemctrl_kernel -ladrenalinectrl -lpsppower_driver -lpsprtc_driver -lpspusb_driver -lpspreg_driver -lpspsysclib_kernel -lpspsysmem_kernel -lpspinit_kernel

ifdef VSHCTRL_DEBUG
CFLAGS += -DDEBUG=$(VSHCTRL_DEBUG)
endif

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
endif

ifeq ($(NIGHTLY),1)
CFLAGS += -DNIGHTLY=1
endif

include ../lib/build_prx.mak