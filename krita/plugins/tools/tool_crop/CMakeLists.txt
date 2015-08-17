

set(kritatoolcrop_PART_SRCS
    tool_crop.cc
    kis_tool_crop.cc
    kis_constrained_rect.cpp
    kistoolcropconfigwidget.cpp
    )


kde4_add_ui_files(kritatoolcrop_PART_SRCS wdg_tool_crop.ui)

kde4_add_plugin(kritatoolcrop ${kritatoolcrop_PART_SRCS})



target_link_libraries(kritatoolcrop kritaui)

install(TARGETS kritatoolcrop  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  tool_crop_cursor.png DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  kritatoolcrop.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)


install( FILES
    KisToolCrop.action
DESTINATION  ${DATA_INSTALL_DIR}/krita/actions)
