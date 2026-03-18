add_cef_prx(systemctrl
  SRC_FILES
    main.c
    conf.c
    executable_patch.c
    game_patch.c
    gameinfo.c
    hide_patch.c
    init_patch.c
    kubridge.c
    libraries_patch.c
    malloc.c
    nodrm.c
    pgd.c
    plugin.c
    setjmp_clone.S
    storage_cache.c
    sysclibforuser.c
    sysmodpatches.c
    systemctrl.c
    utils.c
    ../bits/ttystdio.c
    rebootex.h
  COMPILE_OPS
    -Wno-builtin-declaration-mismatch
  LINK_LIBS
    microlz
    adrenalinectrl
    pspsemaphore
    pspinit_kernel
    pspstdio_kernel
    psputils_kernel
    pspsysmem_kernel
    pspkermit_driver
    pspchkreg_driver
    pspamctrl_driver
    pspsysclib_kernel
    pspdisplay_driver
    psploadcore_kernel
    psploadexec_kernel
    pspthreadman_kernel
    pspmodulemgr_kernel
    pspiofilemgr_kernel
    pspinterruptmanager_kernel
)

target_include_directories(systemctrl PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

add_custom_command(OUTPUT rebootex.h
  DEPENDS rebootex
  COMMAND bin2c "$<TARGET_FILE:rebootex>.bin" rebootex.h rebootex
)

add_custom_command(
    TARGET systemctrl
    POST_BUILD
    COMMAND psp-fixup-imports -m ${CMAKE_CURRENT_SOURCE_DIR}/nid_map.txt systemctrl.prx
)

copy_prx_flash0(systemctrl SystemControl)