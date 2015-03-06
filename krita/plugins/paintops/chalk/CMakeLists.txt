set(kritachalkpaintop_PART_SRCS
    chalk_paintop_plugin.cpp
    kis_chalk_paintop.cpp
    kis_chalkop_option.cpp
    kis_chalk_paintop_settings.cpp
    kis_chalk_paintop_settings_widget.cpp
    chalk_brush.cpp	
    )

ki18n_wrap_ui(kritachalkpaintop_PART_SRCS wdgchalkoptions.ui )

add_library(kritachalkpaintop MODULE ${kritachalkpaintop_PART_SRCS})


target_link_libraries(kritachalkpaintop kritalibpaintop)

install(TARGETS kritachalkpaintop  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kritachalkpaintop.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
install( FILES
    krita-chalk.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)
