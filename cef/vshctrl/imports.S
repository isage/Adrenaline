	.set noreorder

#include "pspimport.s"
	IMPORT_START "SysclibForUser",0x40090000
	IMPORT_FUNC  "SysclibForUser",0xBC7554DF,strcasecmp

	IMPORT_START "SystemCtrlForUser",0x40090000
	IMPORT_FUNC  "SystemCtrlForUser",0x80C0ED7B,sctrlStartUsb
	IMPORT_FUNC  "SystemCtrlForUser",0x5FC12767,sctrlStopUsb
	IMPORT_FUNC  "SystemCtrlForUser",0x05D8E209,sctrlGetUsbState
