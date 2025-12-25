TARGET = recovery
OBJS = main.o menu.o installer.o plugins.o utils.o pspPSAR.o vgraph.o

INCDIR = ../include
CFLAGS = -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division -Wno-unused-variable -Wno-incompatible-pointer-types
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
PSP_FW_VERSION = 661

ifdef RECOVERY_DEBUG
CFLAGS += -DDEBUG=$(RECOVERY_DEBUG)
endif

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
endif

ifeq ($(NIGHTLY),1)
CFLAGS += -DNIGHTLY=1
endif

BUILD_PRX = 1

USE_PSPSDK_LIBC = 1
USE_PSPSDK_LIBS = 1

LIBDIR = ../lib
LIBS = -lz -lpspreg -lpspsystemctrl_user -lpspkubridge -lpspsysc_user

include ../lib/build.mak