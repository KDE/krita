if (HAVE_PYQT5 AND HAVE_SIP AND HAVE_PYTHONLIBS)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${SIP_INCLUDE_DIR} ${PYTHON_INCLUDE_PATH})

    add_subdirectory(sip)
    add_subdirectory(plugin)
    add_subdirectory(kritarunner)

    add_test(pythonUnitTests ${PYTHON_EXECUTABLE} -m unittest discover "${CMAKE_INSTALL_PREFIX}" "${CMAKE_SOURCE_DIR}" -s ${CMAKE_SOURCE_DIR}/plugins/extensions/pykrita/tests -p "*_test.py")

endif ()
