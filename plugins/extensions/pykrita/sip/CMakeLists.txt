include(SIPMacros)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../libkis)

set(SIP_INCLUDES ${PYQT_SIP_DIR_OVERRIDE} ./krita)
set(SIP_CONCAT_PARTS 1)
set(SIP_TAGS ALL WS_X11 ${PYQT5_VERSION_TAG})
set(SIP_EXTRA_OPTIONS -g -x PyKDE_QVector)

set(PYTHON_SITE_PACKAGES_INSTALL_DIR ${DATA_INSTALL_DIR}/krita/pykrita/)
add_sip_python_module(PyKrita.krita ./krita/kritamod.sip kritalibkis kritaui kritaimage kritalibbrush)

install(FILES
    ./__init__.py
    DESTINATION ${PYTHON_SITE_PACKAGES_INSTALL_DIR})

