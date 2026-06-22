add_cef_prx(recovery USER
  SRC_FILES
    src/crt0.c
    src/main.c
    src/libc.c
    src/menu.c
    src/installer.c
    src/plugins.c
    src/utils.c
    src/vgraph.c
    imports.S
  LINK_LIBS
    -nostdlib
    gcc
    z
	pspsdk
    pspge
    pspreg
    pspuser
    psppsar
    pspctrl
    pspmodinfo
    pspdisplay
    pspkubridge
    pspsysc_user
    adrenalinectrl
    pspsystemctrl_user
  COMPILE_OPS
    -Wno-unused-variable
    -Wno-incompatible-pointer-types
)

copy_prx_flash0(recovery Recovery USER)
