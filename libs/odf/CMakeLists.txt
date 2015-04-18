add_subdirectory( tests )
add_subdirectory( writeodf )

include_directories( ${KOODF_INCLUDES} )

########### libkoodf ###############

if( Qca-qt5_FOUND )
    add_definitions( -DQCA2 )
endif()

set(koodf_LIB_SRCS
    KoOdf.cpp
    KoXmlNS.cpp
    KoOdfManifestEntry.cpp
    KoDom.cpp
    KoDocumentInfo.cpp
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
    KoDocumentBase.cpp
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
    KoLZF.cpp
    KoXmlReader.cpp
    KoXmlWriter.cpp
    KoEncryptedStore.cpp
    KoEncryptionChecker.cpp
    KoUnit.cpp
    KoOdfNotesConfiguration.cpp
    KoOdfBibliographyConfiguration.cpp
    KoOdfNumberDefinition.cpp
    KoOdfLineNumberingConfiguration.cpp
    KoElementReference.cpp
    writeodf/helpers.cpp
)

add_library(koodf SHARED ${koodf_LIB_SRCS})
add_dependencies(koodf writeodf.h-target)

target_link_libraries(koodf KF5::Archive KF5::Wallet KF5::KIOWidgets KF5::KDELibs4Support Qt5::Xml)
target_link_libraries(koodf LINK_INTERFACE_LIBRARIES KF5::KIOWidgets KF5::KDELibs4Support Qt5::Xml)
if( Qca-qt5_FOUND )
    target_link_libraries(koodf qca-qt5)
endif()

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
    KoDocumentBase.h
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
    KoUnit.h
    KoElementReference.h
    KoTableTemplate.h
DESTINATION ${INCLUDE_INSTALL_DIR}/calligra COMPONENT Devel)
