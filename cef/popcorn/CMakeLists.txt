add_cef_prx(popcorn
  SRC_FILES
    main.c
    syspatches.c
    libcrypt.c
  LINK_LIBS
    pspsemaphore
    pspinit_kernel
    psputils_kernel
    pspamctrl_driver
    pspsysclib_kernel
    psploadcore_kernel
    pspiofilemgr_kernel
    pspsystemctrl_kernel
)

copy_prx_flash0(popcorn PopcornManager)