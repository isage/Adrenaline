add_executable(rebootex)

target_sources(rebootex PRIVATE main.c)
target_link_libraries(rebootex PRIVATE bootloadex)
target_compile_options(rebootex PRIVATE -std=c99 -Os -G0 -Wall -fno-pic)

create_raw_binary(rebootex)

add_custom_command(
  TARGET rebootex
  POST_BUILD
  COMMAND python3 $<TARGET_PROPERTY:tool_gz,EXEC> rebootex.bin rebootex.bin.gz
)