macro_optional_find_package(PythonLibrary)
macro_log_feature(PYTHON_LIBRARY "PythonLibrary" "Python Library" FALSE "" "Required by the Krita PyQt plugin")
macro_bool_to_01(PYTHONLIBS_FOUND HAVE_PYTHONLIBS)

macro_optional_find_package(SIP 4.18.0)
macro_log_feature(SIP_FOUND "SIP" "Support for SIP Python bindings" FALSE "" "Required by the Krita PyQt plugin")
macro_bool_to_01(SIP_FOUND HAVE_SIP)

macro_optional_find_package(PyQt5 5.6.0)
macro_log_feature(PYQT5_FOUND "PyQt5" "Python bindings for Qt5" FALSE "" "Required by the Krita PyQt plugin")
macro_bool_to_01(PYQT5_FOUND HAVE_PYQT5)

if (HAVE_PYQT5 AND HAVE_SIP AND HAVE_PYTHONLIBS)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${SIP_INCLUDE_DIR} ${PYTHON_INCLUDE_PATH})
    
add_subdirectory(libkis)
add_subdirectory(sip)
add_subdirectory(src)


endif ()
