include_directories( ${KOGUIUTILS_INCLUDES} )

########### Flake Plugin library ###############

SET (tableshape_LIB_SRCS
    TableShape.cpp
    TableShapeFactory.cpp
    TableTool.cpp
    TableToolFactory.cpp
    Plugin.cpp
    TableCell.cpp
)

kde4_add_plugin(tableshape ${tableshape_LIB_SRCS})

target_link_libraries(tableshape flake koguiutils)
install(TARGETS tableshape DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES tableshape.desktop DESTINATION ${SERVICES_INSTALL_DIR})
kde4_install_icons( ${DATA_INSTALL_DIR}/koffice/icons )
