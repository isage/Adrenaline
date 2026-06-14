add_cef_prx(satelite USER
  SRC_FILES
    src/main.c
    src/menu.c
    src/blit.c
  LINK_LIBS
    pspdebug
    psppower
    pspdisplay
    pspvshctrl
    pspsysc_user
    pspsystemctrl_user
)

copy_prx_flash0(satelite Satelite USER)