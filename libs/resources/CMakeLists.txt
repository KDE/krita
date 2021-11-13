include_directories(${QUAZIP_INCLUDE_DIRS})

add_subdirectory(tests)

set(kritaresources_LIB_SRCS
    KisResourceCacheDb.cpp
    KisResourceLoader.cpp
    KisResourceLoaderRegistry.cpp
    KisResourceLocator.cpp
    KisResourceStorage.cpp
    KisResourceModel.cpp
    KisTagFilterResourceProxyModel.cpp
    KisResourceModelProvider.cpp
    KisResourceTypeModel.cpp
    KisStorageModel.cpp
    KisStorageFilterProxyModel.cpp
    KisResourceIterator.cpp
    KisResourceSearchBoxFilter.cpp
    KisResourceQueryMapper.cpp
    KisResourceTypes.cpp

    KisStoragePlugin.cpp
    KisBundleStorage.cpp
    KisFolderStorage.cpp
    KisMemoryStorage.cpp

    KisTag.cpp
    KisTagModel.cpp

    KisTagResourceModel.cpp

    KoResource.cpp
    KoResourceBundle.cpp
    KoResourceBundleManifest.cpp
    KoMD5Generator.cpp
    KoResourcePaths.cpp
    KoResourceSignature.cpp
    KoEmbeddedResource.cpp
    KoResourceLoadResult.cpp

    ResourceDebug.cpp

    kconfigini.cpp
    kconfigdata.cpp
    kconfigbackend.cpp

    KisRequiredResourcesOperators.cpp
    KisResourcesInterface.cpp
    KisLocalStrokeResources.cpp
    KisGlobalResourcesInterface.cpp
    KoResourceCacheInterface.cpp
    KoResourceCacheStorage.cpp
    KoResourceCachePrefixedStorageWrapper.cpp

    KoCanvasResourcesInterface.cpp
    KoLocalStrokeCanvasResources.cpp
)

qt5_add_resources(kritaresources_LIB_SRCS sql.qrc)

add_library(kritaresources SHARED ${kritaresources_LIB_SRCS})

generate_export_header(kritaresources BASE_NAME kritaresources)

target_link_libraries(kritaresources
    PUBLIC
        Qt5::Core
        Qt5::Widgets
        Qt5::Sql
    PRIVATE
        kritaversion
        kritaglobal
        kritaplugin
        kritastore
        KF5::ConfigCore
        KF5::CoreAddons
        KF5::I18n
        ${QUAZIP_LIBRARIES}
)

set_target_properties(kritaresources PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)
install(TARGETS kritaresources ${INSTALL_TARGETS_DEFAULT_ARGS} )

