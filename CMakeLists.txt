cmake_minimum_required(VERSION 3.2)

if(NOT DEFINED ENV{VITASDK})
    message(FATAL_ERROR "Please define VITASDK to point to your vita SDK path!")
endif()

set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")

project(adrenaline)
include("${VITASDK}/share/vita.cmake" REQUIRED)

add_subdirectory(kernel)
add_subdirectory(user)
add_subdirectory(vsh)
add_subdirectory(bubble)


link_directories(${CMAKE_CURRENT_BINARY_DIR}/kernel)
link_directories(${CMAKE_BINARY_DIR}/kernel)

add_dependencies(${PROJECT_NAME} "kernel_all")
add_dependencies(${PROJECT_NAME} "user_all")
add_dependencies(${PROJECT_NAME} "vsh_all")
add_dependencies(adrenaline_user "kernel_all")

