add_subdirectory( actions )
add_subdirectory( brushes )
add_subdirectory( bundles )
add_subdirectory( patterns )
add_subdirectory( gradients )
add_subdirectory( profiles )
add_subdirectory( templates )
add_subdirectory( workspaces )
add_subdirectory( themes )
add_subdirectory( predefined_image_sizes )
add_subdirectory( input )
add_subdirectory( shortcuts )
add_subdirectory( paintoppresets )
add_subdirectory( palettes )

########### install files ###############

install( FILES
    kritarc
    DESTINATION ${CONFIG_INSTALL_DIR} 
)

install( FILES
    krita.xml
    x-gimp-brush-animated.xml
    x-gimp-brush.xml
    DESTINATION ${XDG_MIME_INSTALL_DIR}
)
if (SHARED_MIME_INFO_FOUND)
    update_xdg_mimetypes(${XDG_MIME_INSTALL_DIR})
endif()
