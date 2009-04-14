
INCLUDE(UsePkgConfig)
PKGCONFIG(QtShiva _QtShivaIncDir _QtShivaLinkDir _QtShivaLinkFlags _QtShivaCflags)

set(QTSHIVA_DEFINITIONS ${_QtShivaCflags})
set(QTSHIVA_LIBRARIES ${_QtShivaLinkFlags})
set(QTSHIVA_INCLUDE_DIR ${_QtShivaIncDir})

if(QTSHIVA_DEFINITIONS AND QTSHIVA_LIBRARIES)

  FIND_PROGRAM(PKGCONFIG_EXECUTABLE NAMES pkg-config PATHS /usr/bin/ /usr/local/bin )

  # query pkg-config asking for QtShiva >= 0.9.0
  EXEC_PROGRAM(${PKGCONFIG_EXECUTABLE} ARGS --atleast-version=0.9.0 QtShiva RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _pkgconfigDevNull )

  if(_return_VALUE STREQUAL "0")
    set(QTSHIVA_FOUND TRUE)
    set(HAVE_QTSHIVA TRUE)
  else(_return_VALUE STREQUAL "0")
    message(STATUS "QtShiva >= 0.9.0 was found")
  endif(_return_VALUE STREQUAL "0")
endif(QTSHIVA_DEFINITIONS AND QTSHIVA_LIBRARIES)

if (QTSHIVA_FOUND)
    if (NOT QTSHIVA_FIND_QUIETLY)
        message(STATUS "Found QtShiva: ${QTSHIVA_LIBRARIES}")
    endif (NOT QTSHIVA_FIND_QUIETLY)
else (QTSHIVA_FOUND)
    if (NOT QtShiva_FIND_QUIETLY)
        message(STATUS "QtShiva was NOT found.")
    endif (NOT QtShiva_FIND_QUIETLY)
    if (QtShiva_FIND_REQUIRED)
        message(FATAL_ERROR "Could NOT find QtShiva")
    endif (QtShiva_FIND_REQUIRED)
endif (QTSHIVA_FOUND)
