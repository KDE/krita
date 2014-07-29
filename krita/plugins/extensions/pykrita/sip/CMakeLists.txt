include(SIPMacros)

set(SIP_INCLUDES ${PYQT4_SIP_DIR} ./krita)
set(SIP_CONCAT_PARTS 1)
set(SIP_TAGS ALL WS_X11 ${PYQT4_VERSION_TAG})

if(PYQT4_VERSION STRLESS "040905")
    set(SIP_EXTRA_OPTIONS -g)
else (PYQT4_VERSION STRLESS "040905")
    # Disable QVector<int> for newer PyQt
    set(SIP_EXTRA_OPTIONS -g -x PyKDE_QVector)
endif(PYQT4_VERSION STRLESS "040905")

add_sip_python_module(PyKrita4.krita ./krita/kritamod.sip kritalibkis kritaui kritaimage kritalibbrush)

install(FILES
    ./__init__.py
    DESTINATION ${PYTHON_SITE_PACKAGES_INSTALL_DIR}/PyKrita4/)

