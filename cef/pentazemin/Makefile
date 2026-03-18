PSPSDK = $(shell psp-config --pspsdk-path)

TARGET = pentazemin

C_OBJS = main.o epictrl.o ../bits/ttystdio.o usbcam_patch.o vitamem.o modpatches.o \
	io_patch.o adrenaline.o init_patch.o power_patch.o LflashFatfmt.o impose_patch.o utility_patch.o \

OBJS = $(C_OBJS)

all: $(TARGET).prx

INCDIR = ../include include
CFLAGS = -std=c99 -Os -G0 -Wall -fno-pic -fshort-wchar -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LDFLAGS =  -nostartfiles
PSP_FW_VERSION = 661

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC=1
USE_KERNEL_LIBS=1

LIBDIR = ../lib
LIBS = -lpspusb_driver -lpspinit_kernel -lpspstdio_kernel -lpsppower_driver -lpsputils_kernel -lpspsysmem_kernel -lpspkermit_driver -lpspusbcam_driver -lpspdisplay_driver -lpspsysclib_kernel -lpspsysevent_kernel -lpsploadcore_kernel -lpspthreadman_kernel -lpspmodulemgr_kernel -lpspiofilemgr_kernel -lpspinterruptmanager_kernel -lpspsystemctrl_kernel

ifdef PENTAZEMIN_DEBUG
CFLAGS += -DDEBUG=$(PENTAZEMIN_DEBUG)
endif

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
endif

include $(PSPSDK)/lib/build.mak
