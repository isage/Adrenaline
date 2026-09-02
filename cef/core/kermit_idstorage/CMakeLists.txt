add_cef_prx(kermit_idstorage
  SRC_FILES
    main.c
  LINK_STATICS
    pspsdk
  LINK_STUBS
    pspkernel
    pspwlandrv_driver
    pspkermitperipheral_driver
)

copy_prx_flash0(kermit_idstorage Kermit_IdStorage)