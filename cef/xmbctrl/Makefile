TARGET = xmbctrl
OBJS = main.o list.o plugins.o utils.o exports.o config.o patch_vsh.o

CFLAGS = -std=c99 -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
PSP_FW_VERSION = 661

ifdef XMBCTRL_DEBUG
CFLAGS += -DDEBUG=$(XMBCTRL_DEBUG)
endif

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
endif

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

INCDIR = ../include
LIBDIR = ../lib

USE_PSPSDK_LIBS = 0
USE_PSPSDK_LIBC = 0

LIBS = -lpspsystemctrl_user -lpspsysc_user -lpspkubridge -lpspvshctrl -lpsppaf -lpspvsh -lpspvshcommongui

include ../lib/build_prx.mak