project(SvgTextTool)

set(CMAKE_AUTORCC ON)

set (SvgTextTool_SRCS
    BasicXMLSyntaxHighlighter.cpp
    Plugin.cpp
    SvgTextChangeCommand.cpp
    SvgRichTextCtrl.cpp
    SvgTextEditor.cpp
    SvgTextTool.cpp
    SvgTextToolFactory.cpp
    svgtexttool.qrc
    kis_font_family_combo_box.cpp
    FontSizeAction.cpp
)

ki18n_wrap_ui(SvgTextTool_SRCS 
    WdgSvgTextEditor.ui
    WdgSvgTextSettings.ui
)

install( FILES
    SvgTextTool.action
    DESTINATION  ${DATA_INSTALL_DIR}/krita/actions
)


kis_add_library(krita_tool_svgtext MODULE ${SvgTextTool_SRCS})
target_link_libraries(krita_tool_svgtext kritaui KF5::I18n)
install(TARGETS krita_tool_svgtext DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
