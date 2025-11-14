PSPSDK=$(shell psp-config --pspsdk-path)
OBJS = \
	inferno_driver_0000.o \
	inferno_driver_0001.o \
	inferno_driver_0002.o \
	inferno_driver_0003.o \
	inferno_driver_0004.o \
	inferno_driver_0005.o \
	inferno_driver_0006.o \
	inferno_driver_0007.o \
	inferno_driver_0008.o

CC=psp-gcc
INCDIR =
CFLAGS = -std=c99 -Os -G0 -Wall -fno-builtin-printf -I$(PSPSDK)/include
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LDFLAGS=-nodefaultlibs

LIBDIR =

%.o: inferno_driver.S
	@psp-gcc -g -O2 -G0 -Wall -I${PSPSDK}/include -DF_$* $< -c -o $@

all: $(OBJS)
	@psp-ar cru libpspinferno_driver.a $(OBJS)
	@psp-ranlib libpspinferno_driver.a
	@echo Finished creating stubs for inferno_driver

clean:
	@rm -f *.o
	@rm -f *.a
	@echo inferno_driver cleaning done