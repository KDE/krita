
INCLUDE(FindPkgConfig)

pkg_check_modules(OPENSHIVA OpenShiva>=0.9.16)

if (OPENSHIVA_FOUND)
    set(HAVE_OPENSHIVA TRUE)
    message(STATUS "OpenShiva >= 0.9.16 was found")
    if (NOT OPENSHIVA_FIND_QUIETLY)
        message(STATUS "Found OpenShiva: ${OPENSHIVA_LIBRARIES}")
    endif ()
else ()
    if (NOT OPENSHIVA_FIND_QUIETLY)
        message(STATUS "OpenShiva was NOT found.")
    endif ()
    if (OPENSHIVA_FIND_REQUIRED)
        message(FATAL_ERROR "Could NOT find OpenShiva")
    endif ()
endif ()
