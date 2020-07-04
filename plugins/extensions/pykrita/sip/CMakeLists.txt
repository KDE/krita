include(SIPMacros)

message( ${SIP_VERSION_STR} " - The version of SIP found as a human readable string.")
message( ${SIP_EXECUTABLE} " - Path and filename of the SIP command line executable.")

set(SIP_INCLUDES
    ${PYQT5_SIP_DIR}
    ${PYQT_SIP_DIR_OVERRIDE}
     ./krita)

set(SIP_CONCAT_PARTS 1)
set(SIP_TAGS ALL WS_X11 ${PYQT5_VERSION_TAG})
set(SIP_EXTRA_OPTIONS -g -o -x PyKDE_QVector ${PYQT5_SIP_NAME})

set(PYTHON_SITE_PACKAGES_INSTALL_DIR ${LIB_INSTALL_DIR}/krita-python-libs)
file(GLOB PYKRITA_KRITA_sip_files ./krita/*.sip)
set(SIP_EXTRA_FILES_DEPEND ${PYKRITA_KRITA_sip_files})
add_sip_python_module(PyKrita.krita ./krita/kritamod.sip kritalibkis kritaui kritaimage kritalibbrush)

if (ENABLE_PYTHON_2)
    # Add an init file to turn it into a valid py2 module.
    # Otherwise PyKrita cannot be loaded.
    install(FILES
        ./__init__.py
        DESTINATION ${PYTHON_SITE_PACKAGES_INSTALL_DIR}/PyKrita)
endif (ENABLE_PYTHON_2)
