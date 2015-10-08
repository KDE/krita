add_subdirectory( tests )

include_directories( ${KOODF_INCLUDES} )

########### libkritaodf ###############

set(kritaodf_LIB_SRCS
    KoOdf.cpp
    KoOdfManifestEntry.cpp
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
    KoUnit.cpp
    KoOdfNotesConfiguration.cpp
    KoOdfBibliographyConfiguration.cpp
    KoOdfNumberDefinition.cpp
    KoOdfLineNumberingConfiguration.cpp
    KoElementReference.cpp

    OdfDebug.cpp
)

add_library(kritaodf SHARED ${kritaodf_LIB_SRCS})
generate_export_header(kritaodf BASE_NAME kritaodf)

target_link_libraries(kritaodf kritaversion kritastore KF5::CoreAddons KF5::ConfigCore KF5::I18n Qt5::PrintSupport Qt5::Gui Qt5::Xml)
target_link_libraries(kritaodf LINK_INTERFACE_LIBRARIES kritastore KF5::CoreAddons KF5::I18n Qt5::PrintSupport Qt5::Gui Qt5::Xml)

set_target_properties(kritaodf PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS kritaodf ${INSTALL_TARGETS_DEFAULT_ARGS} )
