PSPSDK = $(shell psp-config --pspsdk-path)
PSPDEV = $(shell psp-config --pspdev-path)
ARKSDK = $(PSPDEV)/share/ark-dev-sdk

TARGET = payloadex.bin

C_OBJS = main.o

CC = psp-gcc
LD = psp-ld
STRIP = psp-strip
OBJCOPY = psp-objcopy
INCDIR = ../include $(PSPSDK)/include
CFLAGS = -Wall -Os -G0 -fno-pic $(addprefix -I, $(INCDIR))
LINKFILE = $(ARKSDK)/build-tools/bootloadex.l
LIBS = -L $(PSPSDK)/lib -lansic

$(TARGET): main.elf
	$(Q)$(STRIP) -s $<
	$(Q)$(OBJCOPY) -O binary $< $(TARGET)
	@echo GET $@

clean:
	$(Q)rm -rf *~ *.s *.o *.elf $(EXTRA_CLEAN)

include $(ARKSDK)/build-tools/beauty_bin.mak