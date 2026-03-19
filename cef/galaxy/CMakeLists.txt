add_cef_prx(galaxy
  SRC_FILES
    main.c
    syspatches.c
    ../bits/iso_common.c
  LINK_LIBS
    cisoread
    pspinit_kernel
    psputils_kernel
    pspsysmem_kernel
    pspkernel_library
    pspsysclib_kernel
    psploadcore_kernel
    pspthreadman_kernel
    pspiofilemgr_kernel
    pspsystemctrl_kernel
    pspstdio_kernel
)

copy_prx_flash0(galaxy GalaxyController)