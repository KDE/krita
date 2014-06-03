include_directories( ${FLAKE_INCLUDES} )

########### Flake Plugin library ###############

SET (threedshape_LIB_SRCS
    ThreedShapePlugin.cpp
    ThreedShapeFactory.cpp
    #ThreedToolFactory.cpp
    #ThreedTool.cpp
    #ThreedShapeConfigWidget.cpp

    Object3D.cpp
    SceneObject.cpp
    Objects.cpp
    utils.cpp
)

kde4_add_plugin(calligra_shape_threed ${threedshape_LIB_SRCS})

target_link_libraries(calligra_shape_threed flake    ${KDE4_KFILE_LIBRARY})
#target_link_libraries(calligra_shape_threed komain flake    ${KDE4_KFILE_LIBRARY}
#)
install(TARGETS calligra_shape_threed DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES calligra_shape_threed.desktop DESTINATION ${SERVICES_INSTALL_DIR})
kde4_install_icons( ${DATA_INSTALL_DIR}/calligra/icons )
