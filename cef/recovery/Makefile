TARGET = recovery
OBJS = crt0.o main.o libc.o menu.o installer.o plugins.o utils.o vgraph.o imports.o

INCDIR = ../include
CFLAGS = -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division -Wno-unused-variable -Wno-incompatible-pointer-types
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
PSP_FW_VERSION = 661

BUILD_PRX = 1

LIBDIR = ../lib
LIBS = -nostdlib -lgcc -lz -lpspuser -lpspreg -lpspsystemctrl_user -lpspkubridge -ladrenalinectrl -lpspsysc_user -lpsppsar -lpspmodinfo


ifdef RECOVERY_DEBUG
CFLAGS += -DDEBUG=$(RECOVERY_DEBUG)
endif

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
endif

ifeq ($(NIGHTLY),1)
CFLAGS += -DNIGHTLY=1
endif

include ../lib/build.mak
