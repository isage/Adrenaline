add_cef_prx(xmbctrl USER
  SRC_FILES
    main.c
    list.c
    plugins.c
    utils.c
    exports.c
    config.c
    patch_vsh.c
  COMPILE_OPS
    -std=c99
  LINK_LIBS
    pspsystemctrl_user
    pspsysc_user
    pspkubridge
    pspvshctrl
    psppaf
    pspvsh 
    pspvshcommongui
)

target_include_directories(vshctrl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

copy_prx_flash0(xmbctrl XmbControl USER)