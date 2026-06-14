TARGET = vshctrl
OBJS = src/main.o \
	src/vshctrl.o \
	src/patch_io.o \
	src/patch_vsh.o \
	src/virtualsfo.o \
	src/virtualpbpmgr.o \
	src/isofs_driver/isofs_driver.o \
	src/isofs_driver/umd9660_driver.o \
	../../bits/iso_common.o \
	imports.o \
	exports.o

INCDIR = ../../include  include
CFLAGS = -Os -G0 -Wall -Wno-array-bounds -fshort-wchar -fno-pic -mno-check-zero-division -D__USE_USER_ALLOC
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
PSP_FW_VERSION = 661

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

LIBDIR = ../../lib
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

include ../../lib/build_prx.mak