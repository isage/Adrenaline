add_cef_prx(pentazemin
  SRC_FILES
    src/main.c
    src/epictrl.c
    src/usbcam_patch.c
    src/vitamem.c
    src/modpatches.c
    src/io_patch.c
    src/adrenaline.c
    src/init_patch.c
    src/power_patch.c
    src/LflashFatfmt.c
    src/impose_patch.c
    src/utility_patch.c
    ../../bits/ttystdio.c
    binary.h
  LINK_STATICS
    pspsdk
	pspmodinfo
  LINK_STUBS
    pspusb_driver
    pspinit_kernel
    pspstdio_kernel
    psppower_driver
    psputils_kernel
    pspsysmem_kernel
    pspkermit_driver
    pspusbcam_driver
    pspdisplay_driver
    pspsysclib_kernel
    pspsysevent_kernel
    psploadcore_kernel
    pspthreadman_kernel
    pspmodulemgr_kernel
    pspiofilemgr_kernel
    pspinterruptmanager_kernel
    pspsystemctrl_kernel
)

target_include_directories(pentazemin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(OUTPUT binary.h
  DEPENDS binary
  COMMAND bin2c "$<TARGET_FILE:binary>.bin" binary.h binary
)

copy_prx_flash0(pentazemin Pentazemin)
