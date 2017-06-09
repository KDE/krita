project(SvgTextTool)

set (SvgTextTool_SRCS
    BasicXMLSyntaxHighlighter.cpp
    Plugin.cpp
    SvgTextChangeCommand.cpp
    SvgTextEditor.cpp
    SvgTextTool.cpp
    SvgTextToolFactory.cpp
)

ki18n_wrap_ui(SvgTextTool_SRCS 
    WdgSvgTextEditor.ui
)

add_library(krita_tool_svgtext MODULE ${SvgTextTool_SRCS})

target_link_libraries(krita_tool_svgtext kritaui KF5::I18n)

install(TARGETS krita_tool_svgtext DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})

