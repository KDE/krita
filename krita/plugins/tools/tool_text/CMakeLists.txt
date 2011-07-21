

set(kritatooltext_PART_SRCS
    tool_text.cc
    kis_tool_text.cc
    kis_text_tool_option_widget.cpp
    )


kde4_add_ui_files(kritatooltext_PART_SRCS wdgtextoptions.ui)

kde4_add_plugin(kritatooltext ${kritatooltext_PART_SRCS})



target_link_libraries(kritatooltext kritaui)

install(TARGETS kritatooltext  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

# install( FILES  tool_crop_cursor.png tool_crop.png DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  kritatooltext.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


