add_cef_prx(march33
  SRC_FILES
    src/main.c
    src/umdman.c
    src/mediaman.c
    src/umd9660.c
    ../../bits/iso_common.c
  LINK_STATICS
    pspsdk
	pspmodinfo
    cisoread
  LINK_STUBS
    pspsemaphore
    pspinit_kernel
    psputils_kernel
    pspsysmem_kernel
    pspkernel_library
    pspsysclib_kernel
    pspsysevent_kernel
    psploadcore_kernel
    pspiofilemgr_kernel
    pspthreadman_kernel
    pspsystemctrl_kernel
    pspstdio_kernel
)

copy_prx_flash0(march33 March33Driver)