if (HAVE_PYQT5 AND HAVE_SIP AND HAVE_PYTHONLIBS)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${PYTHON_INCLUDE_PATH})

    IF(${SIP_VERSION_STR} VERSION_LESS 5)
        include_directories(${SIP_INCLUDE_DIR})
    ENDIF(${SIP_VERSION_STR} VERSION_LESS 5)
    add_subdirectory(sip)
    add_subdirectory(plugin)
    add_subdirectory(kritarunner)

endif ()
