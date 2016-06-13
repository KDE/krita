# NOTE Disable trivial Qt keywords due conflicts w/ some Python.h header
# (at least version 3.3 of it has a member PyType_Spec::slots)
add_definitions(-DQT_NO_KEYWORDS)
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(SOURCES 
    plugin.cpp 
    pyqtpluginsettings.cpp
    utilities.cpp
    engine.cpp
)

ki18n_wrap_ui(SOURCES 
    info.ui 
    manager.ui
)

add_library(kritapykrita MODULE ${SOURCES})

target_link_libraries(
    kritapykrita
    ${PYTHON_LIBRARY}
    kritaui
    kritalibkis
  )

install(TARGETS kritapykrita DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install(FILES kritapyqtplugin.xmlgui DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)

install(FILES kritapyqtplugin.desktop  DESTINATION  ${SERVICETYPES_INSTALL_DIR})

# Install "built-in" plugins
install(
    DIRECTORY krita
    DESTINATION ${DATA_INSTALL_DIR}/krita/plugins/pykrita
    FILES_MATCHING PATTERN "*.py"
)

add_subdirectory(plugins)
#add_subdirectory(test)

