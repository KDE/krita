set(kritagridpaintop_PART_SRCS
    grid_paintop_plugin.cpp
    kis_grid_paintop.cpp
    kis_gridop_option.cpp
    kis_grid_shape_option.cpp
    kis_grid_paintop_settings.cpp
    kis_grid_paintop_settings_widget.cpp
    random_gauss.cpp
    )

ki18n_wrap_ui(kritagridpaintop_PART_SRCS wdggridoptions.ui wdggridbrushshapeoptions.ui )

add_library(kritagridpaintop MODULE ${kritagridpaintop_PART_SRCS})


target_link_libraries(kritagridpaintop kritalibpaintop)

install(TARGETS kritagridpaintop  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritagridpaintop.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
 install( FILES
    krita-grid.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)


