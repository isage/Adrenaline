add_executable(binary)

target_sources(binary PRIVATE start.S main.c)
target_compile_options(binary PRIVATE -Os -G0 -Wall -fshort-wchar -fno-pic)
target_include_directories(binary PRIVATE ${CFWSDK}/include)
target_link_options(binary PRIVATE -nostdlib -T${CMAKE_CURRENT_SOURCE_DIR}/linkfile.x)

create_raw_binary(binary)
