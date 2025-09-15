PSPSDK=$(shell psp-config --pspsdk-path)
OBJS = \
	SystemCtrlForKernel_0000.o SystemCtrlForKernel_0001.o \
	SystemCtrlForKernel_0002.o SystemCtrlForKernel_0003.o \
	SystemCtrlForKernel_0004.o SystemCtrlForKernel_0005.o \
	SystemCtrlForKernel_0006.o SystemCtrlForKernel_0007.o \
	SystemCtrlForKernel_0008.o SystemCtrlForKernel_0009.o \
	SystemCtrlForKernel_0010.o SystemCtrlForKernel_0011.o \
	SystemCtrlForKernel_0012.o SystemCtrlForKernel_0013.o \
	SystemCtrlForKernel_0014.o SystemCtrlForKernel_0015.o \
	SystemCtrlForKernel_0016.o SystemCtrlForKernel_0017.o \
	SystemCtrlForKernel_0018.o SystemCtrlForKernel_0019.o \
	SystemCtrlForKernel_0020.o SystemCtrlForKernel_0021.o \
	SystemCtrlForKernel_0022.o SystemCtrlForKernel_0023.o \
	SystemCtrlForKernel_0024.o SystemCtrlForKernel_0025.o \
	SystemCtrlForKernel_0026.o SystemCtrlForKernel_0027.o \
	SystemCtrlForKernel_0028.o SystemCtrlForKernel_0029.o \
	SystemCtrlForKernel_0030.o SystemCtrlForKernel_0031.o \
	SystemCtrlForKernel_0032.o SystemCtrlForKernel_0033.o \
	SystemCtrlForKernel_0034.o SystemCtrlForKernel_0035.o \
	SystemCtrlForKernel_0036.o SystemCtrlForKernel_0037.o \
	SystemCtrlForKernel_0038.o SystemCtrlForKernel_0039.o \
	SystemCtrlForKernel_0040.o SystemCtrlForKernel_0041.o \
	SystemCtrlForKernel_0042.o SystemCtrlForKernel_0043.o \
	SystemCtrlForKernel_0044.o SystemCtrlForKernel_0045.o \
	SystemCtrlForKernel_0046.o SystemCtrlForKernel_0047.o \
	SystemCtrlForKernel_0048.o SystemCtrlForKernel_0049.o \
	SystemCtrlForKernel_0050.o SystemCtrlForKernel_0051.o \
	SystemCtrlForKernel_0052.o SystemCtrlForKernel_0053.o \
	SystemCtrlForKernel_0054.o SystemCtrlForKernel_0055.o \
	SystemCtrlForKernel_0056.o SystemCtrlForKernel_0057.o \
	SystemCtrlForKernel_0058.o SystemCtrlForKernel_0059.o \
	SystemCtrlForKernel_0060.o SystemCtrlForKernel_0061.o \
	SystemCtrlForKernel_0062.o SystemCtrlForKernel_0063.o \
	SystemCtrlForKernel_0064.o SystemCtrlForKernel_0065.o \
	SystemCtrlForKernel_0066.o SystemCtrlForKernel_0067.o \
	SystemCtrlForKernel_0068.o SystemCtrlForKernel_0069.o \
	SystemCtrlForKernel_0070.o SystemCtrlForKernel_0071.o \
	SystemCtrlForKernel_0072.o SystemCtrlForKernel_0073.o \
	SystemCtrlForKernel_0074.o SystemCtrlForKernel_0075.o \
	SystemCtrlForKernel_0076.o SystemCtrlForKernel_0077.o \
	SystemCtrlForKernel_0078.o SystemCtrlForKernel_0079.o \
	SystemCtrlForKernel_0080.o SystemCtrlForKernel_0081.o \
	SystemCtrlForKernel_0082.o SystemCtrlForKernel_0083.o \
	SystemCtrlForKernel_0084.o SystemCtrlForKernel_0085.o \
	SystemCtrlForKernel_0086.o SystemCtrlForKernel_0087.o \
	SystemCtrlForKernel_0088.o SystemCtrlForKernel_0089.o \
	SystemCtrlForKernel_0090.o SystemCtrlForKernel_0091.o \
	SystemCtrlForKernel_0092.o SystemCtrlForKernel_0093.o \
	SystemCtrlForKernel_0094.o SystemCtrlForKernel_0095.o \
	SystemCtrlForKernel_0096.o SystemCtrlForKernel_0097.o \
	SystemCtrlForKernel_0098.o SystemCtrlForKernel_0099.o \
	SystemCtrlForKernel_0100.o SystemCtrlForKernel_0101.o \
	SystemCtrlForKernel_0102.o SystemCtrlForKernel_0103.o \
	SystemCtrlForKernel_0104.o SystemCtrlForKernel_0105.o \
	SystemCtrlForKernel_0106.o SystemCtrlForKernel_0107.o \
	SystemCtrlForKernel_0108.o


%.o: SystemCtrlForKernel.S
	@psp-gcc -g -O2 -G0 -Wall -I${PSPSDK}/include -DF_$* $< -c -o $@

all: $(OBJS)
	@psp-ar cru libpspsystemctrl_kernel.a $(OBJS)
	@psp-ranlib libpspsystemctrl_kernel.a
	@echo Finished creating stubs for SystemCtrlForKernel

clean:
	@rm -f *.o
	@rm -f *.a
	@echo SystemCtrlForKernel cleaning done