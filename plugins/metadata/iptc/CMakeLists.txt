set(kritaiptc_SOURCES
    kis_iptc_io.cpp
    kis_iptc_plugin.cpp
)

add_library(kritaiptc MODULE ${kritaiptc_SOURCES})

generate_export_header(kritaiptc)

target_link_libraries(kritaiptc
    PRIVATE
        kritametadata
        KF5::CoreAddons
        LibExiv2::LibExiv2
)

install(TARGETS kritaiptc DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
