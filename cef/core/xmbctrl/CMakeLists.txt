add_cef_prx(xmbctrl USER
  SRC_FILES
    src/main.c
    src/list.c
    src/plugins.c
    src/utils.c
    src/config.c
	src/patch_io.c
    src/patch_vsh.c
    src/patch_game_plugin.c
    src/patch_savedata_plugin.c
    # exports.c
  COMPILE_OPS
    -std=c99
  LINK_STATICS
    pspsdk
	pspmodinfo
  LINK_STUBS
    psppaf
    pspvsh
    pspvshctrl
    pspkubridge
    pspsysc_user
    pspvshcommongui
    pspsystemctrl_user
	adrenalinectrl
)

target_include_directories(vshctrl PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

copy_prx_flash0(xmbctrl XmbControl USER)