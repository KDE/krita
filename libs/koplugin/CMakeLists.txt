
set(kritaplugin_LIB_SRCS
    KoPluginLoader.cpp
    KoJsonTrader.cpp
    KisMimeDatabase.cpp
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
        kritaglobal
        KF5::ConfigCore
        KF5::CoreAddons
        KF5::I18n
)

set_target_properties(kritaplugin PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)
install(TARGETS kritaplugin ${INSTALL_TARGETS_DEFAULT_ARGS} )

