function(add_cef_prx NAME)
  set(options USER)
  set(oneValueArgs "")
  set(multiValueArgs SRC_FILES LINK_LIBS COMPILE_OPS)

  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  add_prx_module(${NAME} exports.exp)

  target_sources(${NAME} PRIVATE ${ARG_SRC_FILES})
  target_compile_options(${NAME} PRIVATE -Os -G0 -Wall -fno-pic -fshort-wchar ${ARG_COMPILE_OPS})

  if(ARG_USER)
    target_link_libraries(${NAME} PRIVATE ${ARG_LINK_LIBS} pspdebug pspdisplay pspge pspctrl)
  else()
    target_compile_definitions(${NAME} PRIVATE __KERNEL__)
    target_link_libraries(${NAME} PRIVATE ${ARG_LINK_LIBS} pspmodinfo pspsdk pspkernel)
    target_link_options(${NAME} PRIVATE -nostdlib)
  endif()

  string(TOUPPER ${NAME} NAME_UPPER)
  set(DEBUG_VAR "${NAME_UPPER}_DEBUG")
  if(DEBUG)
    target_compile_definitions(${NAME} PRIVATE -DDEBUG=${DEBUG})
  elseif(DEFINED ${DEBUG_VAR})
    message(STATUS "Enabling debug build for ${NAME}.prx")
    target_compile_definitions(${NAME} PRIVATE DEBUG=${${DEBUG_VAR}})
  endif()
endfunction()

function(copy_prx_flash0 NAME MODULE_NAME)
  set(options USER)
  set(oneValueArgs "")
  set(multiValueArgs "")
  cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(ARG_USER)
    set(OUTPUT_PATH ${FLASH0_DIR}/vsh/module/${NAME}.prx)
    set(PACK_CMD python3 $<TARGET_PROPERTY:tool_pspgz,EXEC>
        ${OUTPUT_PATH} ${CFWSDK}/build-tools/gz/UserModule.hdr $<TARGET_FILE:${NAME}>.prx ${MODULE_NAME} 0x0000)
  else()
    set(OUTPUT_PATH ${FLASH0_DIR}/kd/${NAME}.prx)
    set(PACK_CMD python3 $<TARGET_PROPERTY:tool_pspgz,EXEC>
        ${OUTPUT_PATH} ${CFWSDK}/build-tools/gz/SystemControl.hdr $<TARGET_FILE:${NAME}>.prx ${MODULE_NAME} 0x3007)
  endif()

  if(USE_PSP_PACKER)
    set(PACK_CMD psp-packer $<TARGET_FILE:${NAME}>.prx -o ${OUTPUT_PATH})
  endif()

  add_custom_command(
    TARGET ${NAME} POST_BUILD
    COMMAND ${PACK_CMD}
  )
endfunction()