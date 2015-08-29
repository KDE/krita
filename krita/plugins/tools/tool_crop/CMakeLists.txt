

set(kritatoolcrop_PART_SRCS
    tool_crop.cc
    kis_tool_crop.cc
    kis_constrained_rect.cpp
    kistoolcropconfigwidget.cpp
    )


ki18n_wrap_ui(kritatoolcrop_PART_SRCS wdg_tool_crop.ui)

add_library(kritatoolcrop MODULE ${kritatoolcrop_PART_SRCS})
kcoreaddons_desktop_to_json(kritatoolcrop kritatoolcrop.desktop)



target_link_libraries(kritatoolcrop kritaui)

install(TARGETS kritatoolcrop  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  tool_crop_cursor.png DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  kritatoolcrop.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)


install( FILES
    KisToolCrop.action
DESTINATION  ${DATA_INSTALL_DIR}/krita/actions)
