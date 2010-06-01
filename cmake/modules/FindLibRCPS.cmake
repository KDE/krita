# Find librcps
# Input:
#  LIBRCPS_MIN_VERSION The minimum version to look for
# Result:
#  LIBRCPS_FOUND - librcps with at least min version is found
#  LIBRCPS_INCLUDE_DIR The include directory
#  LIBRCPS_LIBRARIES The libraries needed to use librcps
#  LIBRCPS_VERSION The actual version, may be higher than minimum

message(STATUS "Running FindLibRCPS")

if (WIN32)
  file(TO_CMAKE_PATH "$ENV{PROGRAMFILES}" _program_FILES_DIR)
endif(WIN32)

if(NOT LIBRCPS_MIN_VERSION)
    set(LIBRCPS_MIN_VERSION "0.3")
endif(NOT LIBRCPS_MIN_VERSION)

if(LIBRCPS_INCLUDE_DIR AND LIBRCPS_LIBRARIES)
  # Already in cache, be silent
  set(LIBRCPS_FIND_QUIETLY TRUE)
endif(LIBRCPS_INCLUDE_DIR AND LIBRCPS_LIBRARIES)

find_path(LIBRCPS_INCLUDE_DIR librcps.h)
find_library(LIBRCPS_LIBRARIES librcps.so)

if(LIBRCPS_INCLUDE_DIR AND LIBRCPS_LIBRARIES)
  set(FIND_LIBRCPS_VERSION_SOURCE
    "#include <stdio.h>\n#include <librcps.h>\nint main()\n {\n printf(\"%s\",rcps_version());return 1;\n }\n")
  set(FIND_LIBRCPS_VERSION_SOURCE_FILE ${CMAKE_BINARY_DIR}/CMakeTmp/FindLibRCPS.c)
  file(WRITE "${FIND_LIBRCPS_VERSION_SOURCE_FILE}" "${FIND_LIBRCPS_VERSION_SOURCE}")

  try_run(RUN_RESULT COMPILE_RESULT
    ${CMAKE_BINARY_DIR}
    ${FIND_LIBRCPS_VERSION_SOURCE_FILE}
    CMAKE_FLAGS
        -DINCLUDE_DIRECTORIES:STRING=${LIBRCPS_INCLUDE_DIR}
        -DLINK_LIBRARIES:STRING=${LIBRCPS_LIBRARIES}
    RUN_OUTPUT_VARIABLE LIBRCPS_VERSION)

  if(COMPILE_RESULT AND RUN_RESULT EQUAL 1)
    message(STATUS "Found librcps version ${LIBRCPS_VERSION}")
    macro_ensure_version(${LIBRCPS_MIN_VERSION} ${LIBRCPS_VERSION} LIBRCPS_VERSION_OK)
    if(LIBRCPS_VERSION_OK)
      set(LIBRCPS_FOUND TRUE)
    else(LIBRCPS_VERSION_OK)
      message(STATUS "Note: librcps version ${LIBRCPS_VERSION} is too old. At least version ${LIBRCPS_MIN_VERSION} is needed.")
      set(LIBRCPS_INCLUDE_DIR "")
      set(LIBRCPS_LIBRARIES "")
    endif(LIBRCPS_VERSION_OK)
  else(COMPILE_RESULT AND RUN_RESULT EQUAL 1)
    message(FATAL_ERROR "Unable to compile or run the librcps version detection program.")
  endif(COMPILE_RESULT AND RUN_RESULT EQUAL 1)

endif(LIBRCPS_INCLUDE_DIR AND LIBRCPS_LIBRARIES)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LIBRCPS DEFAULT_MSG LIBRCPS_INCLUDE_DIR LIBRCPS_LIBRARIES)

mark_as_advanced(LIBRCPS_INCLUDE_DIR LIBRCPS_LIBRARIES)
