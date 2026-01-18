	.set noreorder

#include "pspimport.s"
	IMPORT_START "SystemCtrlForKernel",0x00090000
	IMPORT_FUNC  "SystemCtrlForKernel",0x3EB35691,strcasecmp
	IMPORT_FUNC  "SystemCtrlForKernel",0x12BC667F,user_free
	IMPORT_FUNC  "SystemCtrlForKernel",0x4C0FE24C,user_malloc
	IMPORT_FUNC  "SystemCtrlForKernel",0xB19C939A,user_memalign