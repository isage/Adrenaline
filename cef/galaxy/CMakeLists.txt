add_cef_prx(galaxy
  SRC_FILES
    main.c
    syspatches.c
    ../bits/iso_common.c
  LINK_LIBS
    cisoread
    pspsystemctrl_kernel
    pspsysmem_kernel
    pspsysclib_kernel
    pspinit_kernel
)

copy_prx_flash0(galaxy GalaxyController)