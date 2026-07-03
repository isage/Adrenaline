add_cef_prx(satelite_classic USER
  SRC_FILES
    src/main.c
    src/menu.c
    src/blit.c
  LINK_STATICS
    pspsdk
	pspmodinfo
  LINK_STUBS
    pspdebug
    psppower
    pspdisplay
    pspvshctrl
    pspsysc_user
    pspsystemctrl_user
)

copy_prx_flash0(satelite_classic SateliteClassic USER)