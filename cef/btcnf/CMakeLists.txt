set(BTCNF_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/btcnf.py")

set(BTCNFS
  pspbtknf:pspbtcnf_inferno
  pspbtlnf:pspbtcnf_march33
  pspbtjnf:pspbtcnf_normal
  pspbtmnf:pspbtcnf_np9660
  pspbtrnf:pspbtcnf_recovery
)

foreach(BTCNF IN LISTS BTCNFS)
  string(REPLACE ":" ";" kv "${BTCNF}")
  list(GET kv 0 BTCNF_NAME)
  list(GET kv 1 FOLDER)

  set(BTCNF_PATH_NOEXT "${CMAKE_CURRENT_LIST_DIR}/${FOLDER}/pspbtcnf")
  list(APPEND BTCNF_BIN_FILES "${BTCNF_NAME}.bin")

  add_custom_command(
    OUTPUT ${BTCNF_NAME}.bin
    DEPENDS "${BTCNF_PATH_NOEXT}.txt"
    COMMAND python3 ${BTCNF_SCRIPT} build "${BTCNF_PATH_NOEXT}.txt"
    COMMAND ${CMAKE_COMMAND} -E copy "${BTCNF_PATH_NOEXT}.bin" ${BTCNF_NAME}.bin
    COMMAND ${CMAKE_COMMAND} -E copy "${BTCNF_PATH_NOEXT}.bin" "${FLASH0_DIR}/kd/${BTCNF_NAME}.bin"
  )
endforeach()

add_custom_target(build_btcnfs ALL
  DEPENDS ${BTCNF_BIN_FILES}
)