	.set noreorder

#include "pspstub.s"

	STUB_START "pspPSAR",0x40090000,0x00040005
	STUB_FUNC  0xD31CB80A,pspDecryptTable
	STUB_END
