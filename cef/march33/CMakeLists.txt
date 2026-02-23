add_cef_prx(march33
  SRC_FILES
    main.c
    umdman.c
    mediaman.c
    umd9660.c
    ../bits/iso_common.c
  LINK_LIBS
    cisoread
    pspsysmem_kernel
    pspsystemctrl_kernel
    pspthreadman_kernel
    pspinit_kernel
)

copy_prx_flash0(march33 March33Driver)