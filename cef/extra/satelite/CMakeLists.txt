add_cef_prx(satelite USER
  SRC_FILES
    src/main.c
  LINK_STATICS
    pspsdk
	pspmodinfo
	ya2d
	tinyfont
	pspmath
	pspvram
  LINK_STUBS
	guglue
	pspcpaf
    pspreg
    psppower
	pspdisplay
    pspvshctrl
    pspsystemctrl_user
)

copy_prx_flash0(satelite Satelite USER)