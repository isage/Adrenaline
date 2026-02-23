include("FetchContent")

set(PSPCFWSDK_PATH "" CACHE PATH "Path to local psp-cfw-sdk (for local development)")

if(PSPCFWSDK_PATH)
message(STATUS "Using local pspcfw repo: ${PSPCFWSDK_PATH}")
FetchContent_Declare(
  pspcfwsdk
  SOURCE_DIR ${PSPCFWSDK_PATH}
  EXCLUDE_FROM_ALL
)
else()
FetchContent_Declare(
  pspcfwsdk
  GIT_REPOSITORY https://github.com/pspdev/psp-cfw-sdk.git
  GIT_TAG main
  EXCLUDE_FROM_ALL
)
endif()
FetchContent_MakeAvailable(pspcfwsdk)

set(CFWSDK ${pspcfwsdk_SOURCE_DIR})
