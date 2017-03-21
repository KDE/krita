set(kritaqmic_SOURCES 
    QMic.cpp 
    PluginSettings.cpp
    #Converter.cpp
)
add_library(kritaqmic MODULE ${kritaqmic_SOURCES})
target_link_libraries(kritaqmic kritaui)
install(TARGETS kritaqmic  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
