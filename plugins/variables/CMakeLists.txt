project(textvariables)

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${KOTEXT_INCLUDES} ${KOFFICECORE_INCLUDES} )

set(textvariables_PART_SRCS
    KoVariablesPlugin.cpp
    KoDateVariable.cpp
    KoDateVariableFactory.cpp
    FixedDateFormat.cpp
)

kde4_automoc(${textvariables_PART_SRCS})

kde4_add_ui_files( ${textvariables_PART_SRCS}
    FixedDateFormat.ui
)

kde4_add_plugin(textvariables ${textvariables_PART_SRCS})

target_link_libraries(textvariables kotext)

install(TARGETS textvariables DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES textvariables.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
