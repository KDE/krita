include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${CMAKE_SOURCE_DIR}/krita/image/metadata )
include_directories( ${PNG_INCLUDE_DIR} )
add_definitions(${PNG_DEFINITIONS} ${KDE4_ENABLE_EXCEPTIONS})

set(kranimseq_PART_SRCS
    kranim_sequence.cpp
    sequence_generator.cpp
)

kde4_add_ui_files(kranimseq_PART_SRCS kis_wdg_options_kranimseq.ui )
kde4_add_plugin(kranimseq ${kranimseq_PART_SRCS})

target_link_libraries(kranimseq kritaui ${PNG_LIBRARIES})
install(TARGETS kranimseq DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kranimseq_export.desktop DESTINATION ${SERVICES_INSTALL_DIR})
