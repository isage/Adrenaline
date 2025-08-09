TARGET = satelite
OBJS = main.o menu.o blit.o exports.o

INCDIR = ../include
CFLAGS = -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
PSP_FW_VERSION = 661

ifdef SATELITE_DEBUG
CFLAGS += -DDEBUG=$(SATELITE_DEBUG)
endif

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
endif

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

USE_PSPSDK_LIBS = 0
USE_PSPSDK_LIBC = 0

LIBDIR = ../lib
LIBS = -lpspsystemctrl_user -lpspsysc_user -lpspvshctrl -lpsppower_user

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak