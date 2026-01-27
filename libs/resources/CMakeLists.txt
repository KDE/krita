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

    KisRequiredResourcesOperators.cpp
    KisResourcesInterface.cpp
    KisLocalStrokeResources.cpp
    KisGlobalResourcesInterface.cpp
    KoResourceCacheInterface.cpp
    KoResourceCacheStorage.cpp
    KoResourceCachePrefixedStorageWrapper.cpp

    KoCanvasResourcesInterface.cpp
    KoLocalStrokeCanvasResources.cpp
    KisEmbeddedResourceStorageProxy.cpp

    KisResourceThumbnailCache.cpp
    KisResourceMetaDataModel.cpp
    KisSqlQueryLoader.cpp
    KisDatabaseTransactionLock.cpp
    KisTemporaryResourceStorageLock.cpp
)

qt_add_resources(kritaresources_LIB_SRCS sql.qrc)

kis_add_library(kritaresources SHARED ${kritaresources_LIB_SRCS})

generate_export_header(kritaresources BASE_NAME kritaresources)

target_link_libraries(kritaresources
    PUBLIC
        Qt${QT_MAJOR_VERSION}::Core
        Qt${QT_MAJOR_VERSION}::Widgets
        Qt${QT_MAJOR_VERSION}::Sql
        Boost::boost
    PRIVATE
        kritaversion
        kritaglobal
        kritaplugin
        kritastore
        KF${KF_MAJOR}::ConfigCore
        KF${KF_MAJOR}::CoreAddons
        KF${KF_MAJOR}::I18n
        ${QUAZIP_LIBRARIES}
)

set_target_properties(kritaresources PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)
install(TARGETS kritaresources ${INSTALL_TARGETS_DEFAULT_ARGS} )

