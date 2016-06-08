
set(Python_ADDITIONAL_VERSIONS 3.0 3.1 3.2 3.3 3.4 3.5 3.6)
macro_optional_find_package(PythonLibrary)
macro_log_feature(PYTHON_LIBRARY "PythonLibrary" "Python Library" FALSE "" "Required by the Krita PyQt plugin")
macro_bool_to_01(PYTHONLIBS_FOUND HAVE_PYTHONLIBS)

macro_optional_find_package(SIP 4.7.1)
macro_log_feature(SIP_FOUND "SIP" "Support for SIP Python bindings" FALSE "" "Required by the Krita PyQt plugin")
macro_bool_to_01(SIP_FOUND HAVE_SIP)

macro_optional_find_package(PyQt4 4.3.1)
macro_log_feature(PYQT4_FOUND "PyQt4" "Python bindings for Qt4" FALSE "" "Required by the Krita PyQt plugin")
macro_bool_to_01(PYQT4_FOUND HAVE_PYQT4)

if (HAVE_PYQT4 AND HAVE_SIP AND HAVE_PYTHONLIBS)

# if (NOT EXISTS ${PYQT4_SIP_DIR}/QtCore/QtCoremod.sip)
#     message(WARNING "krita-pykrita plugin needs the PyQt4 development sip file QtCoremod.sip")
#     return()
# endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${SIP_INCLUDE_DIR} ${PYTHON_INCLUDE_PATH})
    
add_subdirectory(libkis)
add_subdirectory(sip)
add_subdirectory(src)


endif (HAVE_PYQT4 AND HAVE_SIP AND HAVE_PYTHONLIBS)
