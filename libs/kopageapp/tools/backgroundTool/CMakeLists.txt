include_directories( ../.. )

set(kopabackgroundtool_PART_SRCS
    Plugin.cpp
    KoPABackgroundToolFactory.cpp
    KoPAMasterPageDocker.cpp
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

install(FILES
    KoPABackgroundTool.h
    DESTINATION
    ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)

kde4_install_icons( ${DATA_INSTALL_DIR}/calligra/icons  )
