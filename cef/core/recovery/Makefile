TARGET = recovery
OBJS = src/crt0.o \
	src/main.o \
	src/libc.o \
	src/menu.o \
	src/utils.o \
	src/vgraph.o \
	src/plugins.o \
	src/installer.o \
	imports.o

INCDIR = ../../include  include
CFLAGS = -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division -Wno-unused-variable -Wno-incompatible-pointer-types
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
PSP_FW_VERSION = 661

BUILD_PRX = 1

LIBDIR = ../../lib
LIBS = -nostdlib -lgcc -lz -lpspge -lpspreg -lpspuser -lpsppsar -lpspctrl -lpspmodinfo -lpspdisplay -lpspkubridge -lpspsysc_user -ladrenalinectrl -lpspsystemctrl_user

ifdef RECOVERY_DEBUG
CFLAGS += -DDEBUG=$(RECOVERY_DEBUG)
endif

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
endif

ifeq ($(NIGHTLY),1)
CFLAGS += -DNIGHTLY=1
endif

include ../../lib/build.mak
