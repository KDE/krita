set(kritaxmp_SOURCES
    kis_xmp_io.cpp
    kis_xmp_plugin.cpp
)

add_library(kritaxmp MODULE ${kritaxmp_SOURCES})

generate_export_header(kritaxmp)

target_link_libraries(kritaxmp
    PRIVATE
        kritametadata
        KF5::CoreAddons
        LibExiv2::LibExiv2
)

install(TARGETS kritaxmp DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
