all: _libs _payloadex _rebootex _binary _systemctrl _popcorn _inferno _vshctrl _satelite _recovery _kermit_idstorage _btcnf _xmbctrl

SYSTEM_STUBS = pspamctrl_driver pspsemaphore pspsysmem_kernel pspvshbridge_user psppower_user pspreg_user psppaf pspvsh pspvshcommongui

_libs:
	make -C lib/system_stubs clean
	make -C lib/system_stubs -j $(SYSTEM_STUBS:%=.%)
	cp lib/system_stubs/*.a "lib/"

	make -C lib/libpspkubridge clean
	make -C lib/libpspkubridge
	cp "lib/libpspkubridge/libpspkubridge.a" "lib/"

	make -C lib/libpspsystemctrl_kernel clean
	make -C lib/libpspsystemctrl_kernel
	cp "lib/libpspsystemctrl_kernel/libpspsystemctrl_kernel.a" "lib/"

	make -C lib/libpspsystemctrl_user clean
	make -C lib/libpspsystemctrl_user
	cp "lib/libpspsystemctrl_user/libpspsystemctrl_user.a" "lib/"

	make -C lib/libpspvshctrl clean
	make -C lib/libpspvshctrl
	cp "lib/libpspvshctrl/libpspvshctrl.a" "lib/"

	make -C lib/libpspsysc_user clean
	make -C lib/libpspsysc_user
	cp "lib/libpspsysc_user/libpspsysc_user.a" "lib/"

	make -C lib/system_stubs clean
	make -C lib/libpspkubridge clean
	make -C lib/libpspsystemctrl_kernel clean
	make -C lib/libpspsystemctrl_user clean
	make -C lib/libpspvshctrl clean
	make -C lib/libpspsysc_user clean

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
	make -C payloadex
	cp "payloadex/payloadex.bin" "../user/flash0/payloadex.bin"
	make -C payloadex clean

_rebootex:
	make -C rebootex clean
	make -C rebootex
	bin2c rebootex/rebootex.bin systemctrl/rebootex.h rebootex
	make -C rebootex clean

_binary:
	make -C binary clean
	make -C binary
	bin2c binary/binary.bin systemctrl/binary.h binary
	make -C binary clean

_systemctrl: | _libs _rebootex _binary
	make -C systemctrl clean
	make -C systemctrl
	psp-packer systemctrl/systemctrl.prx
	cp "systemctrl/systemctrl.prx" "../user/flash0/kd/systemctrl.prx"
	make -C systemctrl clean

_popcorn: | _libs
	make -C popcorn clean
	make -C popcorn
	psp-packer popcorn/popcorn.prx
	cp "popcorn/popcorn.prx" "../user/flash0/kd/popcorn.prx"
	make -C popcorn clean

_inferno: | _libs
	make -C inferno clean
	make -C inferno
	psp-packer inferno/inferno.prx
	cp "inferno/inferno.prx" "../user/flash0/kd/inferno.prx"
	make -C inferno clean

_vshctrl: | _libs
	make -C vshctrl clean
	make -C vshctrl
	psp-packer vshctrl/vshctrl.prx
	cp "vshctrl/vshctrl.prx" "../user/flash0/kd/vshctrl.prx"
	make -C vshctrl clean

_kermit_idstorage: | _libs
	make -C kermit_idstorage clean
	make -C kermit_idstorage
	psp-packer kermit_idstorage/kermit_idstorage.prx
	cp "kermit_idstorage/kermit_idstorage.prx" "../user/flash0/kd/kermit_idstorage.prx"
	make -C kermit_idstorage clean

_xmbctrl: | _libs
	make -C xmbctrl clean
	make -C xmbctrl
	psp-packer xmbctrl/xmbctrl.prx
	cp "xmbctrl/xmbctrl.prx" "../user/flash0/vsh/module/xmbctrl.prx"
	make -C xmbctrl clean

_satelite: | _libs
	make -C satelite clean
	make -C satelite
	psp-packer satelite/satelite.prx
	cp "satelite/satelite.prx" "../user/flash0/vsh/module/satelite.prx"
	make -C satelite clean

_recovery: | _libs
	make -C newrecovery clean
	make -C newrecovery
	psp-packer newrecovery/recovery.prx
	cp "newrecovery/recovery.prx" "../user/flash0/vsh/module/recovery.prx"
	make -C newrecovery clean

clean:
	make -C lib/libpspkubridge clean
	make -C lib/libpspsystemctrl_kernel clean
	make -C lib/libpspsystemctrl_user clean
	make -C lib/libpspvshctrl clean
	make -C lib/libpspsysc_user clean
	make -C btcnf/pspbtcnf_inferno clean
	make -C btcnf/pspbtcnf_march33 clean
	make -C btcnf/pspbtcnf_normal clean
	make -C btcnf/pspbtcnf_np9660 clean
	make -C btcnf/pspbtcnf_recovery clean
	make -C payloadex clean
	make -C rebootex clean
	make -C binary clean
	make -C systemctrl clean
	make -C popcorn clean
	make -C inferno clean
	make -C vshctrl clean
	make -C kermit_idstorage clean
	make -C xmbctrl clean
	make -C satelite clean
	make -C newrecovery clean