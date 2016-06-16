add_subdirectory ( tests )

set(kritaquickpaintop_PART_SRCS
    quick_paintop_plugin.cpp
    kis_quickop_settings_widget.cpp
    kis_quickop.cpp
    kis_multipoint_painter.cpp
)

add_library(kritaquickpaintop MODULE ${kritaquickpaintop_PART_SRCS})

target_link_libraries(kritaquickpaintop kritalibpaintop)

generate_export_header(kritaquickpaintop BASE_NAME kritaquickpaintop)

install(TARGETS kritaquickpaintop DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install( FILES  krita-quickop.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)

