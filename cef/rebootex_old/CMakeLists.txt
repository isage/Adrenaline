add_executable(rebootex_old ALIAS reboot)

target_sources(rebootex_old PRIVATE main.c libc.c)
target_compile_options(rebootex_old PRIVATE -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division -fno-builtin)
target_link_libraries(rebootex_old PRIVATE bootloadex)
target_link_options(rebootex_old PRIVATE -nostartfiles -nostdlib -T${CMAKE_CURRENT_SOURCE_DIR}/linkfile.x)

create_raw_binary(rebootex_old)
