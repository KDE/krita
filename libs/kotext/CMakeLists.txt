include_directories(${KOTEXT_INCLUDES}
                    ${FONTCONFIG_INCLUDE_DIR}/fontconfig 
                    ${FREETYPE_INCLUDE_DIRS})

add_subdirectory( tests )
add_subdirectory( styles/tests )
add_subdirectory( opendocument/tests )

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

    KoInlineTextObjectManager.cpp
    KoInlineObjectFactoryBase.cpp
    KoInlineObjectRegistry.cpp
    InsertInlineObjectActionBase.cpp
    InsertVariableAction.cpp
    InsertNamedVariableAction.cpp
    InsertTextReferenceAction.cpp
    InsertTextLocator.cpp
    KoInlineObject.cpp
    KoVariable.cpp
    KoVariableManager.cpp
    KoNamedVariable.cpp
    KoSection.cpp
    KoTextLocator.cpp
    KoTextReference.cpp
    KoTextAnchor.cpp
    KoTextShapeSavingContext.cpp
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
    styles/ChangeFollower.cpp
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
    changetracker/KoDeleteChangeMarker.cpp
    changetracker/KoFormatChangeInformation.cpp
    changetracker/KoDeletedRowColumnDataStore.cpp
    changetracker/KoDeletedRowData.cpp
    changetracker/KoDeletedColumnData.cpp
    changetracker/KoDeletedCellData.cpp

    commands/ChangeAnchorPropertiesCommand.cpp
    commands/ChangeListCommand.cpp
    commands/ChangeStylesCommand.cpp
    commands/ChangeStylesMacroCommand.cpp
    commands/ChangeTrackedDeleteCommand.cpp
    commands/DeleteAnchorsCommand.cpp
    commands/DeleteCommand.cpp
    commands/DeleteTableColumnCommand.cpp
    commands/DeleteTableRowCommand.cpp
    commands/InsertDeleteChangesCommand.cpp
    commands/InsertNoteCommand.cpp
    commands/InsertTableColumnCommand.cpp
    commands/InsertTableRowCommand.cpp
    commands/ResizeTableCommand.cpp
    commands/InsertInlineObjectCommand.cpp
    commands/ListItemNumberingCommand.cpp
    commands/RemoveDeleteChangesCommand.cpp
    commands/TextPasteCommand.cpp

    KoTextDrag.cpp
    KoTextCommandBase.cpp
)

if( Soprano_FOUND )
    set(kotext_LIB_SRCS ${kotext_LIB_SRCS}
        KoTextRdfCore.cpp
    )
endif( Soprano_FOUND )

kde4_add_library(kotext SHARED ${kotext_LIB_SRCS})

target_link_libraries(kotext flake)
if( Soprano_FOUND )
    target_link_libraries(kotext ${SOPRANO_LIBRARIES})
endif( Soprano_FOUND )
target_link_libraries(kotext LINK_INTERFACE_LIBRARIES flake)

if( FONTCONFIG_FOUND )
    target_link_libraries(kotext ${FONTCONFIG_LIBRARIES})
endif( FONTCONFIG_FOUND )

if( FREETYPE_FOUND )
    target_link_libraries(kotext ${FREETYPE_LIBRARIES})
endif( FREETYPE_FOUND )


set_target_properties(kotext PROPERTIES
   VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS kotext  ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( FILES texteditingplugin.desktop inlinetextobject.desktop textvariableplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

install(
    FILES
    kotext_export.h
    KoDocumentRdfBase.h
    KoInlineObject.h
    KoTextEditor.h
    KoTextEditingFactory.h
    KoTextEditingPlugin.h
    KoTextEditingRegistry.h
    KoInlineObjectRegistry.h
    KoInlineObjectFactoryBase.h
    KoBookmark.h
    KoBookmarkManager.h
    KoInlineTextObjectManager.h
    KoTextAnchor.h
    KoTextBlockBorderData.h
    KoTextBlockData.h
    KoTextDocument.h
    KoText.h
    KoList.h
    KoTextLocator.h
    KoTextPage.h
    KoTextPaste.h
    KoVariable.h
    KoVariableManager.h
    KoTextRdfCore.h
    KoTextInlineRdf.h
    KoTextMeta.h
    KoTextSoftPageBreak.cpp
    KoSection.h
    KoTextCommandBase.h
    KoTextTableTemplate.h

    DESTINATION
    ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

install(
    FILES
    styles/KoCharacterStyle.h
    styles/KoListLevelProperties.h
    styles/KoListStyle.h
    styles/KoParagraphStyle.h
    styles/KoTableColumnStyle.h
    styles/KoTableRowStyle.h
    styles/KoTableCellStyle.h
    styles/KoSectionStyle.h
    styles/KoStyleManager.h
    styles/KoTableStyle.h
    DESTINATION
    ${INCLUDE_INSTALL_DIR}/styles COMPONENT Devel
)

install(
    FILES
    changetracker/KoChangeTracker.h
    changetracker/KoChangeTrackerElement.h
    changetracker/KoDeleteChangeMarker.h
    changetracker/KoDeletedRowColumnDataStore.h
    changetracker/KoDeletedRowData.cpp
    changetracker/KoDeletedColumnData.cpp
    changetracker/KoDeletedCellData.cpp
    DESTINATION
    ${INCLUDE_INSTALL_DIR}/changetracker COMPONENT Devel
)
