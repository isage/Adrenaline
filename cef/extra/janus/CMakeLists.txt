add_cef_prx(janus
  SRC_FILES
    src/main.c
    src/blit.c
	imports.S
  LINK_STATICS
    pspsdk
    tinyfont
  LINK_STUBS
    pspkernel
    pspctrl_driver
    pspsysclib_kernel
    pspdisplay_driver
    pspisoctrl_driver
	pspreg_driver
    pspsystemctrl_kernel
)

target_include_directories(vshctrl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

copy_prx_flash0(janus Janus)