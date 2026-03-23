TARGET = isotope
OBJS = main.o sceumd.o sceumdman.o umd9660_march_driver.o isoread.o csoread.o stub.o exports.o

INCDIR = ../include
CFLAGS = -O2 -G0 -Wall -Wno-array-bounds -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
PSP_FW_VERSION = 661

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

LIBDIR = ../lib
LIBS = -lcisoread -lpspsemaphore -lpspinit_kernel -lpsputils_kernel -lpspsysmem_kernel -lpspkernel_library -lpspsysclib_kernel -lpspsysevent_kernel -lpsploadcore_kernel -lpspiofilemgr_kernel -lpspthreadman_kernel -lpspsystemctrl_kernel

ifdef MEISO_DEBUG
CFLAGS += -DDEBUG=$(MEISO_DEBUG)
LIBS += -lpspsysclib_kernel
endif

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
LIBS += -lpspsysclib_kernel
endif

include ../lib/build_prx.mak
