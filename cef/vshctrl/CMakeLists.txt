add_cef_prx(vshctrl
  SRC_FILES
    main.c
    imports.S
    isofs_driver/isofs_driver.c
    isofs_driver/umd9660_driver.c
    patch_io.c
    patch_vsh.c
    virtualpbpmgr.c
    virtualsfo.c
    vshctrl.c
    ../bits/iso_common.c
  LINK_LIBS
    cisoread
    pspsystemctrl_kernel
    adrenalinectrl
    psppower_driver
    psprtc_driver
    pspusb_driver
    pspreg_driver
    pspsysclib_kernel
    pspsysmem_kernel
    pspinit_kernel
    pspctrl_driver
)

target_include_directories(vshctrl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(vshctrl PRIVATE __USE_USER_ALLOC)

copy_prx_flash0(vshctrl VshControl)