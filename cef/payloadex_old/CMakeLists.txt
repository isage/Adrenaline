add_executable(payloadex)

target_sources(payloadex PRIVATE main.c libc.c)
target_compile_options(payloadex PRIVATE -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division -fno-builtin)
target_link_options(payloadex PRIVATE -nostdlib -T${CMAKE_CURRENT_SOURCE_DIR}/linkfile.x)

create_raw_binary(payloadex)

add_custom_command(
  TARGET payloadex POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy payloadex.bin ${FLASH0_DIR}
)