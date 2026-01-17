	.set noreorder

#include "pspimport.s"
	IMPORT_START "SysclibForUser",0x40090000
	IMPORT_FUNC  "SysclibForUser",0xBC7554DF,strcasecmp
