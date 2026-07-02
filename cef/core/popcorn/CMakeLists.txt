add_cef_prx(popcorn
  SRC_FILES
    src/main.c
    src/syspatches.c
    src/libcrypt.c
  LINK_STATICS
    pspsdk
	pspmodinfo
  LINK_STUBS
    pspsemaphore
    pspinit_kernel
    psputils_kernel
    pspamctrl_driver
    pspsysclib_kernel
    psploadcore_kernel
    pspiofilemgr_kernel
    pspthreadman_kernel
    pspsystemctrl_kernel
    pspstdio_kernel
)

copy_prx_flash0(popcorn PopcornManager)