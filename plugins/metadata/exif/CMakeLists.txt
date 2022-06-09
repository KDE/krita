set(kritaexif_SOURCES
    kis_exif_io.cpp
    kis_exif_plugin.cpp
)

kis_add_library(kritaexif MODULE ${kritaexif_SOURCES} PCH_FILE kis_global_pch.h)

generate_export_header(kritaexif)

target_link_libraries(kritaexif
    PRIVATE
        kritametadata
        KF5::CoreAddons
        LibExiv2::LibExiv2
)

install(TARGETS kritaexif DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
