

set(kritatooltext_SOURCES
    tool_text.cc
    kis_tool_text.cc
    kis_text_tool_option_widget.cpp
    )


ki18n_wrap_ui(kritatooltext_SOURCES wdgtextoptions.ui)

add_library(kritatooltext MODULE ${kritatooltext_SOURCES})
kcoreaddons_desktop_to_json(kritatooltext kritatooltext.desktop)


target_link_libraries(kritatooltext kritaui)

install(TARGETS kritatooltext  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
