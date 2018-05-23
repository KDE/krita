
set(kritaresources_LIB_SRCS
    KisResourceCacheDb.cpp
)

qt5_add_resources(kritawidgetutils_LIB_SRCS sql.qrc)


# help plugin trader find installed plugins when run from uninstalled tests
if (BUILD_TESTING)
    add_definitions(-DCMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")
endif()

add_library(kritaresources SHARED ${kritaresources_LIB_SRCS})
generate_export_header(kritaresources BASE_NAME kritaresources)

target_link_libraries(kritaresources
    PUBLIC
        Qt5::Core
    PRIVATE
        Qt5::Sql
        KF5::ConfigCore
        KF5::CoreAddons
        KF5::I18n
)

set_target_properties(kritaresources PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)
install(TARGETS kritaresources ${INSTALL_TARGETS_DEFAULT_ARGS} )

