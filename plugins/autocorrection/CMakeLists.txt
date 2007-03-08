project(autocorrection)

add_subdirectory( data )

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${KOTEXT_INCLUDES} ${KOFFICECORE_INCLUDES} )

set(autocorrect_SRCS
    AutocorrectPlugin.cpp
    Autocorrect.cpp
    AutocorrectFactory.cpp
)

kde4_automoc(${autocorrect_SRCS})

#kde4_add_ui_files( autocorrect_SRCS
#)

kde4_add_plugin(autocorrect ${autocorrect_SRCS})

target_link_libraries(autocorrect kotext)

install(TARGETS autocorrect DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES autocorrect.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
