project( paragraphPlugin )

include_directories( ${KOGUIUTILS_INCLUDES} ${KOTEXT_INCLUDES} )

########### Flake Plugin library ###############

SET ( paragraphtool_SRCS
    ParagraphPlugin.cpp
    ParagraphToolFactory.cpp
    ParagraphTool.cpp
    ParagraphBase.cpp
    ParagraphEditor.cpp
    ParagraphHighlighter.cpp
    ParagraphFragment.cpp
    Ruler.cpp
    RulerFragment.cpp
    Label.cpp

    dialogs/OptionWidget.cpp
)

kde4_add_ui_files(
    paragraphtool_SRCS
    dialogs/OptionWidget.ui
    )

kde4_add_plugin(paragraphtool ${paragraphtool_SRCS})

target_link_libraries(
    paragraphtool
    flake
    koguiutils
    kotext
    komain
    kostore
    pigmentcms
    ${KDE4_KDEUI_LIBS}
    ${QT_QTCORE_LIBRARY}
    )
install(TARGETS paragraphtool DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES paragraphtool.desktop DESTINATION ${SERVICES_INSTALL_DIR})
