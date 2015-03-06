include_directories( ${FLAKE_INCLUDES} )

########### Flake Plugin library ###############

set (threedshape_LIB_SRCS
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

add_library(calligra_shape_threed MODULE ${threedshape_LIB_SRCS})

target_link_libraries(calligra_shape_threed flake    ${KDE4_KFILE_LIBRARY})
install(TARGETS calligra_shape_threed DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES calligra_shape_threed.desktop DESTINATION ${SERVICES_INSTALL_DIR}/calligra)
ecm_install_icons( ${DATA_INSTALL_DIR}/calligra/icons )
