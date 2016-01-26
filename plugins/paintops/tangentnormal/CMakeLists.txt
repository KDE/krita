set(kritatangentnormalpaintop_SOURCES
    kis_tangent_normal_paintop_plugin.cpp
    kis_tangent_normal_paintop.cpp
    kis_tangent_normal_paintop_settings_widget.cpp
    kis_tangent_tilt_option.cpp
    kis_normal_preview_widget.cpp

)

ki18n_wrap_ui(kritatangentnormalpaintop_SOURCES wdgtangenttiltoption.ui )

add_library(kritatangentnormalpaintop MODULE ${kritatangentnormalpaintop_SOURCES})

target_link_libraries(kritatangentnormalpaintop kritalibpaintop)

install(TARGETS kritatangentnormalpaintop  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
########### install files ###############

install( FILES  krita-tangetnormal.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)
install( FILES  krita-tangentnormal-preview.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)

