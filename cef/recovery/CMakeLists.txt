add_cef_prx(recovery USER
  SRC_FILES
    crt0.c
    main.c
    libc.c
    menu.c
    installer.c
    plugins.c
    utils.c
    vgraph.c
    imports.S
  LINK_LIBS
    -nostdlib
    gcc
    z
    pspuser
    pspreg
    pspsystemctrl_user
    pspkubridge
    adrenalinectrl
    pspsysc_user
    psppsar
    pspmodinfo
  COMPILE_OPS
    -Wno-unused-variable
    -Wno-incompatible-pointer-types
)

copy_prx_flash0(recovery Recovery USER)
