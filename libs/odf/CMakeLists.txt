add_subdirectory( tests )

include_directories( ${KOODF_INCLUDES} )

########### libkoodf ###############

if( QCA2_FOUND )
    include_directories( ${QCA2_INCLUDE_DIR} )
    add_definitions( -DQCA2 )
endif( QCA2_FOUND )

set(koodf_LIB_SRCS
    KoOdf.cpp
    KoXmlNS.cpp
    KoOdfManifestEntry.cpp
    KoDom.cpp
    KoGenStyle.cpp
    KoGenStyles.cpp
    KoFontFace.cpp
    KoOdfLoadingContext.cpp
    KoOasisSettings.cpp
    KoOdfStylesReader.cpp
    KoOdfNumberStyles.cpp
    KoOdfPaste.cpp
    KoOdfReadStore.cpp
    KoOdfWriteStore.cpp
    KoStyleStack.cpp
    KoOdfGraphicStyles.cpp
    KoGenChange.cpp
    KoGenChanges.cpp
    KoOdfDocument.cpp
    KoEmbeddedDocumentSaver.cpp
    KoBorder.cpp
    Ko3dScene.cpp
    KoShadowStyle.cpp
    KoPageLayout.cpp
    KoPageFormat.cpp
    KoColumns.cpp
    KoStore.cpp
    KoTarStore.cpp
    KoDirectoryStore.cpp
    KoZipStore.cpp
    KoXmlReader.cpp
    KoXmlWriter.cpp
    KoEncryptedStore.cpp
    KoEncryptionChecker.cpp
    KoProperties.cpp
    KoUnit.cpp
    KoOdfNotesConfiguration.cpp
    KoOdfBibliographyConfiguration.cpp
    KoOdfNumberDefinition.cpp
    KoOdfLineNumberingConfiguration.cpp
    KoCell.cpp
    KoCellChild.cpp
    KoCellValue.cpp
    KoDummyCellValue.cpp
    KoCellStyle.cpp
    KoColumn.cpp
    KoRawCellChild.cpp
    KoRow.cpp
    KoStyle.cpp
    KoTable.cpp
    KoColumnStyle.cpp
    KoRowStyle.cpp
    KoTblStyle.cpp
    KoElementReference.cpp
)

kde4_add_library(koodf SHARED ${koodf_LIB_SRCS})

target_link_libraries(koodf ${KDE4_KIO_LIBS} ${QT_QTXML_LIBRARY})
target_link_libraries(koodf LINK_INTERFACE_LIBRARIES ${KDE4_KIO_LIBS})
if( QCA2_FOUND )
    target_link_libraries(koodf ${QCA2_LIBRARIES})
endif( QCA2_FOUND )

set_target_properties(koodf PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS koodf ${INSTALL_TARGETS_DEFAULT_ARGS} )


install( FILES
    koodf_export.h
    KoOdf.h
    KoXmlNS.h
    KoOdfManifestEntry.h
    KoDom.h
    KoGenStyle.h
    KoGenStyles.h
    KoFontFace.h
    KoOdfLoadingContext.h
    KoOasisSettings.h
    KoOdfStylesReader.h
    KoOdfNotesConfiguration.h
    KoOdfBibliographyConfiguration.h
    KoOdfNumberStyles.h
    KoOdfNumberDefinition.h
    KoOdfLineNumberingConfiguration.h
    KoOdfPaste.h
    KoOdfReadStore.h
    KoOdfWriteStore.h
    KoStyleStack.h
    KoOdfGraphicStyles.h
    KoOdfDocument.h
    KoGenChange.h
    KoGenChanges.h
    KoEmbeddedDocumentSaver.h
    KoBorder.h
    Ko3dScene.h
    KoShadowStyle.h
    KoPageLayout.h
    KoPageFormat.h
    KoColumns.h
    KoStore.h
    KoStoreDevice.h
    KoXmlReader.h
    KoXmlWriter.h
    KoXmlReaderForward.h
    KoProperties.h
    KoUnit.h
    KoElementReference.h
    KoTableTemplate.h
DESTINATION ${INCLUDE_INSTALL_DIR} COMPONENT Devel)

