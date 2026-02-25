add_cef_prx(satelite USER
  SRC_FILES
    main.c
    menu.c
    blit.c
  LINK_LIBS
	pspdebug
    pspsystemctrl_user
    pspsysc_user
    pspvshctrl
    psppower
)

copy_prx_flash0(satelite Satelite USER)