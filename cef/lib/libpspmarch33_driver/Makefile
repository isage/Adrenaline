PSPSDK=$(shell psp-config --pspsdk-path)
OBJS = \
	march33_driver_0000.o \
	march33_driver_0001.o \
	march33_driver_0002.o \
	march33_driver_0003.o \
	march33_driver_0004.o

CC=psp-gcc
INCDIR =
CFLAGS = -std=c99 -Os -G0 -Wall -fno-builtin-printf -I$(PSPSDK)/include
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LDFLAGS=-nodefaultlibs

LIBDIR =

%.o: march33_driver.S
	@psp-gcc -g -O2 -G0 -Wall -I${PSPSDK}/include -DF_$* $< -c -o $@

all: $(OBJS)
	@psp-ar cru libpspmarch33_driver.a $(OBJS)
	@psp-ranlib libpspmarch33_driver.a
	@echo Finished creating stubs for march33_driver

clean:
	@rm -f *.o
	@rm -f *.a
	@echo march33_driver cleaning done