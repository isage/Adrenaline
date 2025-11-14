# Adrenaline — https://github.com/isage/Adrenaline
# -----------------------------------------------------------------------
# Licensed under the BSD 3-clause license
#
# build_prx.mak — An extension of PSPSDK build_prx.mak
#
# Copyright (c) 2025 GrayJack

ifeq ($(USE_KERNEL_LIBS),1)
CFLAGS += -D__KERNEL__
else
CFLAGS += -D__USER__
endif

PSPSDK = $(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak