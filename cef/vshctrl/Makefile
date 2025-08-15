TARGET = vshctrl
OBJS = main.o registry.o virtualsfo.o virtualpbpmgr.o isofs_driver/isofs_driver.o isofs_driver/umd9660_driver.o isofs_driver/csoread.o exports.o imports.o

INCDIR = ../include
CFLAGS = -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division -D__KERNEL__
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

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak