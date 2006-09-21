

set(kritatoolcrop_PART_SRCS
    tool_crop.cc
    kis_tool_crop.cc
    )

kde4_automoc(${kritatoolcrop_PART_SRCS})

kde4_add_ui_files(kritatoolcrop_PART_SRCS wdg_tool_crop.ui)

kde4_add_plugin(kritatoolcrop ${kritatoolcrop_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kritatoolcrop )

target_link_libraries(kritatoolcrop kritaui)

install(TARGETS kritatoolcrop  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  tool_crop_cursor.png  DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  kritatoolcrop.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


