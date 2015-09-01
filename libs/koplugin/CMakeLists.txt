
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

target_link_libraries(koplugin KF5::KDELibs4Support KF5::Service Qt5::Core)
target_link_libraries(koplugin LINK_INTERFACE_LIBRARIES KF5::Service Qt5::Core)

set_target_properties(koplugin PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS koplugin ${INSTALL_TARGETS_DEFAULT_ARGS} )

