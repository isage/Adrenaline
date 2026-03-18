add_cef_prx(pentazemin
  SRC_FILES
    main.c
    epictrl.c
    usbcam_patch.c
    vitamem.c
    modpatches.c
    io_patch.c
    adrenaline.c
    init_patch.c
    power_patch.c
    LflashFatfmt.c
    impose_patch.c
    utility_patch.c
    ../bits/ttystdio.c
    binary.h
  LINK_LIBS
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
