set(kritacolorsmudgepaintop_PART_SRCS
    colorsmudge_paintop_plugin.cpp
    kis_colorsmudgeop.cpp
    kis_colorsmudgeop_settings_widget.cpp
    kis_rate_option.cpp
    kis_rate_option_widget.cpp
    kis_smudge_option.cpp
    kis_smudge_option_widget.cpp
)

kde4_add_plugin(kritacolorsmudgepaintop ${kritacolorsmudgepaintop_PART_SRCS})


target_link_libraries(kritacolorsmudgepaintop kritalibpaintop)

install(TARGETS kritacolorsmudgepaintop DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES  kritacolorsmudgepaintop.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES  krita-colorsmudge.png DESTINATION ${DATA_INSTALL_DIR}/krita/images)

