PSPSDK=$(shell psp-config --pspsdk-path)
OBJS = \
	VshCtrl_0000.o \
	VshCtrl_0001.o \
	VshCtrl_0002.o \
	VshCtrl_0003.o \
	VshCtrl_0004.o


%.o: VshCtrl.S
	psp-gcc -g -O2 -G0 -Wall -I${PSPSDK}/include -DF_$* $< -c -o $@

all: $(OBJS)
	psp-ar cru libpspvshctrl.a $(OBJS)
	psp-ranlib libpspvshctrl.a

clean:
	rm -f *.o
	rm -f *.a
