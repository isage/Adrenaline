PSPSDK = $(shell psp-config --pspsdk-path)

TARGET = pentazemin

C_OBJS = main.o src/epictrl.o src/ttystdio.o src/usbcam_patch.o src/vitamem.o src/modpatches.o \
	src/io_patch.o src/adrenaline.o src/init_patch.o src/power_patch.o src/LflashFatfmt.o src/impose_patch.o src/utility_patch.o \

OBJS = \
	$(C_OBJS)

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
LIBS = -lpspsystemctrl_kernel -lpspusbcam_driver -lpspusb_driver -lpsppower_driver -lpspkermit_driver -lpspinit_kernel -lpspsysmem_kernel -lpspmodulemgr_kernel

ifdef PENTAZEMIN_DEBUG
CFLAGS += -DDEBUG=$(PENTAZEMIN_DEBUG)
LIBS += -lpspsysclib_kernel
endif

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
LIBS += -lpspsysclib_kernel
endif

include $(PSPSDK)/lib/build.mak
