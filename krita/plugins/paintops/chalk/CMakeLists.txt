set(kritachalkpaintop_SOURCES
    chalk_paintop_plugin.cpp
    kis_chalk_paintop.cpp
    kis_chalkop_option.cpp
    kis_chalk_paintop_settings.cpp
    kis_chalk_paintop_settings_widget.cpp
    chalk_brush.cpp	
    )

ki18n_wrap_ui(kritachalkpaintop_SOURCES wdgchalkoptions.ui )

add_library(kritachalkpaintop MODULE ${kritachalkpaintop_SOURCES})

target_link_libraries(kritachalkpaintop kritalibpaintop)

install(TARGETS kritachalkpaintop  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
install( FILES
    krita-chalk.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)
