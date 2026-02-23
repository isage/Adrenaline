add_cef_prx(popcorn
  SRC_FILES
    main.c
    syspatches.c
    libcrypt.c
  LINK_LIBS
    pspsystemctrl_kernel
    pspsemaphore
    pspamctrl_driver
)

copy_prx_flash0(popcorn PopcornManager)