TARGET = janus
OBJS = src/main.o \
	src/blit.o \
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
LIBS = -ltinyfont -lpspsystemctrl_kernel -lpspisoctrl_driver -lpspdisplay_driver -lpspctrl_driver -lpspkernel -lpspsysclib_kernel -lpspreg_driver

ifdef JANUS_DEBUG
CFLAGS += -DDEBUG=$(JANUS_DEBUG)
endif

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
endif

ifeq ($(NIGHTLY),1)
CFLAGS += -DNIGHTLY=1
endif

include ../../lib/build_prx.mak