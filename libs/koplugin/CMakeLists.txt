
set(kritaplugin_LIB_SRCS
    KoPluginLoader.cpp
    KoJsonTrader.cpp
)

# help plugin trader find installed plugins when run from uninstalled tests
if (BUILD_TESTING)
    add_definitions(-DCMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")
endif()

add_library(kritaplugin SHARED ${kritaplugin_LIB_SRCS})
generate_export_header(kritaplugin BASE_NAME kritaplugin)

target_link_libraries(kritaplugin
    PUBLIC
        Qt5::Core
    PRIVATE
        KF5::ConfigCore
        KF5::CoreAddons
)

set_target_properties(kritaplugin PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS kritaplugin ${INSTALL_TARGETS_DEFAULT_ARGS} )

