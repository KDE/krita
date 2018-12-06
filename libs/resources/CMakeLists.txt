add_subdirectory(tests)
add_subdirectory(dbexplorer)

set(kritaresources_LIB_SRCS
    KisResourceCacheDb.cpp
    KisResourceLoader.cpp
    KisResourceLoaderRegistry.cpp
    KisResourceLocator.cpp
    KisResourceStorage.cpp
    KisResourceModel.cpp
    KisResourceTypeModel.cpp

    KisStoragePlugin.cpp
    KisAbrStorage.cpp
    KisAslStorage.cpp
    KisBundleStorage.cpp
    KisFolderStorage.cpp

    KisTag.cpp
    KisTagModel.cpp

    KoResource.cpp
    KoResourceBundle.cpp
    KoResourceBundleManifest.cpp
    KoMD5Generator.cpp
    KoHashGeneratorProvider.cpp
    KoResourcePaths.cpp

    kconfigini.cpp
    kconfigdata.cpp
    kconfigbackend.cpp
)

qt5_add_resources(kritaresources_LIB_SRCS sql.qrc)

add_library(kritaresources SHARED ${kritaresources_LIB_SRCS})

generate_export_header(kritaresources BASE_NAME kritaresources)

target_link_libraries(kritaresources
    PUBLIC
        Qt5::Core
        Qt5::Widgets
    PRIVATE
        Qt5::Sql
        kritaversion
        kritaglobal
        kritaplugin
        kritastore
        KF5::ConfigCore
        KF5::CoreAddons
        KF5::I18n
        KF5::Archive
)

set_target_properties(kritaresources PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)
install(TARGETS kritaresources ${INSTALL_TARGETS_DEFAULT_ARGS} )

