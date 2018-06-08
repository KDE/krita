add_subdirectory(tests)

set(kritaresources_LIB_SRCS
    KisResourceCacheDb.cpp
    KisResourceLoader.cpp
    KisResourceLoaderRegistry.cpp
    KisResourceLocator.cpp
    KisResourceStorage.cpp

    KoResource.cpp
    KoMD5Generator.cpp
    KoHashGeneratorProvider.cpp
    KoResourcePaths.cpp
)

qt5_add_resources(kritaresources_LIB_SRCS sql.qrc)

add_library(kritaresources SHARED ${kritaresources_LIB_SRCS})

generate_export_header(kritaresources BASE_NAME kritaresources)

target_link_libraries(kritaresources
    PUBLIC
        Qt5::Core
        Qt5::Widgets
    PRIVATE
        kritaversion
        kritaglobal
        Qt5::Sql
        KF5::ConfigCore
        KF5::CoreAddons
        KF5::I18n
        KF5::Archive
)

set_target_properties(kritaresources PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)
install(TARGETS kritaresources ${INSTALL_TARGETS_DEFAULT_ARGS} )

