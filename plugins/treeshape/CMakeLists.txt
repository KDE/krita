include_directories( ${FLAKE_INCLUDES} )

########### Flake Plugin library ###############

SET (treeshape_LIB_SRCS
    TreeLayout.cpp
    TreeShape.cpp

    TreeShapeConfigWidget.cpp
    TreeChangeStructureCommand.cpp
    TreeChangeConnectionCommand.cpp
    TreeShapeMoveStrategy.cpp
    TreeShapeMoveCommand.cpp
    TreeShapeFactory.cpp

    SelectionDecorator.cpp
    TreeTool.cpp
    TreeToolFactory.cpp
    Plugin.cpp
)

kde4_add_ui_files(treeshape_LIB_SRCS
    TreeShapeConfigWidget.ui
)
kde4_add_plugin(treeshape ${treeshape_LIB_SRCS})

target_link_libraries(treeshape flake)

install(TARGETS treeshape DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES treeshape.desktop DESTINATION ${SERVICES_INSTALL_DIR})
