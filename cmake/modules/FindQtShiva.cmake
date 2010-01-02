
INCLUDE(FindPkgConfig)

pkg_check_modules(QTSHIVA QtShiva>=0.9.0)

if (QTSHIVA_FOUND)
    message(STATUS "QtShiva >= 0.9.0 was found")
    set(HAVE_QTSHIVA TRUE)
    if (NOT QTSHIVA_FIND_QUIETLY)
	message(STATUS "Found QtShiva: ${QTSHIVA_LIBRARIES}")
    endif ()
else ()
    if (NOT QtShiva_FIND_QUIETLY)
        message(STATUS "QtShiva was NOT found.")
    endif ()
    if (QtShiva_FIND_REQUIRED)
        message(FATAL_ERROR "Could NOT find QtShiva")
    endif ()
endif ()
