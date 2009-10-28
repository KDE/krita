set(kritagridpaintop_PART_SRCS
    grid_paintop_plugin.cpp
    kis_grid_paintop.cpp
    kis_grid_paintop_factory.cpp
    kis_gridop_option.cpp
    kis_grid_shape_option.cpp
    kis_grid_paintop_settings.cpp
    kis_grid_paintop_settings_widget.cpp
    random_gauss.cpp
    )

kde4_add_ui_files(kritagridpaintop_PART_SRCS wdggridoptions.ui wdgshapeoptions.ui )

kde4_add_plugin(kritagridpaintop ${kritagridpaintop_PART_SRCS})


target_link_libraries(kritagridpaintop kritalibpaintop)

install(TARGETS kritagridpaintop  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritagridpaintop.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
# install( FILES
#    krita-grid.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)


