TARGET = xmbctrl
OBJS = main.o settings.o list.o plugins.o utils.o imports.o exports.o config.o

CFLAGS = -std=c99 -O2 -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

BUILD_PRX = 1
PRX_EXPORTS = exports.exp

INCDIR = ../include
LIBDIR = ../lib

USE_PSPSDK_LIBS = 0
USE_PSPSDK_LIBC = 0

LIBS = -lpspsystemctrl_user -lpspsysc_user -lpspkubridge -lpspvshctrl -lpspreg

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak
