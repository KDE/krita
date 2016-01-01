set(kritagridpaintop_SOURCES
    grid_paintop_plugin.cpp
    kis_grid_paintop.cpp
    kis_gridop_option.cpp
    kis_grid_shape_option.cpp
    kis_grid_paintop_settings.cpp
    kis_grid_paintop_settings_widget.cpp
    random_gauss.cpp
    )

ki18n_wrap_ui(kritagridpaintop_SOURCES wdggridoptions.ui wdggridbrushshapeoptions.ui )

add_library(kritagridpaintop MODULE ${kritagridpaintop_SOURCES})

target_link_libraries(kritagridpaintop kritalibpaintop)

install(TARGETS kritagridpaintop  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
install( FILES
    krita-grid.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)


