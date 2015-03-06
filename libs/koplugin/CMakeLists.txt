include_directories( ${KDE4_INCLUDES} )

set(koplugin_LIB_SRCS
	KoPluginLoader.cpp
    KoServiceLocator.cpp
)

add_library(koplugin SHARED ${koplugin_LIB_SRCS})

target_link_libraries(koplugin koversion ${KDE4_KDEUI_LIBS})
target_link_libraries(koplugin LINK_INTERFACE_LIBRARIES koversion ${KDE4_KDEUI_LIBS})

set_target_properties(koplugin PROPERTIES
    VERSION ${GENERIC_CALLIGRA_LIB_VERSION} SOVERSION ${GENERIC_CALLIGRA_LIB_SOVERSION}
)
install(TARGETS koplugin ${INSTALL_TARGETS_DEFAULT_ARGS} )

