     

INCLUDE(UsePkgConfig)
PKGCONFIG(OpenCTL _OpenCTLIncDir _OpenCTLLinkDir _OpenCTLLinkFlags _OpenCTLCflags)

set(OPENCTL_DEFINITIONS ${_OpenCTLCflags})
set(OPENCTL_LIBRARIES ${_OpenCTLLinkFlags})
set(OPENCTL_INCLUDE_DIR ${_OpenCTLIncDir})

if(OPENCTL_DEFINITIONS AND OPENCTL_LIBRARIES)

  FIND_PROGRAM(PKGCONFIG_EXECUTABLE NAMES pkg-config PATHS /usr/bin/ /usr/local/bin )

  # query pkg-config asking for OpenCTL >= 0.9.12
  EXEC_PROGRAM(${PKGCONFIG_EXECUTABLE} ARGS --atleast-version=0.9.12 OpenCTL RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _pkgconfigDevNull )

  if(_return_VALUE STREQUAL "0")
    set(OPENCTL_FOUND TRUE)
    set(HAVE_OPENCTL TRUE)
  else(_return_VALUE STREQUAL "0")
    message(STATUS "OpenCTL < 0.9.12 was found")
  endif(_return_VALUE STREQUAL "0")

endif(OPENCTL_DEFINITIONS AND OPENCTL_LIBRARIES)

if (OPENCTL_FOUND)
    if (NOT OpenCTL_FIND_QUIETLY)
        message(STATUS "Found OPENCTL: ${OPENCTL_LIBRARIES}")
    endif (NOT OpenCTL_FIND_QUIETLY)
else (OPENCTL_FOUND)
    if (NOT OpenCTL_FIND_QUIETLY)
        message(STATUS "OpenCTL was NOT found.")
    endif (NOT OpenCTL_FIND_QUIETLY)
    if (OpenCTL_FIND_REQUIRED)
        message(FATAL_ERROR "Could NOT find OPENCTL")
    endif (OpenCTL_FIND_REQUIRED)
endif (OPENCTL_FOUND)
