add_cef_prx(pentazemin
  SRC_FILES
    main.c
    src/epictrl.c
    ../bits/ttystdio.c
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
    binary.h
  LINK_LIBS
    pspsystemctrl_kernel
    pspinit_kernel
    pspthreadman_kernel
    pspsysclib_kernel
    psploadcore_kernel
    pspmodulemgr_kernel
    pspusbcam_driver
    pspusb_driver
    psppower_driver
    pspdisplay_driver
    pspkermit_driver
    pspsysmem_kernel
)

target_include_directories(pentazemin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(OUTPUT binary.h
  DEPENDS binary
  COMMAND bin2c "$<TARGET_FILE:binary>.bin" binary.h binary
)

copy_prx_flash0(pentazemin Pentazemin)
