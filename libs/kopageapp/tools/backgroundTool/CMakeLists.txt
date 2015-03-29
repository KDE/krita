include_directories( ../.. )

set(kopabackgroundtool_PART_SRCS
    Plugin.cpp
    KoPABackgroundToolFactory.cpp
    KoPAMasterPageDocker.cpp
)


add_library(kopabackgroundtool MODULE ${kopabackgroundtool_PART_SRCS})

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

ecm_install_icons( ICONS
    22-actions-backgroundtool.png
    DESTINATION ${DATA_INSTALL_DIR}/calligra/icons
    THEME oxygen
)
