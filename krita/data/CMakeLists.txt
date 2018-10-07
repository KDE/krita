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
add_subdirectory( symbols )
add_subdirectory( preset_icons )
add_subdirectory( metadata/schemas )
add_subdirectory( gamutmasks )

########### install files ###############

install( FILES
    kritarc
    DESTINATION ${CONFIG_INSTALL_DIR} 
)

