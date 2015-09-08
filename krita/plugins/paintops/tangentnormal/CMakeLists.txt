set(kritatangentnormalpaintop_PART_SRCS
    kis_tangent_normal_paintop_plugin.cpp
    kis_tangent_normal_paintop.cpp
    kis_tangent_normal_paintop_settings_widget.cpp
    kis_tangent_tilt_option.cpp
    kis_normal_preview_widget.cpp

)

ki18n_wrap_ui(kritatangentnormalpaintop_PART_SRCS wdgtangenttiltoption.ui )

add_library(kritatangentnormalpaintop MODULE ${kritatangentnormalpaintop_PART_SRCS})
kcoreaddons_desktop_to_json(kritatangentnormalpaintop kritatangentnormalpaintop.desktop)

target_link_libraries(kritatangentnormalpaintop kritalibpaintop)

install(TARGETS kritatangentnormalpaintop  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
########### install files ###############

install( FILES  kritatangentnormalpaintop.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
install( FILES  krita-tangetnormal.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)
install( FILES  krita-tangentnormal-preview.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)

