add_definitions(-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

include_directories( ${KDE4_INCLUDES} )

set(koplugin_LIB_SRCS
	KoPluginLoader.cpp
)

kde4_add_library(koplugin SHARED ${koplugin_LIB_SRCS})

target_link_libraries(koplugin ${KDE4_KDECORE_LIBS})
target_link_libraries(koplugin LINK_INTERFACE_LIBRARIES ${KDE4_KDECORE_LIBS})

set_target_properties(koplugin PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)
install(TARGETS koplugin ${INSTALL_TARGETS_DEFAULT_ARGS} )

