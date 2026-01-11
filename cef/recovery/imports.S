	.set noreorder

#include "pspimport.s"

	IMPORT_START "SystemCtrlForUser",0x40090000
	IMPORT_FUNC  "SystemCtrlForUser",0x80C0ED7B,sctrlStartUsb
	IMPORT_FUNC  "SystemCtrlForUser",0x5FC12767,sctrlStopUsb
	IMPORT_FUNC  "SystemCtrlForUser",0x05D8E209,sctrlGetUsbState

	IMPORT_START "SysclibForUser",0x40090000
	IMPORT_FUNC  "SysclibForUser",0x4C0FE24C,user_malloc
	IMPORT_FUNC  "SysclibForUser",0x12BC667F,user_free

#include "pspstub.s"

	# why is pspDecryptTable in pspPSAR? it should be in pspDecrypt
	STUB_START "pspPSAR",0x40090000,0x00040005
	STUB_FUNC  0xD31CB80A,pspDecryptTable
