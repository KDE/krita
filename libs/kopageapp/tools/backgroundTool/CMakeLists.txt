include_directories( ../.. )

set(kopabackgroundtool_PART_SRCS
    Plugin.cpp
    KoPABackgroundToolFactory.cpp
    KoPAMasterPageDocker.cpp
)


add_library(kopabackgroundtool MODULE ${kopabackgroundtool_PART_SRCS})
kcoreaddons_desktop_to_json(kopabackgroundtool kopabackgroundtool.desktop)

target_link_libraries(
    kopabackgroundtool
    kopageapp
)

install(TARGETS kopabackgroundtool  DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})


########### install files ###############

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
