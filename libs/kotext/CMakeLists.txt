include_directories(${KOTEXT_INCLUDES}
                    ${FONTCONFIG_INCLUDE_DIR}/fontconfig
                    ${FREETYPE_INCLUDE_DIRS})

add_subdirectory( tests )
add_subdirectory( styles/tests )

########### next target ###############
set(kotext_LIB_SRCS
    KoDocumentRdfBase.cpp
    KoText.cpp
    KoTextBlockData.cpp
    KoTextBlockBorderData.cpp
    KoTextBlockPaintStrategyBase.cpp
    KoTextOdfSaveHelper.cpp
    KoTextPaste.cpp
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
    FindDirection_p.cpp
    KoFindStrategy.cpp
    KoReplaceStrategy.cpp
    KoFind_p.cpp
    KoFind.cpp
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

if( SHOULD_BUILD_FEATURE_RDF )
    set(kotext_LIB_SRCS ${kotext_LIB_SRCS}
        KoTextRdfCore.cpp
    )
endif()

add_library(kotext SHARED ${kotext_LIB_SRCS})
generate_export_header(kotext BASE_NAME kotext)

target_link_libraries(kotext flake KF5::TextWidgets KF5::XmlGui KF5::WindowSystem)
if( SHOULD_BUILD_FEATURE_RDF )
    target_link_libraries(kotext ${SOPRANO_LIBRARIES})
endif()
target_link_libraries(kotext LINK_INTERFACE_LIBRARIES flake KF5::TextWidgets)

if( FONTCONFIG_FOUND )
    target_link_libraries(kotext ${FONTCONFIG_LIBRARIES})
endif()

if( FREETYPE_FOUND )
    target_link_libraries(kotext ${FREETYPE_LIBRARIES})
endif()


set_target_properties(kotext PROPERTIES
   VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS kotext  ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( FILES texteditingplugin.desktop inlinetextobject.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})
