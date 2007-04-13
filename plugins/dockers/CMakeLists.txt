project(kofficedockers)

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${KOFFICEUI_INCLUDES} )

set(kofficedockers_SRCS
    shapeselector/ShapeSelector.cpp
    shapeselector/ShapeSelectorFactory.cpp

    Plugin.cpp
)

kde4_automoc(${kofficedockers_SRCS})

#kde4_add_ui_files( kofficedockers_SRCS
#)

kde4_add_plugin(kofficedockers ${kofficedockers_SRCS})

target_link_libraries(kofficedockers kofficeui kofficecore)

install(TARGETS kofficedockers DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES kofficedockers.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
