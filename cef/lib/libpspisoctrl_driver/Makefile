PSPSDK=$(shell psp-config --pspsdk-path)
OBJS = \
	isoCtrl_driver_0000.o \
	isoCtrl_driver_0001.o \
	isoCtrl_driver_0002.o \
	isoCtrl_driver_0003.o

CC=psp-gcc
INCDIR =
CFLAGS = -std=c99 -Os -G0 -Wall -fno-builtin-printf -I$(PSPSDK)/include
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
LDFLAGS=-nodefaultlibs

LIBDIR =

%.o: isoCtrl_driver.S
	@psp-gcc -g -O2 -G0 -Wall -I${PSPSDK}/include -DF_$* $< -c -o $@

all: $(OBJS)
	@psp-ar cru libpspisoctrl_driver.a $(OBJS)
	@psp-ranlib libpspisoctrl_driver.a
	@echo Finished creating stubs for isoCtrl_driver

clean:
	@rm -f *.o
	@rm -f *.a
	@echo isoCtrl_driver cleaning done