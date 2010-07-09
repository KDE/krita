include_directories(  )

########### Flake Plugin library ###############

SET (treeshape_LIB_SRCS
    Layout.cpp
    Plugin.cpp
    SelectionDecorator.cpp
    TreeShape.cpp
    TreeShapeFactory.cpp
    TreeShapeMoveCommand.cpp
    TreeShapeMoveStrategy.cpp
    TreeTool.cpp
    TreeToolFactory.cpp
)

kde4_add_plugin(treeshape ${treeshape_LIB_SRCS})

target_link_libraries(treeshape komain)
install(TARGETS treeshape DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES treeshape.desktop DESTINATION ${SERVICES_INSTALL_DIR})

