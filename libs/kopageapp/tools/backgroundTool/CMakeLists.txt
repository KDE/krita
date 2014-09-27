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
)

install(TARGETS kopabackgroundtool  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES  kopabackgroundtool.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/calligra)

install(FILES
    KoPABackgroundTool.h
    DESTINATION
    ${INCLUDE_INSTALL_DIR}/calligra COMPONENT Devel
)

kde4_install_icons( ${DATA_INSTALL_DIR}/calligra/icons  )
