include_directories( ${KOGUIUTILS_INCLUDES}  ${KOTEXT_INCLUDES} )

add_subdirectory( tests )

########### Flake Plugin library ###############

SET (tableshape_LIB_SRCS
    TableShape.cpp
    TableShapeFactory.cpp
    TableTool.cpp
    TableToolFactory.cpp
    Plugin.cpp
    TableCell.cpp
    TableRow.cpp
    Table.cpp
    TableColumn.cpp
)

kde4_add_plugin(tableshape ${tableshape_LIB_SRCS})

target_link_libraries(tableshape flake koguiutils kotext)
install(TARGETS tableshape DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES tableshape.desktop DESTINATION ${SERVICES_INSTALL_DIR})
kde4_install_icons( ${DATA_INSTALL_DIR}/koffice/icons )
