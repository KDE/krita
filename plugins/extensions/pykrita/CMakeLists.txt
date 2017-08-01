# PythonLibrary should've been found in the root CMakeLists.txt
set_package_properties(PythonLibrary PROPERTIES
    DESCRIPTION "Python Library"
    URL "http://www.python.org"
    TYPE OPTIONAL
    PURPOSE "Required by the Krita PyQt plugin")
macro_bool_to_01(PYTHONLIBS_FOUND HAVE_PYTHONLIBS)


find_package(SIP "4.18.0")
set_package_properties(SIP PROPERTIES
    DESCRIPTION "Support for generating SIP Python bindings"
    URL "https://www.riverbankcomputing.com/software/sip/download"
    TYPE OPTIONAL
    PURPOSE "Required by the Krita PyQt plugin")
macro_bool_to_01(SIP_FOUND HAVE_SIP)

find_package(PyQt5 "5.6.0")
set_package_properties(PyQt5 PROPERTIES
    DESCRIPTION "Python bindings for Qt5."
    URL "https://www.riverbankcomputing.com/software/pyqt/download5"
    TYPE OPTIONAL
    PURPOSE "Required by the Krita PyQt plugin")
macro_bool_to_01(PYQT5_FOUND HAVE_PYQT5)

if (HAVE_PYQT5 AND HAVE_SIP AND HAVE_PYTHONLIBS)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${SIP_INCLUDE_DIR} ${PYTHON_INCLUDE_PATH})

add_subdirectory(sip)
add_subdirectory(plugin)
add_subdirectory(kritarunner)

add_test(pythonUnitTests ${PYTHON_EXECUTABLE} -m unittest discover "${CMAKE_INSTALL_PREFIX}" "${CMAKE_SOURCE_DIR}" -s ${CMAKE_SOURCE_DIR}/plugins/extensions/pykrita/tests -p "*_test.py")

endif ()
