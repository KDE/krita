set(kritaqmic_SOURCES 
    QMic.cpp 
    PluginSettings.cpp
    kis_qmic_simple_convertor.cpp
    kis_input_output_mapper.cpp
)

ki18n_wrap_ui (
    kritaqmic_SOURCES
    WdgQMicSettings.ui
)

add_library(kritaqmic MODULE ${kritaqmic_SOURCES})
target_link_libraries(kritaqmic kritaui)
install(TARGETS kritaqmic  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
