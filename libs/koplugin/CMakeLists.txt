
set(koplugin_LIB_SRCS
    KoPluginLoader.cpp
    KoJsonTrader.cpp
)

# help plugin trader find installed plugins when run from uninstalled tests
if (KDE4_BUILD_TESTS)
    add_definitions(-DCMAKE_INSTALL_PREFIX="${CMAKE_INSTALL_PREFIX}")
endif()

add_library(koplugin SHARED ${koplugin_LIB_SRCS})
generate_export_header(koplugin BASE_NAME koplugin)

target_link_libraries(koplugin
    PUBLIC
        Qt5::Core
    PRIVATE
        KF5::ConfigCore
        KF5::CoreAddons
)

set_target_properties(koplugin PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS koplugin ${INSTALL_TARGETS_DEFAULT_ARGS} )

