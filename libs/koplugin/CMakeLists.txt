
set(koplugin_LIB_SRCS
	KoPluginLoader.cpp
    KoServiceLocator.cpp
	KoJsonTrader.cpp
)

add_library(koplugin SHARED ${koplugin_LIB_SRCS})
generate_export_header(koplugin BASE_NAME koplugin)

target_link_libraries(koplugin koversion KF5::KDELibs4Support Qt5::Core)
target_link_libraries(koplugin LINK_INTERFACE_LIBRARIES koversion KF5::KDELibs4Support Qt5::Core)

set_target_properties(koplugin PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS koplugin ${INSTALL_TARGETS_DEFAULT_ARGS} )

