	.set noreorder

#include "pspimport.s"

// Build files
// isoCtrl_driver_XXXX.o

#ifdef F_isoCtrl_driver_0000
	IMPORT_START "isoCtrl_driver",0x00090011
#endif
#ifdef F_isoCtrl_driver_0001
	IMPORT_FUNC  "isoCtrl_driver",0xD4FAB33F,isoGetTitleId
#endif
#ifdef F_isoCtrl_driver_0002
	IMPORT_FUNC  "isoCtrl_driver",0xFAEC97D6,isoSetUmdDelay
#endif
#ifdef F_isoCtrl_driver_0003
	IMPORT_FUNC  "isoCtrl_driver",0x3F83B110,isoReadUmdFile
#endif
