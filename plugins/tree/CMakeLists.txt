include_directories(  )

########### Flake Plugin library ###############

SET (tree_LIB_SRCS
    Tree.cpp
    TreeTool.cpp
    TreeFactory.cpp
    TreeToolFactory.cpp
    Layout.cpp
    Plugin.cpp
)

kde4_add_plugin(tree ${tree_LIB_SRCS})

target_link_libraries(tree komain)
install(TARGETS tree DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES tree.desktop DESTINATION ${SERVICES_INSTALL_DIR})

