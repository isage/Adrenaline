add_cef_prx(kermit_idstorage
  SRC_FILES
    main.c
  LINK_LIBS
    pspwlandrv_driver
    pspkermitperipheral_driver
    pspmodinfo
    pspsdk
    pspkernel
)

copy_prx_flash0(kermit_idstorage Kermit_IdStorage)