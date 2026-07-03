PY = $(shell which python3)
PSPDEV = $(shell psp-config --pspdev-path)
CFWSDK = $(PSPDEV)/share/psp-cfw-sdk
PSPGZ = $(CFWSDK)/build-tools/gz/pspgz.py
KHDR = $(CFWSDK)/build-tools/gz/SystemControl.hdr
UHDR = $(CFWSDK)/build-tools/gz/UserModule.hdr

all: _payloadex _payloadex_ark _rebootex _binary _systemctrl _pentazemin _popcorn _galaxy _march33 _inferno _me _vshctrl _satelite _recovery _kermit_idstorage _btcnf _xmbctrl

KERNEL_STUBS = pspamctrl_driver pspwlandrv_driver pspkermitperipheral_driver psppower_driver pspreg_driver pspchkreg_driver pspusb_driver pspdisplay_driver pspusbcam_driver pspthreadman_kernel psploadcore_kernel pspmodulemgr_kernel pspkermit_driver pspinit_kernel pspsysclib_kernel psploadexec_kernel pspinterruptmanager_kernel pspsemaphore pspsysmem_kernel

USER_STUBS = psppower pspreg psppaf pspvsh pspvshcommongui

SYSTEM_STUBS = $(KERNEL_STUBS) $(USER_STUBS)

_btcnf:
	make -C core/btcnf/pspbtcnf_inferno clean
	make -C core/btcnf/pspbtcnf_inferno
	cp "core/btcnf/pspbtcnf_inferno/pspbtcnf.bin" "../user/flash0/kd/pspbtknf.bin"
	make -C core/btcnf/pspbtcnf_inferno clean

	make -C core/btcnf/pspbtcnf_march33 clean
	make -C core/btcnf/pspbtcnf_march33
	cp "core/btcnf/pspbtcnf_march33/pspbtcnf.bin" "../user/flash0/kd/pspbtlnf.bin"
	make -C core/btcnf/pspbtcnf_march33 clean

	make -C core/btcnf/pspbtcnf_me clean
	make -C core/btcnf/pspbtcnf_me
	cp "core/btcnf/pspbtcnf_me/pspbtcnf.bin" "../user/flash0/kd/pspbtnnf.bin"
	make -C core/btcnf/pspbtcnf_me clean

	make -C core/btcnf/pspbtcnf_normal clean
	make -C core/btcnf/pspbtcnf_normal
	cp "core/btcnf/pspbtcnf_normal/pspbtcnf.bin" "../user/flash0/kd/pspbtjnf.bin"
	make -C core/btcnf/pspbtcnf_normal clean

	make -C core/btcnf/pspbtcnf_np9660 clean
	make -C core/btcnf/pspbtcnf_np9660
	cp "core/btcnf/pspbtcnf_np9660/pspbtcnf.bin" "../user/flash0/kd/pspbtmnf.bin"
	make -C core/btcnf/pspbtcnf_np9660 clean

	make -C core/btcnf/pspbtcnf_recovery clean
	make -C core/btcnf/pspbtcnf_recovery
	cp "core/btcnf/pspbtcnf_recovery/pspbtcnf.bin" "../user/flash0/kd/pspbtrnf.bin"
	make -C core/btcnf/pspbtcnf_recovery clean

_payloadex:
ifdef USE_OLD_BOOTLOADEX
	make -C core/payloadex_old clean
	make -C core/payloadex_old -j
	cp "core/payloadex_old/payloadex.bin" "../user/flash0/payloadex.bin"
	make -C core/payloadex_old clean
else
	make -C core/payloadex clean
	make -C core/payloadex -j
	cp "core/payloadex/payloadex.bin" "../user/flash0/payloadex.bin"
	make -C core/payloadex clean
endif

_payloadex_ark:
	make -C core/payloadex_ark clean
	make -C core/payloadex_ark -j
	cp "core/payloadex_ark/payloadex_ark.bin" "../user/flash0/payloadex_ark.bin"
	make -C core/payloadex_ark clean

_rebootex:
ifdef USE_OLD_BOOTLOADEX
	make -C core/rebootex_old clean
	make -C core/rebootex_old -j
	bin2c core/rebootex_old/rebootex.bin core/systemctrl/rebootex.h rebootex
	make -C core/rebootex_old clean
else
	make -C core/rebootex clean
	make -C core/rebootex -j
	bin2c core/rebootex/rebootex.bin core/systemctrl/rebootex.h rebootex
	make -C core/rebootex clean
endif

_binary:
	make -C core/binary clean
	make -C core/binary -j
	bin2c core/binary/binary.bin core/pentazemin/binary.h binary
	make -C core/binary clean

_systemctrl: | _rebootex _binary
	make -C core/systemctrl clean
	make -C core/systemctrl -j
ifdef USE_PSP_PACKER
	psp-packer core/systemctrl/systemctrl.prx -o ../user/flash0/kd/systemctrl.prx
else
	$(PY) $(PSPGZ) ../user/flash0/kd/systemctrl.prx $(KHDR) core/systemctrl/systemctrl.prx SystemControl 0x3007
endif
	make -C core/systemctrl clean

_pentazemin: | _binary
	make -C core/pentazemin clean
	make -C core/pentazemin -j
ifdef USE_PSP_PACKER
	psp-packer core/pentazemin/pentazemin.prx -o ../user/flash0/kd/pentazemin.prx
else
	$(PY) $(PSPGZ) ../user/flash0/kd/pentazemin.prx $(KHDR) core/pentazemin/pentazemin.prx Pentazemin 0x3007
endif
	make -C core/pentazemin clean

_popcorn:
	make -C core/popcorn clean
	make -C core/popcorn -j
ifdef USE_PSP_PACKER
	psp-packer core/popcorn/popcorn.prx -o ../user/flash0/kd/popcorn.prx
else
	$(PY) $(PSPGZ) ../user/flash0/kd/popcorn.prx $(KHDR) core/popcorn/popcorn.prx PopcornManager 0x3007
endif
	make -C core/popcorn clean

_galaxy:
	make -C iso_drivers/galaxy clean
	make -C iso_drivers/galaxy -j
ifdef USE_PSP_PACKER
	psp-packer iso_drivers/galaxy/galaxy.prx -o ../user/flash0/kd/galaxy.prx
else
	$(PY) $(PSPGZ) ../user/flash0/kd/galaxy.prx $(KHDR) iso_drivers/galaxy/galaxy.prx GalaxyController 0x3007
endif
	make -C iso_drivers/galaxy clean

_march33:
	make -C iso_drivers/march33 clean
	make -C iso_drivers/march33 -j
ifdef USE_PSP_PACKER
	psp-packer iso_drivers/march33/march33.prx -o ../user/flash0/kd/march33.prx
else
	$(PY) $(PSPGZ) ../user/flash0/kd/march33.prx $(KHDR) iso_drivers/march33/march33.prx March33Driver 0x3007
endif
	make -C iso_drivers/march33 clean

_inferno:
	make -C iso_drivers/inferno clean
	make -C iso_drivers/inferno -j
ifdef USE_PSP_PACKER
	psp-packer iso_drivers/inferno/inferno.prx -o ../user/flash0/kd/inferno.prx
else
	$(PY) $(PSPGZ) ../user/flash0/kd/inferno.prx $(KHDR) iso_drivers/inferno/inferno.prx InfernoDriver 0x3007
endif
	make -C iso_drivers/inferno clean

_me:
	make -C iso_drivers/minimal_edition clean
	make -C iso_drivers/minimal_edition -j
ifdef USE_PSP_PACKER
	psp-packer iso_drivers/minimal_edition/minimal_edition.prx -o ../user/flash0/kd/minimal_edition.prx
else
	$(PY) $(PSPGZ) ../user/flash0/kd/minimal_edition.prx $(KHDR) iso_drivers/minimal_edition/minimal_edition.prx InfernoDriver 0x3007
endif
	make -C iso_drivers/minimal_edition clean

_vshctrl:
	make -C core/vshctrl clean
	make -C core/vshctrl -j
ifdef USE_PSP_PACKER
	psp-packer core/vshctrl/vshctrl.prx -o ../user/flash0/kd/vshctrl.prx
else
	$(PY) $(PSPGZ) ../user/flash0/kd/vshctrl.prx $(KHDR) core/vshctrl/vshctrl.prx VshControl 0x3007
endif
	make -C core/vshctrl clean

_kermit_idstorage:
	make -C core/kermit_idstorage clean
	make -C core/kermit_idstorage -j
ifdef USE_PSP_PACKER
	psp-packer core/kermit_idstorage/kermit_idstorage.prx -o ../user/flash0/kd/kermit_idstorage.prx
else
	$(PY) $(PSPGZ) ../user/flash0/kd/kermit_idstorage.prx $(KHDR) core/kermit_idstorage/kermit_idstorage.prx Kermit_IdStorage 0x3007
endif
	make -C core/kermit_idstorage clean

_xmbctrl:
	make -C core/xmbctrl clean
	make -C core/xmbctrl -j
ifdef USE_PSP_PACKER
	psp-packer core/xmbctrl/xmbctrl.prx -o ../user/flash0/vsh/module/xmbctrl.prx
else
	$(PY) $(PSPGZ) ../user/flash0/vsh/module/xmbctrl.prx $(UHDR) core/xmbctrl/xmbctrl.prx XmbControl 0x0000
endif
	make -C core/xmbctrl clean

_satelite:
	make -C extra/satelite clean
	make -C extra/satelite -j
ifdef USE_PSP_PACKER
	psp-packer extra/satelite/satelite.prx -o ../user/flash0/vsh/module/satelite.prx
else
	$(PY) $(PSPGZ) ../user/flash0/vsh/module/satelite.prx $(UHDR) extra/satelite/satelite.prx Satelite 0x0000
endif
	make -C extra/satelite clean

_recovery:
	make -C core/recovery clean
	make -C core/recovery -j
ifdef USE_PSP_PACKER
	psp-packer core/recovery/recovery.prx -o ../user/flash0/vsh/module/recovery.prx
else
	$(PY) $(PSPGZ) ../user/flash0/vsh/module/recovery.prx $(UHDR) core/recovery/recovery.prx Recovery 0x0000
endif
	make -C core/recovery clean

clean:
	make -C core/btcnf/pspbtcnf_inferno clean
	make -C core/btcnf/pspbtcnf_march33 clean
	make -C core/btcnf/pspbtcnf_me clean
	make -C core/btcnf/pspbtcnf_normal clean
	make -C core/btcnf/pspbtcnf_np9660 clean
	make -C core/btcnf/pspbtcnf_recovery clean
	make -C core/payloadex clean
	make -C core/rebootex clean
	make -C core/payloadex_old clean
	make -C core/rebootex_old clean
	make -C core/payloadex_ark clean
	make -C core/binary clean
	make -C core/systemctrl clean
	make -C core/pentazemin clean
	make -C core/popcorn clean
	make -C core/vshctrl clean
	make -C core/kermit_idstorage clean
	make -C core/recovery clean
	make -C iso_drivers/galaxy clean
	make -C iso_drivers/march33 clean
	make -C iso_drivers/inferno clean
	make -C iso_drivers/minimal_edition clean
	make -C extra/xmbctrl clean
	make -C extra/satelite clean
	rm -f pentazemin/binary.h systemctrl/rebootex.h