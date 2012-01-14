     
INCLUDE(FindPkgConfig)

pkg_check_modules(OPENCTL OpenCTL>=0.9.16)

if (OPENCTL_FOUND)
    set(HAVE_OPENCTL TRUE)
    message(STATUS "OpenCTL Found Version: " ${OPENCTL_VERSION})

    if (NOT OpenCTL_FIND_QUIETLY )
        message(STATUS "Found OPENCTL: ${OPENCTL_LIBRARIES}")
    endif (NOT OpenCTL_FIND_QUIETLY)
else ()
    if (NOT OpenCTL_FIND_QUIETLY)
        message(STATUS "OpenCTL was NOT found.")
    endif ()
    if (OpenCTL_FIND_REQUIRED)
        message(FATAL_ERROR "Could NOT find OPENCTL")
    endif ()
endif ()
