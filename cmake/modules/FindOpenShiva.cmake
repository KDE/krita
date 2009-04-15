
INCLUDE(UsePkgConfig)
PKGCONFIG(OpenShiva _OpenShivaIncDir _OpenShivaLinkDir _OpenShivaLinkFlags _OpenShivaCflags)

set(OPENSHIVA_DEFINITIONS ${_OpenShivaCflags})
set(OPENSHIVA_LIBRARIES ${_OpenShivaLinkFlags})
set(OPENSHIVA_INCLUDE_DIR ${_OpenShivaIncDir})

if(OPENSHIVA_DEFINITIONS AND OPENSHIVA_LIBRARIES)

  FIND_PROGRAM(PKGCONFIG_EXECUTABLE NAMES pkg-config PATHS /usr/bin/ /usr/local/bin )

  # query pkg-config asking for OpenShiva >= 0.9.8
  EXEC_PROGRAM(${PKGCONFIG_EXECUTABLE} ARGS --atleast-version=0.9.8 OpenShiva RETURN_VALUE _return_VALUE OUTPUT_VARIABLE _pkgconfigDevNull )

  if(_return_VALUE STREQUAL "0")
    set(OPENSHIVA_FOUND TRUE)
    set(HAVE_OPENSHIVA TRUE)
  else(_return_VALUE STREQUAL "0")
    message(STATUS "OpenShiva >= 0.9.9 8 not found")
  endif(_return_VALUE STREQUAL "0")
endif(OPENSHIVA_DEFINITIONS AND OPENSHIVA_LIBRARIES)

if (OPENSHIVA_FOUND)
    if (NOT OPENSHIVA_FIND_QUIETLY)
        message(STATUS "Found OpenShiva: ${OPENSHIVA_LIBRARIES}")
    endif (NOT OPENSHIVA_FIND_QUIETLY)
else (OPENShiva_FOUND)
    if (NOT OPENSHIVA_FIND_QUIETLY)
        message(STATUS "OpenShiva was NOT found.")
    endif (NOT OPENSHIVA_FIND_QUIETLY)
    if (OPENSHIVA_FIND_REQUIRED)
        message(FATAL_ERROR "Could NOT find OpenShiva")
    endif (OPENSHIVA_FIND_REQUIRED)
endif (OPENSHIVA_FOUND)
