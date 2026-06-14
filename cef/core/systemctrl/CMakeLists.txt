add_cef_prx(systemctrl
  SRC_FILES
    src/main.c
    src/conf.c
    src/executable_patch.c
    src/game_patch.c
    src/gameinfo.c
    src/hide_patch.c
    src/init_patch.c
    src/kubridge.c
    src/libraries_patch.c
    src/malloc.c
    src/nodrm.c
    src/pgd.c
    src/plugin.c
    src/setjmp_clone.S
    src/storage_cache.c
    src/sysclibforuser.c
    src/sysmodpatches.c
    src/systemctrl.c
    src/utils.c
    ../../bits/ttystdio.c
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