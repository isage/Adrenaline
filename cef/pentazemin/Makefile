PSPSDK = $(shell psp-config --pspsdk-path)

TARGET = pentazemin

C_OBJS =  \
	main.o \
	src/usbcam.o \
	src/vitamem.o \
	src/syspatch.o \
	src/io_patch.o  \
	src/loadexec.o   \
	src/adrenaline.o  \
	src/init_patch.o   \
	src/power_patch.o   \
	src/LflashFatfmt.o   \
	src/impose_patch.o    \
	src/utility_patch.o    \
	src/vlf.o \

OBJS = \
	$(C_OBJS)

all: $(TARGET).prx

INCDIR = ../include include
CFLAGS = -std=c99 -Os -G0 -Wall -fno-pic -fshort-wchar -mno-check-zero-division

CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC=1
USE_KERNEL_LIBS=1

LIBDIR = ../lib
LDFLAGS =  -nostartfiles
LIBS = -lpspsystemctrl_kernel -lpspusbcam_driver -lpspusb_driver -lpsppower_driver -lpspkermit_driver -lpspinit_kernel -lpspsysmem_kernel -lpspmodulemgr_kernel

PSP_FW_VERSION = 660

include $(PSPSDK)/lib/build.mak
