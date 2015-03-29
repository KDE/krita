include_directories( ${KOMAIN_INCLUDES} )

########### Flake Plugin library ###############

set (pluginshape_LIB_SRCS
    PluginShape.cpp
    PluginShapeFactory.cpp
    Plugin.cpp
)

add_library(calligra_shape_plugin MODULE ${pluginshape_LIB_SRCS})
kcoreaddons_desktop_to_json(calligra_shape_plugin calligra_shape_plugin.desktop)

target_link_libraries(calligra_shape_plugin komain
  )
install(TARGETS calligra_shape_plugin DESTINATION ${PLUGIN_INSTALL_DIR})
