add_executable(payloadex_ark)

target_sources(payloadex_ark PRIVATE main.c)
target_link_libraries(payloadex_ark PRIVATE bootloadex)
target_compile_options(payloadex_ark PRIVATE -std=c99 -Os -G0 -Wall -fno-pic)

create_raw_binary(payloadex_ark)

add_custom_command(
  TARGET payloadex_ark POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy payloadex_ark.bin ${FLASH0_DIR}
)