add_executable(rebootex)

target_sources(rebootex PRIVATE main.c libc.c)
target_compile_options(rebootex PRIVATE -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division -fno-builtin)
target_link_libraries(rebootex PRIVATE bootloadex)
target_link_options(rebootex PRIVATE -nostartfiles -nostdlib -T${CMAKE_CURRENT_SOURCE_DIR}/linkfile.x)

create_raw_binary(rebootex)
