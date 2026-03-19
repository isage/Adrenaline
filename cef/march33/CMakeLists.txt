add_cef_prx(march33
  SRC_FILES
    main.c
    umdman.c
    mediaman.c
    umd9660.c
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

copy_prx_flash0(march33 March33Driver)