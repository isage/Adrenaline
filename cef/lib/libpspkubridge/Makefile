PSPSDK=$(shell psp-config --pspsdk-path)
OBJS = \
	KUBridge_0000.o \
	KUBridge_0001.o \
	KUBridge_0002.o \
	KUBridge_0003.o \
	KUBridge_0004.o \
	KUBridge_0005.o \
	KUBridge_0006.o \
	KUBridge_0007.o \
	KUBridge_0008.o \
	KUBridge_0009.o \
	KUBridge_0010.o \
	KUBridge_0011.o \
	KUBridge_0012.o \
	KUBridge_0013.o \
	KUBridge_0014.o


CC=psp-gcc
INCDIR =
CFLAGS = -std=c99 -Os -G0 -Wall -fno-builtin-printf -I$(PSPSDK)/include
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LDFLAGS=-nodefaultlibs

LIBDIR =

%.o: KUBridge.S
	@psp-gcc -g -O2 -G0 -Wall -I${PSPSDK}/include -DF_$* $< -c -o $@

all: $(OBJS)
	@psp-ar cru libpspkubridge.a $(OBJS)
	@psp-ranlib libpspkubridge.a
	@echo Finished creating stubs for KUBridge

clean:
	@rm -f *.o
	@rm -f *.a
	@echo KUBridge cleaning done