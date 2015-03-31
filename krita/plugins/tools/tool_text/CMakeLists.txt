

set(kritatooltext_PART_SRCS
    tool_text.cc
    kis_tool_text.cc
    kis_text_tool_option_widget.cpp
    )


ki18n_wrap_ui(kritatooltext_PART_SRCS wdgtextoptions.ui)

add_library(kritatooltext MODULE ${kritatooltext_PART_SRCS})
kcoreaddons_desktop_to_json(kritatooltext kritatooltext.desktop)


target_link_libraries(kritatooltext kritaui)

install(TARGETS kritatooltext  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  dark_draw-text.png light_draw-text.png DESTINATION ${DATA_INSTALL_DIR}/krita/pics)
install( FILES  kritatooltext.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)


