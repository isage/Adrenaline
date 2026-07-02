TARGET = xmbctrl
OBJS = src/main.o \
	src/list.o \
	src/plugins.o \
	src/utils.o \
	src/config.o \
	src/patch_io.o \
	src/patch_vsh.o \
	src/patch_game_plugin.o \
	src/patch_savedata_plugin.o \
	exports.o

CFLAGS = -std=c99 -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
PSP_FW_VERSION = 661

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

INCDIR = ../../include include
LIBDIR = ../../lib

USE_PSPSDK_LIBS = 0
USE_PSPSDK_LIBC = 0

LIBS = -lpsppaf -lpspvsh -lpspvshctrl -lpspkubridge -lpspvshcommongui -lpspsystemctrl_user

ifdef XMBCTRL_DEBUG
CFLAGS += -DDEBUG=$(XMBCTRL_DEBUG)
LIBS += -lpspsysc_user
endif

ifdef DEBUG
CFLAGS += -DDEBUG=$(DEBUG)
LIBS += -lpspsysc_user
endif

include ../../lib/build_prx.mak