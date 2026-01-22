PY = $(shell which python3)
PSPDEV = $(shell psp-config --pspdev-path)
CFWSDK = $(PSPDEV)/share/psp-cfw-sdk
PSPGZ = $(CFWSDK)/build-tools/gz/pspgz.py
KHDR = $(CFWSDK)/build-tools/gz/SystemControl.hdr
UHDR = $(CFWSDK)/build-tools/gz/UserModule.hdr

all: _payloadex _payloadex_ark _rebootex _binary _systemctrl _pentazemin _popcorn _galaxy _march33 _inferno _vshctrl _satelite _recovery _kermit_idstorage _btcnf _xmbctrl

KERNEL_STUBS = pspamctrl_driver pspwlandrv_driver pspkermitperipheral_driver psppower_driver pspreg_driver pspchkreg_driver pspusb_driver pspdisplay_driver pspusbcam_driver pspthreadman_kernel psploadcore_kernel pspmodulemgr_kernel pspkermit_driver pspinit_kernel pspsysclib_kernel psploadexec_kernel pspinterruptmanager_kernel pspsemaphore pspsysmem_kernel

USER_STUBS = psppower pspreg psppaf pspvsh pspvshcommongui

SYSTEM_STUBS = $(KERNEL_STUBS) $(USER_STUBS)

_btcnf:
	make -C btcnf/pspbtcnf_inferno clean
	make -C btcnf/pspbtcnf_inferno
	cp "btcnf/pspbtcnf_inferno/pspbtcnf.bin" "../user/flash0/kd/pspbtknf.bin"
	make -C btcnf/pspbtcnf_inferno clean

	make -C btcnf/pspbtcnf_march33 clean
	make -C btcnf/pspbtcnf_march33
	cp "btcnf/pspbtcnf_march33/pspbtcnf.bin" "../user/flash0/kd/pspbtlnf.bin"
	make -C btcnf/pspbtcnf_march33 clean

	make -C btcnf/pspbtcnf_normal clean
	make -C btcnf/pspbtcnf_normal
	cp "btcnf/pspbtcnf_normal/pspbtcnf.bin" "../user/flash0/kd/pspbtjnf.bin"
	make -C btcnf/pspbtcnf_normal clean

	make -C btcnf/pspbtcnf_np9660 clean
	make -C btcnf/pspbtcnf_np9660
	cp "btcnf/pspbtcnf_np9660/pspbtcnf.bin" "../user/flash0/kd/pspbtmnf.bin"
	make -C btcnf/pspbtcnf_np9660 clean

	make -C btcnf/pspbtcnf_recovery clean
	make -C btcnf/pspbtcnf_recovery
	cp "btcnf/pspbtcnf_recovery/pspbtcnf.bin" "../user/flash0/kd/pspbtrnf.bin"
	make -C btcnf/pspbtcnf_recovery clean

_payloadex:
	make -C payloadex clean
	make -C payloadex -j
	cp "payloadex/payloadex.bin" "../user/flash0/payloadex.bin"
	make -C payloadex clean

_payloadex_ark:
	make -C payloadex_ark clean
	make -C payloadex_ark -j
	cp "payloadex_ark/payloadex_ark.bin" "../user/flash0/payloadex_ark.bin"
	make -C payloadex_ark clean

_rebootex:
	make -C rebootex clean
	make -C rebootex -j
	bin2c rebootex/rebootex.bin systemctrl/rebootex.h rebootex
	make -C rebootex clean

_binary:
	make -C binary clean
	make -C binary -j
	bin2c binary/binary.bin systemctrl/binary.h binary
	bin2c binary/binary.bin pentazemin/binary.h binary
	make -C binary clean

_systemctrl: | _rebootex _binary
	make -C systemctrl clean
	make -C systemctrl -j
ifdef USE_PSP_PACKER
	psp-packer systemctrl/systemctrl.prx -o ../user/flash0/kd/systemctrl.prx
else
	$(PY) $(PSPGZ) ../user/flash0/kd/systemctrl.prx $(KHDR) systemctrl/systemctrl.prx SystemControl 0x3007
endif
	make -C systemctrl clean

_pentazemin: | _binary
	make -C pentazemin clean
	make -C pentazemin -j
ifdef USE_PSP_PACKER
	psp-packer pentazemin/pentazemin.prx -o ../user/flash0/kd/pentazemin.prx
else
	$(PY) $(PSPGZ) ../user/flash0/kd/pentazemin.prx $(KHDR) pentazemin/pentazemin.prx Pentazemin 0x3007
endif
	make -C pentazemin clean

_popcorn:
	make -C popcorn clean
	make -C popcorn -j
ifdef USE_PSP_PACKER
	psp-packer popcorn/popcorn.prx -o ../user/flash0/kd/popcorn.prx
else
	$(PY) $(PSPGZ) ../user/flash0/kd/popcorn.prx $(KHDR) popcorn/popcorn.prx PopcornManager 0x3007
endif
	make -C popcorn clean

_galaxy:
	make -C galaxy clean
	make -C galaxy -j
ifdef USE_PSP_PACKER
	psp-packer galaxy/galaxy.prx -o ../user/flash0/kd/galaxy.prx
else
	$(PY) $(PSPGZ) ../user/flash0/kd/galaxy.prx $(KHDR) galaxy/galaxy.prx GalaxyController 0x3007
endif
	make -C galaxy clean

_march33:
	make -C march33 clean
	make -C march33 -j
ifdef USE_PSP_PACKER
	psp-packer march33/march33.prx -o ../user/flash0/kd/march33.prx
else
	$(PY) $(PSPGZ) ../user/flash0/kd/march33.prx $(KHDR) march33/march33.prx March33Driver 0x3007
endif
	make -C march33 clean

_inferno:
	make -C inferno clean
	make -C inferno -j
ifdef USE_PSP_PACKER
	psp-packer inferno/inferno.prx -o ../user/flash0/kd/inferno.prx
else
	$(PY) $(PSPGZ) ../user/flash0/kd/inferno.prx $(KHDR) inferno/inferno.prx InfernoDriver 0x3007
endif
	make -C inferno clean

_vshctrl:
	make -C vshctrl clean
	make -C vshctrl -j
ifdef USE_PSP_PACKER
	psp-packer vshctrl/vshctrl.prx -o ../user/flash0/kd/vshctrl.prx
else
	$(PY) $(PSPGZ) ../user/flash0/kd/vshctrl.prx $(KHDR) vshctrl/vshctrl.prx VshControl 0x3007
endif
	make -C vshctrl clean

_kermit_idstorage:
	make -C kermit_idstorage clean
	make -C kermit_idstorage -j
ifdef USE_PSP_PACKER
	psp-packer kermit_idstorage/kermit_idstorage.prx -o ../user/flash0/kd/kermit_idstorage.prx
else
	$(PY) $(PSPGZ) ../user/flash0/kd/kermit_idstorage.prx $(KHDR) kermit_idstorage/kermit_idstorage.prx Kermit_IdStorage 0x3007
endif
	make -C kermit_idstorage clean

_xmbctrl:
	make -C xmbctrl clean
	make -C xmbctrl -j
ifdef USE_PSP_PACKER
	psp-packer xmbctrl/xmbctrl.prx -o ../user/flash0/vsh/module/xmbctrl.prx
else
	$(PY) $(PSPGZ) ../user/flash0/vsh/module/xmbctrl.prx $(UHDR) xmbctrl/xmbctrl.prx XmbControl 0x0000
endif
	make -C xmbctrl clean

_satelite:
	make -C satelite clean
	make -C satelite -j
ifdef USE_PSP_PACKER
	psp-packer satelite/satelite.prx -o ../user/flash0/vsh/module/satelite.prx
else
	$(PY) $(PSPGZ) ../user/flash0/vsh/module/satelite.prx $(UHDR) satelite/satelite.prx Satelite 0x0000
endif
	make -C satelite clean

_recovery:
	make -C recovery clean
	make -C recovery -j
ifdef USE_PSP_PACKER
	psp-packer recovery/recovery.prx -o ../user/flash0/vsh/module/recovery.prx
else
	$(PY) $(PSPGZ) ../user/flash0/vsh/module/recovery.prx $(UHDR) recovery/recovery.prx Recovery 0x0000
endif
	make -C recovery clean

clean:
	make -C btcnf/pspbtcnf_inferno clean
	make -C btcnf/pspbtcnf_march33 clean
	make -C btcnf/pspbtcnf_normal clean
	make -C btcnf/pspbtcnf_np9660 clean
	make -C btcnf/pspbtcnf_recovery clean
	make -C payloadex clean
	make -C rebootex clean
	make -C payloadex_ark clean
	make -C binary clean
	make -C systemctrl clean
	make -C pentazemin clean
	make -C popcorn clean
	make -C galaxy clean
	make -C inferno clean
	make -C vshctrl clean
	make -C kermit_idstorage clean
	make -C xmbctrl clean
	make -C satelite clean
	make -C recovery clean
	rm -f pentazemin/binary.h systemctrl/rebootex.h