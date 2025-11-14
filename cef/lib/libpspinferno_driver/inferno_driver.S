	.set noreorder

#include "pspimport.s"

// Build files
// inferno_driver_XXXX.o

#ifdef F_inferno_driver_0000
	IMPORT_START "inferno_driver",0x00090011
#endif
#ifdef F_inferno_driver_0001
	IMPORT_FUNC  "inferno_driver",0xCF8299BE,infernoSetDiscType
#endif
#ifdef F_inferno_driver_0002
	IMPORT_FUNC  "inferno_driver",0xC0736FD6,infernoCacheSetPolicy
#endif
#ifdef F_inferno_driver_0003
	IMPORT_FUNC  "inferno_driver",0x8CDE7F95,infernoCacheInit
#endif
#ifdef F_inferno_driver_0004
	IMPORT_FUNC_WITH_ALIAS  "inferno_driver",0xB6522E93,infernoSetUmdDelay,isoSetUmdDelay
#endif
#ifdef F_inferno_driver_0005
	IMPORT_FUNC_WITH_ALIAS  "inferno_driver",0xFAEC97D6,infernoSetUmdDelay,isoSetUmdDelay
#endif
#ifdef F_inferno_driver_0006
	IMPORT_FUNC_WITH_ALIAS  "inferno_driver",0xB573209C,isoReadUmdFile,infernoIsoRead
#endif
#ifdef F_inferno_driver_0007
	IMPORT_FUNC_WITH_ALIAS  "inferno_driver",0x3F83B110,isoReadUmdFile,infernoIsoRead
#endif
#ifdef F_inferno_driver_0008
	IMPORT_FUNC  "inferno_driver",0xD4FAB33F,isoGetTitleId
#endif
