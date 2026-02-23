add_executable(payloadex_old)

target_sources(payloadex_old PRIVATE main.c libc.c)
target_compile_options(payloadex_old PRIVATE -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division -fno-builtin)
target_link_options(payloadex_old PRIVATE -nostdlib -T${CMAKE_CURRENT_SOURCE_DIR}/linkfile.x)

create_raw_binary(payloadex_old)

add_custom_command(
  TARGET payloadex_old POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy payloadex_old.bin "${FLASH0_DIR}/payloadex.bin"
)