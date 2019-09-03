project( textPlugin)


if (${Qt5_VERSION} VERSION_GREATER "5.8.0" )
    remove_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x50900)
elseif(${Qt5_VERSION} VERSION_GREATER "5.7.0" )
    remove_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x50800)
elseif(${Qt5_VERSION} VERSION_GREATER "5.6.0" )
    remove_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x50700)
else()
    remove_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x50600)
endif()

add_definitions(
  -DQT_DISABLE_DEPRECATED_BEFORE=0
)

kis_define_library_level_pch(krita_text_pch ${CMAKE_SOURCE_DIR}/pch/kis_qt_only_pch.h "Qt5::Core;Qt5::Gui")
macro(kis_add_legacy_text_library)
    kis_add_library_pch(PCH_TARGET krita_text_pch ${ARGV})
endmacro()

add_subdirectory( textlayout )
add_subdirectory( kotext )

set ( textshape_SRCS
    TextPlugin.cpp
    TextShape.cpp
    TextShapeFactory.cpp
    TextTool.cpp
    TextEditingPluginContainer.cpp
    TextToolFactory.cpp
    ShrinkToFitShapeContainer.cpp
    SimpleRootAreaProvider.cpp

    FontSizeAction.cpp
    FontFamilyAction.cpp

    dialogs/StylesCombo.cpp
    dialogs/StylesComboPreview.cpp
    dialogs/DockerStylesComboModel.cpp
    dialogs/SimpleCharacterWidget.cpp
    dialogs/SimpleParagraphWidget.cpp
    dialogs/SimpleTableWidget.cpp
    dialogs/SimpleInsertWidget.cpp
    dialogs/ParagraphLayout.cpp
    dialogs/ParagraphIndentSpacing.cpp
    dialogs/ParagraphDecorations.cpp
    dialogs/ParagraphBulletsNumbers.cpp
    dialogs/ParagraphSettingsDialog.cpp
    dialogs/ParagraphDropCaps.cpp
    dialogs/ListsSpinBox.cpp
    dialogs/StylesModel.cpp
    dialogs/StylesManagerModel.cpp
    dialogs/StylesSortFilterProxyModel.cpp
    dialogs/AbstractStylesModel.cpp
    dialogs/StylesFilteredModelBase.cpp
    dialogs/ValidParentStylesProxyModel.cpp
    dialogs/StylesDelegate.cpp
    dialogs/StyleManager.cpp
    dialogs/StyleManagerDialog.cpp
    dialogs/ParagraphGeneral.cpp
    dialogs/CharacterGeneral.cpp
    dialogs/CharacterHighlighting.cpp
    dialogs/InsertCharacter.cpp
    dialogs/FontDia.cpp
    dialogs/FontDecorations.cpp
    dialogs/LanguageTab.cpp
    dialogs/FormattingPreview.cpp
    dialogs/StyleManagerWelcome.cpp
    dialogs/TableDialog.cpp
    dialogs/QuickTableButton.cpp
    dialogs/FormattingButton.cpp
    dialogs/ChangeConfigureDialog.cpp
    dialogs/AcceptRejectChangeDialog.cpp
    dialogs/TrackedChangeModel.cpp
    dialogs/TrackedChangeManager.cpp
    dialogs/BibliographyConfigureDialog.cpp
    dialogs/TableOfContentsConfigure.cpp
    dialogs/TableOfContentsStyleConfigure.cpp
    dialogs/TableOfContentsStyleModel.cpp
    dialogs/TableOfContentsStyleDelegate.cpp
    dialogs/TableOfContentsPreview.cpp
    dialogs/TableOfContentsEntryDelegate.cpp
    dialogs/TableOfContentsEntryModel.cpp
    dialogs/TableOfContentsTemplate.cpp
    dialogs/BibliographyTemplate.cpp
    dialogs/BibliographyPreview.cpp
    dialogs/ListLevelChooser.cpp
    dialogs/SectionFormatDialog.cpp
    dialogs/SectionsSplitDialog.cpp

    commands/ChangeListLevelCommand.cpp
    commands/ShowChangesCommand.cpp
    commands/AcceptChangeCommand.cpp
    commands/RejectChangeCommand.cpp
    commands/AutoResizeCommand.cpp
)

ki18n_wrap_ui(textshape_SRCS
    dialogs/SimpleCharacterWidget.ui
    dialogs/SimpleParagraphWidget.ui
    dialogs/SimpleTableWidget.ui
    dialogs/SimpleInsertWidget.ui
    dialogs/ParagraphLayout.ui
    dialogs/ParagraphIndentSpacing.ui
    dialogs/ParagraphDecorations.ui
    dialogs/ParagraphBulletsNumbers.ui
    dialogs/ParagraphDropCaps.ui
    dialogs/StyleManager.ui
    dialogs/CharacterGeneral.ui
    dialogs/CharacterHighlighting.ui
    dialogs/StyleManagerWelcome.ui
    dialogs/TableDialog.ui
    dialogs/BibliographyConfigureDialog.ui
    dialogs/TableOfContentsConfigure.ui
    dialogs/TableOfContentsStyleConfigure.ui


    dialogs/FontDecorations.ui
    dialogs/LanguageTab.ui
    dialogs/ChangeConfigureDialog.ui
    dialogs/AcceptRejectChangeDialog.ui
    dialogs/TrackedChangeManager.ui
    dialogs/SectionFormatDialog.ui
    dialogs/SectionsSplitDialog.ui

)
qt5_add_resources(textshape_SRCS textshape.qrc)

kis_add_legacy_text_library(krita_shape_text MODULE ${textshape_SRCS})

target_link_libraries(krita_shape_text kritatext kritatextlayout kritawidgetutils kritawidgets Qt5::Network KF5::Completion KF5::ItemViews KF5::WidgetsAddons
)

if( SHOULD_BUILD_FEATURE_RDF )
    target_link_libraries(krita_shape_text ${SOPRANO_LIBRARIES})
endif()


install(TARGETS krita_shape_text DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
