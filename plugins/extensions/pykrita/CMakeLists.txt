if (HAVE_PYQT5 AND HAVE_SIP AND HAVE_PYTHONLIBS)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${SIP_INCLUDE_DIR} ${PYTHON_INCLUDE_PATH})

    add_subdirectory(sip)
    add_subdirectory(plugin)
    add_subdirectory(kritarunner)

endif ()
