all: _payloadex _rebootex _binary _systemctrl _popcorn _inferno _vshctrl _satelite _recovery _btcnf

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

_systemctrl:
	make -C systemctrl clean
	make -C systemctrl
	psp-packer systemctrl/systemctrl.prx
	cp "systemctrl/systemctrl.prx" "../user/flash0/kd/systemctrl.prx"
	make -C systemctrl clean

_popcorn:
	make -C popcorn clean
	make -C popcorn
	psp-packer popcorn/popcorn.prx
	cp "popcorn/popcorn.prx" "../user/flash0/kd/popcorn.prx"
	make -C popcorn clean

_inferno:
	make -C inferno clean
	make -C inferno
	psp-packer inferno/inferno.prx
	cp "inferno/inferno.prx" "../user/flash0/kd/inferno.prx"
	make -C inferno clean

_vshctrl:
	make -C vshctrl clean
	make -C vshctrl
	psp-packer vshctrl/vshctrl.prx
	cp "vshctrl/vshctrl.prx" "../user/flash0/kd/vshctrl.prx"
	make -C vshctrl clean

_satelite:
	make -C satelite clean
	make -C satelite
	psp-packer satelite/satelite.prx
	cp "satelite/satelite.prx" "../user/flash0/vsh/module/satelite.prx"
	make -C satelite clean

_recovery:
	make -C recovery clean
	make -C recovery
	psp-packer recovery/recovery.prx
	cp "recovery/recovery.prx" "../user/flash0/vsh/module/recovery.prx"
	make -C recovery clean
