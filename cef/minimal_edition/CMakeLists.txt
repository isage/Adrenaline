add_cef_prx(minimal_edition
  SRC_FILES
    main.c
    sceumd.c
    sceumdman.c
    umd9660_march_driver.c
    ../bits/iso_common.c
  LINK_LIBS
    cisoread
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

copy_prx_flash0(minimal_edition MEisoDriver)