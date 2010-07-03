include_directories(  )

########### Flake Plugin library ###############

SET (treeshape_LIB_SRCS
    TreeShape.cpp
    TreeTool.cpp
    TreeShapeFactory.cpp
    TreeToolFactory.cpp
    Layout.cpp
    Plugin.cpp
)

kde4_add_plugin(treeshape ${treeshape_LIB_SRCS})

target_link_libraries(treeshape komain)
install(TARGETS treeshape DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES treeshape.desktop DESTINATION ${SERVICES_INSTALL_DIR})

