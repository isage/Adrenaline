add_cef_prx(inferno
  SRC_FILES
    main.c
    iodrv_funcs.c
    umd.c
    isoread.c
    isocache.c
    ../bits/iso_common.c
  LINK_LIBS
    cisoread
    pspsystemctrl_kernel
    pspsemaphore
    pspsysmem_kernel
    pspinit_kernel
)

copy_prx_flash0(inferno InfernoDriver)