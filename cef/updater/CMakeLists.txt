add_executable(updater EXCLUDE_FROM_ALL)

target_sources(updater PRIVATE
  main.c
  ${CMAKE_CURRENT_SOURCE_DIR}/psp-updatelist.txt
)

target_compile_options(updater PRIVATE -Os -G0 -Wall -fshort-wchar -fno-pic -mno-check-zero-division)
target_include_directories(updater PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
target_link_directories(updater PRIVATE ${CFWSDK}/libs)
target_link_libraries(updater PRIVATE
  adrenalinectrl
  pspsystemctrl_user
  pspdebug
  pspdisplay
  pspge
  pspctrl
  pspnet
  pspnet_apctl
)

create_pbp_file(
  TARGET updater
  SFO_PATH "${CMAKE_CURRENT_SOURCE_DIR}/UPDATE.SFO"
  ICON_PATH ${CMAKE_CURRENT_SOURCE_DIR}/ICON0.PNG
  BUILD_PRX
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/psp-updatelist.txt
  COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/gen_updatelist.py
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/psp-updatelist.template
)