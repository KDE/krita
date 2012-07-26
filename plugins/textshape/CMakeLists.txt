project( textPlugin)

add_subdirectory( pics )
#add_subdirectory( tests )

include_directories(
    ${KOTEXT_INCLUDES}
    ${TEXTLAYOUT_INCLUDES}
    ${FLAKE_INCLUDES}
    ${KOMAIN_INCLUDES}
    ${CMAKE_SOURCE_DIR}/libs/widgets/colorwidgets/
    ${KDE4_INCLUDE_DIR}/threadweaver/)

########### Flake Plugin library ###############

SET ( textshape_SRCS
    TextPlugin.cpp
    TextShape.cpp
    TextShapeFactory.cpp
    TextTool.cpp
    TextEditingPluginContainer.cpp
    TextToolFactory.cpp
    ShrinkToFitShapeContainer.cpp
    SimpleRootAreaProvider.cpp

    ChangeTracker.cpp
    ReviewTool.cpp
    ReviewToolFactory.cpp
    TextChanges.cpp
    TextChange.cpp
    FontSizeAction.cpp

    ReferencesTool.cpp
    ReferencesToolFactory.cpp

    dialogs/StylesWidget.cpp
    dialogs/SpecialButton.cpp
    dialogs/StylesCombo.cpp
    dialogs/StylesComboPreview.cpp
    dialogs/SimpleCharacterWidget.cpp
    dialogs/SimpleParagraphWidget.cpp
    dialogs/SimpleTableWidget.cpp
    dialogs/SimpleInsertWidget.cpp
    dialogs/SimpleTableOfContentsWidget.cpp
    dialogs/SimpleCitationBibliographyWidget.cpp
    dialogs/CitationInsertionDialog.cpp
    dialogs/InsertBibliographyDialog.cpp
    dialogs/SimpleFootEndNotesWidget.cpp
    dialogs/NotesConfigurationDialog.cpp
    dialogs/SimpleCaptionsWidget.cpp
    dialogs/ParagraphLayout.cpp
    dialogs/ParagraphIndentSpacing.cpp
    dialogs/ParagraphDecorations.cpp
    dialogs/ParagraphBulletsNumbers.cpp
    dialogs/ParagraphSettingsDialog.cpp
    dialogs/ParagraphDropCaps.cpp
    dialogs/ListsSpinBox.cpp
    dialogs/StylesModel.cpp
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

    commands/ChangeListLevelCommand.cpp
    commands/ShowChangesCommand.cpp
    commands/AcceptChangeCommand.cpp
    commands/RejectChangeCommand.cpp
    commands/AutoResizeCommand.cpp
)

kde4_add_ui_files(textshape_SRCS
    dialogs/SimpleCharacterWidget.ui
    dialogs/SimpleParagraphWidget.ui
    dialogs/SimpleTableWidget.ui
    dialogs/SimpleInsertWidget.ui
    dialogs/SimpleTableOfContentsWidget.ui
    dialogs/SimpleCitationBibliographyWidget.ui
    dialogs/CitationInsertionDialog.ui
    dialogs/InsertBibliographyDialog.ui
    dialogs/SimpleFootEndNotesWidget.ui
    dialogs/NotesConfigurationDialog.ui
    dialogs/SimpleCaptionsWidget.ui
    dialogs/StylesWidget.ui
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
)

kde4_add_plugin(textshape ${textshape_SRCS})

target_link_libraries(textshape kotext textlayout komain ${EXTRA_LIBS})
if( Soprano_FOUND )
    target_link_libraries(textshape ${SOPRANO_LIBRARIES})
endif( Soprano_FOUND )


install(TARGETS textshape DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES textshape.desktop DESTINATION ${SERVICES_INSTALL_DIR})
#kde4_install_icons( ${DATA_INSTALL_DIR}/calligra/icons )
