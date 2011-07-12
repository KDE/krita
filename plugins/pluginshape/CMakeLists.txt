include_directories( ${KOMAIN_INCLUDES} )

########### Flake Plugin library ###############

SET (pluginshape_LIB_SRCS
    PluginShape.cpp
    PluginShapeFactory.cpp
    Plugin.cpp
)

kde4_add_plugin(pluginshape ${pluginshape_LIB_SRCS})

target_link_libraries(pluginshape komain
  )
install(TARGETS pluginshape DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES pluginshape.desktop DESTINATION ${SERVICES_INSTALL_DIR})
kde4_install_icons( ${DATA_INSTALL_DIR}/calligra/icons )
