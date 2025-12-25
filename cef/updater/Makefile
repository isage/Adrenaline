release: all
	pack-pbp EBOOT.PBP UPDATE.SFO ICON0.PNG NULL NULL NULL NULL updater.prx NULL

version:
	gcc version.c -o version

psp-updatelist.txt: version
	sed "s/{{ADRENALINE_VERSION}}/$$(./version)/g" psp-updatelist.template > psp-updatelist.txt

TARGET = updater
OBJS = main.o

INCDIR = ../include
CFLAGS = -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)
PSP_FW_VERSION = 661

ifeq ($(NIGHTLY),1)
CFLAGS += -DNIGHTLY=1
endif

BUILD_PRX = 1
USE_UPDATER_LIBS = 1

#LIBDIR = ../lib
LIBS = ../lib/libpspsystemctrl_user.a
EXTRA_TARGETS = version psp-updatelist.txt adrenaline_kernel.h adrenaline_user.h adrenaline_vsh.h frame_xml.h template_xml.h

include ../lib/build.mak