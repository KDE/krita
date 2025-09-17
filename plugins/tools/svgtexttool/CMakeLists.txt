project(SvgTextTool)
add_subdirectory(tests)

set(CMAKE_AUTORCC ON)

set (SvgTextTool_SRCS
    BasicXMLSyntaxHighlighter.cpp
    SvgCreateTextStrategy.cpp
    SvgInlineSizeChangeCommand.cpp
    SvgInlineSizeChangeStrategy.cpp
    SvgSelectTextStrategy.cpp
    SvgMoveTextCommand.cpp
    SvgMoveTextStrategy.cpp
    SvgTextChangeCommand.cpp
    SvgTextInsertCommand.cpp
    SvgTextInsertRichCommand.cpp
    SvgTextRemoveCommand.cpp
    SvgTextMergePropertiesRangeCommand.cpp
    SvgRichTextCtrl.cpp
    SvgTextEditor.cpp
    SvgTextCursor.cpp
    SvgTextTool.cpp
    SvgTextToolFactory.cpp
    FontSizeAction.cpp
    SvgTextShapeManagerBlocker.cpp
    SvgConvertTextTypeCommand.cpp
    SvgTextShortCuts.cpp

    glyphpalette/GlyphPaletteDialog.cpp
    glyphpalette/GlyphPaletteProxyModel.cpp
    glyphpalette/GlyphPaletteAltPopup.cpp
)

ki18n_wrap_ui(SvgTextTool_SRCS 
    WdgSvgTextEditor.ui
    WdgSvgTextOptionWidget.ui
    WdgSvgTextSettings.ui
)

install( FILES
    SvgTextTool.action
    DESTINATION  ${KDE_INSTALL_DATADIR}/krita/actions
)

set(SvgTextTool_SOURCES
    Plugin.cpp
    svgtexttool.qrc
)


kis_add_library(krita_tool_svgtext_static STATIC ${SvgTextTool_SRCS})
kis_add_library(krita_tool_svgtext MODULE ${SvgTextTool_SOURCES})
generate_export_header(krita_tool_svgtext_static BASE_NAME kritatoolsvgtext)

target_link_libraries(krita_tool_svgtext_static
    kritaui
    kritaqmlwidgets
    Qt${QT_MAJOR_VERSION}::Quick
    Qt${QT_MAJOR_VERSION}::QuickWidgets
    Qt${QT_MAJOR_VERSION}::QuickControls2
    Qt${QT_MAJOR_VERSION}::Qml
    KF${KF_MAJOR}::I18n)

target_link_libraries(krita_tool_svgtext PRIVATE krita_tool_svgtext_static)

install(TARGETS krita_tool_svgtext DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
