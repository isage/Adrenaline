TARGET = recovery
OBJS = main.o menu.o installer.o plugins.o utils.o pspPSAR.o vgraph.o

INCDIR = ../include
CFLAGS = -O2 -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division -Wno-unused-variable -Wno-incompatible-pointer-types
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
PSP_FW_VERSION = 661

ifdef RECOVERY_DEBUG
CFLAGS += -DDEBUG=$(RECOVERY_DEBUG)
endif

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
endif

BUILD_PRX = 1

USE_PSPSDK_LIBC = 1
USE_PSPSDK_LIBS = 1

LIBDIR = ../lib
LIBS = -lpspsystemctrl_user -lpspkubridge -lpspreg -lz

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak