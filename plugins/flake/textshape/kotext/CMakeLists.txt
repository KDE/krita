include_directories(${FONTCONFIG_INCLUDE_DIR}
                    ${FREETYPE_INCLUDE_DIRS})

set(kritatext_LIB_SRCS
    KoDocumentRdfBase.cpp
    KoText.cpp
    KoTextBlockData.cpp
    KoTextBlockBorderData.cpp
    KoTextBlockPaintStrategyBase.cpp
    KoTextOdfSaveHelper.cpp
    KoTextDocument.cpp
    KoTextEditor.cpp
    KoTextEditor_undo.cpp
    KoTextEditor_format.cpp
    KoList.cpp

    KoTextEditingRegistry.cpp
    KoTextEditingFactory.cpp
    KoTextEditingPlugin.cpp

    KoTextRangeManager.cpp
    KoInlineTextObjectManager.cpp
    KoInlineObjectFactoryBase.cpp
    KoInlineObjectRegistry.cpp
    InsertInlineObjectActionBase_p.cpp
    InsertVariableAction.cpp
    InsertNamedVariableAction.cpp
    InsertTextReferenceAction.cpp
    InsertTextLocator.cpp
    KoInlineObject.cpp
    KoTextRange.cpp
    KoVariable.cpp
    KoVariableManager.cpp
    KoNamedVariable.cpp
    KoSection.cpp
    KoSectionEnd.cpp
    KoSectionUtils.cpp
    KoSectionModel.cpp
    KoTextLocator.cpp
    KoTextReference.cpp
    KoAnchorInlineObject.cpp
    KoAnchorTextRange.cpp
    KoTextShapeSavingContext.cpp
    KoAnnotation.cpp
    KoAnnotationManager.cpp
    KoBookmark.cpp
    KoBookmarkManager.cpp
    KoInlineNote.cpp
    KoInlineCite.cpp
    KoTextSoftPageBreak.cpp
    KoTextDebug.cpp
    KoTextPage.cpp
    KoPageProvider.cpp
    KoTableColumnAndRowStyleManager.cpp
    KoTextInlineRdf.cpp
    KoTextMeta.cpp
    KoTextTableTemplate.cpp

    OdfTextTrackStyles.cpp

    ToCBibGeneratorInfo.cpp
    KoTableOfContentsGeneratorInfo.cpp
    KoBibliographyInfo.cpp
    BibliographyGenerator.cpp

    styles/Styles_p.cpp
    styles/KoCharacterStyle.cpp
    styles/KoParagraphStyle.cpp
    styles/KoStyleManager.cpp
    styles/KoListStyle.cpp
    styles/KoListLevelProperties.cpp
    styles/KoTableStyle.cpp
    styles/KoTableColumnStyle.cpp
    styles/KoTableRowStyle.cpp
    styles/KoTableCellStyle.cpp
    styles/KoSectionStyle.cpp
    opendocument/KoTextSharedLoadingData.cpp
    opendocument/KoTextSharedSavingData.cpp
    opendocument/KoTextLoader.cpp
    opendocument/KoTextWriter_p.cpp
    opendocument/KoTextWriter.cpp

    changetracker/KoChangeTracker.cpp
    changetracker/KoChangeTrackerElement.cpp
    changetracker/KoFormatChangeInformation.cpp
    changetracker/KoDeletedRowColumnDataStore.cpp
    changetracker/KoDeletedRowData.cpp
    changetracker/KoDeletedColumnData.cpp
    changetracker/KoDeletedCellData.cpp

    commands/ChangeAnchorPropertiesCommand.cpp
    commands/ChangeListCommand.cpp
    commands/ChangeStylesCommand.cpp
    commands/ChangeStylesMacroCommand.cpp
    commands/DeleteAnchorsCommand.cpp
    commands/DeleteAnnotationsCommand.cpp
    commands/DeleteCommand.cpp
    commands/DeleteTableColumnCommand.cpp
    commands/DeleteTableRowCommand.cpp
    commands/InsertNoteCommand.cpp
    commands/InsertTableColumnCommand.cpp
    commands/InsertTableRowCommand.cpp
    commands/ResizeTableCommand.cpp
    commands/InsertInlineObjectCommand.cpp
    commands/ListItemNumberingCommand.cpp
    commands/TextPasteCommand.cpp
    commands/AddTextRangeCommand.cpp
    commands/AddAnnotationCommand.cpp
    commands/ParagraphFormattingCommand.cpp
    commands/RenameSectionCommand.cpp
    commands/NewSectionCommand.cpp
    commands/SplitSectionsCommand.cpp

    KoTextDrag.cpp
    KoTextCommandBase.cpp

    TextDebug.cpp
)

kis_add_legacy_text_library(kritatext SHARED ${kritatext_LIB_SRCS})
generate_export_header(kritatext BASE_NAME kritatext)

target_link_libraries(kritatext kritaflake Qt5::Gui kritawidgetutils )
target_link_libraries(kritatext LINK_INTERFACE_LIBRARIES kritaflake Qt5::Gui )

target_include_directories(kritatext
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/opendocument>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/changetracker>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/styles>
)
set_target_properties(kritatext PROPERTIES
   VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)
install(TARGETS kritatext  ${INSTALL_TARGETS_DEFAULT_ARGS})
