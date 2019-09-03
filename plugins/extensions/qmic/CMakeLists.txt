set(kritaqmic_SOURCES 
    QMic.cpp
    kis_qmic_data.cpp
    kis_qmic_applicator.cpp
    kis_qmic_progress_manager.cpp
    kis_qmic_synchronize_image_size_command.cpp
    kis_qmic_synchronize_layers_command.cpp
    kis_import_qmic_processing_visitor.cpp
    kis_input_output_mapper.cpp
    kis_qmic_simple_convertor.cpp
    PluginSettings.cpp
)

ki18n_wrap_ui (
    kritaqmic_SOURCES
    WdgQMicSettings.ui
)

kis_add_library(kritaqmic MODULE ${kritaqmic_SOURCES})
target_link_libraries(kritaqmic kritaui)
install(TARGETS kritaqmic  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

add_subdirectory(tests)
