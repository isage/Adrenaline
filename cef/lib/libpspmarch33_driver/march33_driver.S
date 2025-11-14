	.set noreorder

#include "pspimport.s"

// Build files
// march33_driver_XXXX.o

#ifdef F_march33_driver_0000
	IMPORT_START "march33_driver",0x00090011
#endif
#ifdef F_march33_driver_0001
	IMPORT_FUNC  "march33_driver",0xFAEC97D6,isoSetUmdDelay
#endif
#ifdef F_march33_driver_0002
	IMPORT_FUNC  "march33_driver",0xB6522E93,isoSetUmdDelay
#endif
#ifdef F_march33_driver_0003
	IMPORT_FUNC  "march33_driver",0xD4FAB33F,isoGetTitleId
#endif
#ifdef F_march33_driver_0004
	IMPORT_FUNC  "march33_driver",0x3F83B110,isoReadUmdFile
#endif
