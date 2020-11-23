# GMic bindings exposure.
set(kisqmicinterface_SOURCES
    kis_import_qmic_processing_visitor.cpp
    kis_input_output_mapper.cpp
    kis_qmic_applicator.cpp
    kis_qmic_data.cpp
    kis_qmic_interface.cpp
    kis_qmic_progress_manager.cpp
    kis_qmic_simple_convertor.cpp
    kis_qmic_synchronize_image_size_command.cpp
    kis_qmic_synchronize_layers_command.cpp
)

add_library(kisqmicinterface SHARED ${kisqmicinterface_SOURCES})
generate_export_header(kisqmicinterface)
target_link_libraries(kisqmicinterface kritaui)

set(kisqmicinterface_HEADERS
    kis_qmic_interface.h
    kis_qmic_plugin_interface.h
    ${CMAKE_CURRENT_BINARY_DIR}/kisqmicinterface_export.h
)

set_target_properties(kisqmicinterface
    PROPERTIES
    PUBLIC_HEADER "${kisqmicinterface_HEADERS}"
)

set_target_properties(kisqmicinterface PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)

install(TARGETS kisqmicinterface ${INSTALL_TARGETS_DEFAULT_ARGS}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

set(kritaqmic_SOURCES 
    QMic.cpp
)

ki18n_wrap_ui (
    kritaqmic_SOURCES
)

add_library(kritaqmic MODULE ${kritaqmic_SOURCES})
target_link_libraries(kritaqmic kisqmicinterface)

install(TARGETS kritaqmic LIBRARY DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

add_subdirectory(tests)
