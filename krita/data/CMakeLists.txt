add_subdirectory( brushes )
add_subdirectory( patterns )
add_subdirectory( gradients )
add_subdirectory( profiles )
add_subdirectory( palettes )
add_subdirectory( templates )
add_subdirectory( shaders )
add_subdirectory( images )
########### install files ###############

install( FILES  
    krita_filter.desktop      
    krita_paintop.desktop      
    krita_plugin.desktop      
    krita_tool.desktop
    krita_generator.desktop
    DESTINATION ${SERVICETYPES_INSTALL_DIR})



