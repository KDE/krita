add_subdirectory( tests )

set(kritaodf_LIB_SRCS
    KoOdfManifestEntry.cpp
    KoGenStyle.cpp
    KoGenStyles.cpp
    KoFontFace.cpp
    KoOdfLoadingContext.cpp
    KoOdfStylesReader.cpp
    KoOdfReadStore.cpp
    KoOdfWriteStore.cpp
    KoStyleStack.cpp
    KoOdfGraphicStyles.cpp
    KoEmbeddedDocumentSaver.cpp
    KoShadowStyle.cpp
    KoElementReference.cpp

    OdfDebug.cpp
)

add_library(kritaodf SHARED ${kritaodf_LIB_SRCS})
generate_export_header(kritaodf BASE_NAME kritaodf)

target_link_libraries(kritaodf kritaglobal kritaversion kritaplugin kritastore KF5::CoreAddons KF5::ConfigCore KF5::I18n Qt5::PrintSupport Qt5::Gui Qt5::Xml)

set_target_properties(kritaodf PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)
install(TARGETS kritaodf ${INSTALL_TARGETS_DEFAULT_ARGS} )
