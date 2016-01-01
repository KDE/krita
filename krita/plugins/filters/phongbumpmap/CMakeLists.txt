set(kritaphongbumpmap_SOURCES
    kis_phong_bumpmap_plugin.cpp
    kis_phong_bumpmap_config_widget.cpp
    kis_phong_bumpmap_filter.cpp
    phong_pixel_processor.cpp
    )

ki18n_wrap_ui(kritaphongbumpmap_SOURCES wdgphongbumpmap.ui)
add_library(kritaphongbumpmap MODULE ${kritaphongbumpmap_SOURCES})
target_link_libraries(kritaphongbumpmap kritaui)
install(TARGETS kritaphongbumpmap  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
