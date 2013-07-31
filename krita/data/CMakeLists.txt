add_subdirectory( brushes )
add_subdirectory( paintoppresets )
add_subdirectory( patterns )
add_subdirectory( gradients )
add_subdirectory( profiles )
add_subdirectory( templates )
add_subdirectory( shaders )
add_subdirectory( cursors )
add_subdirectory( workspaces )
add_subdirectory( themes )
add_subdirectory( predefined_image_sizes )
add_subdirectory( input )
########### install files ###############

install( FILES  
    krita_filter.desktop      
    krita_brush.desktop      
    krita_paintop.desktop      
    krita_plugin.desktop      
    krita_tool.desktop
    krita_generator.desktop
    krita_dock.desktop
    krita_animationdock.desktop
    DESTINATION ${SERVICETYPES_INSTALL_DIR})

install( FILES
    kritarc
    kritapresets.knsrc
    kritabrushes.knsrc
    kritapatterns.knsrc
    kritagradients.knsrc
    kritaiccprofiles.knsrc
    DESTINATION ${CONFIG_INSTALL_DIR} )

install( FILES
    krita.xml
    kritaanimation.xml
    DESTINATION ${XDG_MIME_INSTALL_DIR}
)
update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})
