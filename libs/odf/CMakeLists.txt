add_subdirectory( tests )
add_subdirectory( writeodf )

include_directories( ${KOODF_INCLUDES} )

########### libkoodf ###############

set(koodf_LIB_SRCS
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
    writeodf/helpers.cpp
)

add_library(koodf SHARED ${koodf_LIB_SRCS})
add_dependencies(koodf writeodf.h-target)
generate_export_header(koodf BASE_NAME koodf)

target_link_libraries(koodf koversion kostore KF5::Archive KF5::Wallet KF5::KIOWidgets KF5::I18n Qt5::PrintSupport Qt5::Gui Qt5::Xml KF5::KDELibs4Support)
target_link_libraries(koodf LINK_INTERFACE_LIBRARIES kostore KF5::I18n Qt5::PrintSupport Qt5::Gui Qt5::Xml  KF5::KDELibs4Support)

set_target_properties(koodf PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS koodf ${INSTALL_TARGETS_DEFAULT_ARGS} )


install( FILES
    ${CMAKE_CURRENT_BINARY_DIR}/koodf_export.h
    KoOdf.h
    KoOdfManifestEntry.h
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
    KoUnit.h
    KoElementReference.h
    KoTableTemplate.h
DESTINATION ${INCLUDE_INSTALL_DIR}/calligra COMPONENT Devel)
