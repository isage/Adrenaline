PSPSDK=$(shell psp-config --pspsdk-path)
OBJS = \
	galaxy_driver_0000.o \
	galaxy_driver_0001.o \
	galaxy_driver_0002.o \
	galaxy_driver_0003.o \
	galaxy_driver_0004.o

CC=psp-gcc
INCDIR =
CFLAGS = -std=c99 -Os -G0 -Wall -fno-builtin-printf -I$(PSPSDK)/include
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LDFLAGS=-nodefaultlibs

LIBDIR =

%.o: galaxy_driver.S
	@psp-gcc -g -O2 -G0 -Wall -I${PSPSDK}/include -DF_$* $< -c -o $@

all: $(OBJS)
	@psp-ar cru libpspgalaxy_driver.a $(OBJS)
	@psp-ranlib libpspgalaxy_driver.a
	@echo Finished creating stubs for galaxy_driver

clean:
	@rm -f *.o
	@rm -f *.a
	@echo galaxy_driver cleaning done