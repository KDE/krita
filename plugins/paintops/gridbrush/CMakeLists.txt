set(kritagridpaintop_SOURCES
    grid_paintop_plugin.cpp
    kis_grid_paintop.cpp
    KisGridOpOptionData.cpp
    KisGridOpOptionModel.cpp
    KisGridOpOptionWidget.cpp
    
    kis_grid_paintop_settings.cpp
    kis_grid_paintop_settings_widget.cpp
    KisGridShapeOptionData.cpp
    KisGridShapeOptionModel.cpp
    KisGridShapeOptionWidget.cpp
    )

ki18n_wrap_ui(kritagridpaintop_SOURCES wdggridoptions.ui wdggridbrushshapeoptions.ui )

kis_add_library(kritagridpaintop MODULE ${kritagridpaintop_SOURCES})

target_link_libraries(kritagridpaintop kritalibpaintop)

install(TARGETS kritagridpaintop  DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
install( FILES
    krita-grid.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)


