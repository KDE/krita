project( textPlugin)

add_subdirectory( tests )

include_directories( ${KOGUIUTILS_INCLUDES} ${KOTEXT_INCLUDES} ${KDE4_INCLUDE_DIR}/threadweaver/)

########### Flake Plugin library ###############

SET ( textshape_SRCS
    TextPlugin.cpp
    TextShape.cpp
    TextShapeFactory.cpp
    TextTool.cpp
    TextToolFactory.cpp
    Layout.cpp
    ListItemsHelper.cpp
    PluginHelperAction.cpp
    ParagraphTool.cpp
    ParagraphToolFactory.cpp
    ShapeSpecificData.cpp
    Ruler.cpp

    ChangeTracker.cpp
    TextChanges.cpp
    TextChange.cpp

    dialogs/StylesWidget.cpp
    dialogs/SimpleStyleWidget.cpp
    dialogs/ParagraphLayout.cpp
    dialogs/ParagraphIndentSpacing.cpp
    dialogs/ParagraphBulletsNumbers.cpp
    dialogs/ParagraphSettingsDialog.cpp
    dialogs/ListsSpinBox.cpp
    dialogs/StyleManager.cpp
    dialogs/StyleManagerDialog.cpp
    dialogs/ParagraphGeneral.cpp
    dialogs/NewStyleWidget.cpp
    dialogs/CharacterGeneral.cpp
    dialogs/CharacterHighlighting.cpp
    dialogs/CharacterDecorations.cpp
    dialogs/CharacterStyleOptions.cpp
    dialogs/InsertCharacter.cpp
    dialogs/FontDia.cpp
    dialogs/FontLayoutTab.cpp
    dialogs/FontDecorations.cpp
    dialogs/FontTab.cpp
    dialogs/LanguageTab.cpp

    commands/ChangeListCommand.cpp
    commands/TextCommandBase.cpp
)

kde4_add_ui_files(textshape_SRCS
    dialogs/SimpleStyleWidget_2.ui
    dialogs/StylesWidget.ui
    dialogs/ParagraphLayout.ui
    dialogs/ParagraphIndentSpacing.ui
    dialogs/ParagraphBulletsNumbers.ui
    dialogs/StyleManager.ui
    dialogs/ParagraphGeneral.ui
    dialogs/NewStyleWidget.ui
    dialogs/CharacterGeneral.ui
    dialogs/CharacterDecorations.ui
    dialogs/CharacterHighlighting.ui
    dialogs/CharacterStyleOptions.ui

    dialogs/FontDecorations.ui
    dialogs/FontLayoutTab.ui
    dialogs/LanguageTab.ui
)

kde4_add_plugin(textshape ${textshape_SRCS})

target_link_libraries(textshape flake koguiutils kotext)
install(TARGETS textshape DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES textshape.desktop DESTINATION ${SERVICES_INSTALL_DIR})
#kde4_install_icons( ${DATA_INSTALL_DIR}/koffice/icons )
