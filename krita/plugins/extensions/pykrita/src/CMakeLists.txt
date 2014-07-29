# NOTE Disable trivial Qt keywords due conflicts w/ some Python.h header
# (at least version 3.3 of it has a member PyType_Spec::slots)
#add_definitions(-DQT_NO_KEYWORDS)
configure_file(config.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config.h)

set(SOURCES 
    plugin.cpp 
    pyqtpluginsettings.cpp
    utilities.cpp
    engine.cpp
)

kde4_add_ui_files(SOURCES 
    info.ui 
    manager.ui
)

kde4_add_plugin(kritapykrita ${SOURCES})

target_link_libraries(
    kritapykrita
    ${PYTHON_LIBRARY}
    kritaui
    kritalibbrush
  )

install(TARGETS kritapykrita DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kritapyqt.desktop DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
install(FILES kritapyqtplugin.rc DESTINATION ${DATA_INSTALL_DIR}/kritaplugins)

install(FILES kritapyqtplugin.desktop  DESTINATION  ${SERVICETYPES_INSTALL_DIR})

# Install "built-in" plugins
install(
    DIRECTORY krita
    DESTINATION ${DATA_INSTALL_DIR}/krita/plugins/pykrita
    FILES_MATCHING PATTERN "*.py"
)

add_subdirectory(plugins)
add_subdirectory(test)

