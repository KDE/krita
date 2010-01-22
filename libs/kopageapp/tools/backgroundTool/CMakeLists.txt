include_directories( ../.. )

set(kopabackgroundtool_PART_SRCS
    Plugin.cpp
    KoPABackgroundTool.cpp
    KoPABackgroundToolFactory.cpp
    KoPABackgroundToolWidget.cpp
    KoPAMasterPageDocker.cpp
)


kde4_add_ui_files(kopabackgroundtool_PART_SRCS
    BackgroundToolWidget.ui
)


kde4_add_plugin(kopabackgroundtool ${kopabackgroundtool_PART_SRCS})

target_link_libraries(
    kopabackgroundtool
    kopageapp
    ${KDE4_KIO_LIBRARY}
    ${KDE4_PHONON_LIBRARY}
)

install(TARGETS kopabackgroundtool  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kopabackgroundtool.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

kde4_install_icons( ${DATA_INSTALL_DIR}/koffice/icons  )
