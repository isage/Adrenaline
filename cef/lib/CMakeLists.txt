file(GLOB PREBUILT_LIBS CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.a")

foreach(FILE IN LISTS PREBUILT_LIBS)
  get_filename_component(LIB_FILENAME "${FILE}" NAME_WE)
  string(REGEX REPLACE "^lib" "" LIB "${LIB_FILENAME}")
  
  add_library(${LIB} STATIC IMPORTED GLOBAL)
  
  target_include_directories(${LIB} INTERFACE ${PROJECT_SOURCE_DIR}/include)
  
  set_target_properties(${LIB} PROPERTIES
    IMPORTED_LOCATION ${FILE}
  )   
endforeach()
