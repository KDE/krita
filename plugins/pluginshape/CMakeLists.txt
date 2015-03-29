include_directories( ${KOMAIN_INCLUDES} )

########### Flake Plugin library ###############

set (pluginshape_LIB_SRCS
    PluginShape.cpp
    PluginShapeFactory.cpp
    Plugin.cpp
)

add_library(calligra_shape_plugin MODULE ${pluginshape_LIB_SRCS})

target_link_libraries(calligra_shape_plugin komain
  )
install(TARGETS calligra_shape_plugin DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES calligra_shape_plugin.desktop DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
