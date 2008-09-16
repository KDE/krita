project( paragraphPlugin )

include_directories( ${KOGUIUTILS_INCLUDES} ${KOTEXT_INCLUDES} ${KDE4_INCLUDE_DIR}/threadweaver/)

########### Flake Plugin library ###############

SET ( paragraphtool_SRCS
    ParagraphPlugin.cpp
    ParagraphToolFactory.cpp
    ParagraphTool.cpp
    ParagraphFragment.cpp
    Ruler.cpp
    RulerFragment.cpp
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
