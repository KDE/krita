include_directories(${KOTEXT_INCLUDES})

add_subdirectory( tests )
add_subdirectory( styles/tests )
add_subdirectory( opendocument/tests )

########### next target ###############
set(kotext_LIB_SRCS
    KoTextShapeData.cpp
    KoText.cpp
    KoTextBlockData.cpp
    KoTextBlockBorderData.cpp
    KoTextBlockPaintStrategyBase.cpp
    KoTextDocumentLayout.cpp
    KoTextOdfSaveHelper.cpp
    KoTextPaste.cpp
    KoTextDocument.cpp
    KoTextEditor.cpp
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
    KoTextLocator.cpp
    KoTextReference.cpp
    KoTextAnchor.cpp
    KoTextShapeContainerModel.cpp
    KoTextShapeSavingContext.cpp
    KoBookmark.cpp
    KoBookmarkManager.cpp
    KoInlineNote.cpp
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
    styles/KoTableFormat.cpp
    styles/KoTableFormat_p.cpp
    styles/KoSectionStyle.cpp
    opendocument/KoTextSharedLoadingData.cpp
    opendocument/KoTextSharedSavingData.cpp
    opendocument/KoTextLoader.cpp
    opendocument/KoTextWriter.cpp

    changetracker/KoChangeTracker.cpp
    changetracker/KoChangeTrackerElement.cpp
    changetracker/KoDeleteChangeMarker.cpp
    KoTextDrag.cpp
)

if( Soprano_FOUND )
    set(kotext_LIB_SRCS ${kotext_LIB_SRCS}
        KoTextRdfCore.cpp
    )
endif( Soprano_FOUND )

kde4_add_library(kotext SHARED ${kotext_LIB_SRCS})

target_link_libraries(kotext flake ${KDE4_KUTILS_LIBS})
if( Soprano_FOUND )
    target_link_libraries(kotext ${SOPRANO_LIBRARIES})
endif( Soprano_FOUND )
target_link_libraries(kotext LINK_INTERFACE_LIBRARIES flake)

set_target_properties(kotext PROPERTIES
   VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS kotext  ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install( FILES texteditingplugin.desktop inlinetextobject.desktop textvariableplugin.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

install(
    FILES
    kotext_export.h
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
    KoTextShapeData.h
    KoVariable.h
    KoVariableManager.h
    KoTextRdfCore.h
    KoTextInlineRdf.h
    KoTextMeta.h

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
    styles/KoTableFormat.h
    styles/KoTableColumnFormat.h
    styles/KoTableRowFormat.h
    styles/KoSectionStyle.h
    styles/KoStyleManager.h
    DESTINATION
    ${INCLUDE_INSTALL_DIR}/styles COMPONENT Devel
)

install(
    FILES
    changetracker/KoChangeTracker.h
    changetracker/KoChangeTrackerElement.h
    changetracker/KoDeleteChangeMarker.h
    DESTINATION
    ${INCLUDE_INSTALL_DIR}/changetracker COMPONENT Devel
)
