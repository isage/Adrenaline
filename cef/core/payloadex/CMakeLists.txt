add_executable(payloadex)

target_sources(payloadex PRIVATE main.c)
target_link_libraries(payloadex PRIVATE bootloadex)
target_compile_options(payloadex PRIVATE -std=c99 -Os -G0 -Wall -fno-pic)

create_raw_binary(payloadex)

add_custom_command(
  TARGET payloadex POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy payloadex.bin ${FLASH0_DIR}
)