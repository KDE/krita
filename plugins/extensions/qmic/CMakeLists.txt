# GMic bindings exposure.
set(kisimageinterface_SOURCES
    kis_image_interface.cpp
    kis_import_qmic_processing_visitor.cpp
    kis_input_output_mapper.cpp
    kis_qmic_applicator.cpp
    kis_qmic_data.cpp
    kis_qmic_simple_convertor.cpp
    kis_qmic_progress_manager.cpp
    kis_qmic_synchronize_image_size_command.cpp
    kis_qmic_synchronize_layers_command.cpp
)

add_library(kisimageinterface SHARED ${kisimageinterface_SOURCES})
generate_export_header(kisimageinterface)
target_link_libraries(kisimageinterface kritaui)

set(kisimageinterface_HEADERS
    kis_image_interface.h
    KritaGmicPluginInterface.h
    ${CMAKE_CURRENT_BINARY_DIR}/kisimageinterface_export.h
)

set_target_properties(kisimageinterface
    PROPERTIES
    PUBLIC_HEADER "${kisimageinterface_HEADERS}"
)

set_target_properties(kisimageinterface PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)

install(TARGETS kisimageinterface ${INSTALL_TARGETS_DEFAULT_ARGS}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

set(kritaqmic_SOURCES 
    QMic.cpp
    PluginSettings.cpp
)

ki18n_wrap_ui (
    kritaqmic_SOURCES
    WdgQMicSettings.ui
)

add_library(kritaqmic MODULE ${kritaqmic_SOURCES})
target_link_libraries(kritaqmic kritaui kisimageinterface)

install(TARGETS kritaqmic LIBRARY DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

add_subdirectory(tests)
