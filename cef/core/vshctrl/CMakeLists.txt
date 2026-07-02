add_cef_prx(vshctrl
  SRC_FILES
    src/main.c
    src/isofs_driver/isofs_driver.c
    src/isofs_driver/umd9660_driver.c
    src/patch_io.c
    src/patch_vsh.c
    src/virtualpbpmgr.c
    src/virtualsfo.c
    src/vshctrl.c
    imports.S
    ../../bits/iso_common.c
  LINK_STATICS
    pspsdk
	pspmodinfo
    cisoread
  LINK_STUBS
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
	pspkernel
)

target_include_directories(vshctrl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(vshctrl PRIVATE __USE_USER_ALLOC)

copy_prx_flash0(vshctrl VshControl)