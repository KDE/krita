set(kritatangentnormalpaintop_PART_SRCS
    kis_tangent_normal_paintop_plugin.cpp
    kis_tangent_normal_paintop.cpp
    kis_tangent_normal_paintop_settings_widget.cpp
    kis_tangent_tilt_option.cpp
    kis_normal_preview_widget.cpp

)

kde4_add_ui_files(kritatangentnormalpaintop_PART_SRCS wdgtangenttiltoption.ui )

kde4_add_plugin(kritatangentnormalpaintop ${kritatangentnormalpaintop_PART_SRCS})

target_link_libraries(kritatangentnormalpaintop kritalibpaintop)

install(TARGETS kritatangentnormalpaintop DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES  kritatangentnormalpaintop.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
install( FILES  krita-tangetnormal.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)
install( FILES  krita-tangentnormal-preview.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)

