TARGET = binary
OBJS = start.o main.o

CFLAGS = -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LDFLAGS = -nostartfiles -T linkfile.x -nostdlib

PSPSDK = $(shell psp-config --pspsdk-path)
include build_bin.mak