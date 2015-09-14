add_definitions(-DTRANSLATION_DOMAIN=\"calligra_shape_threed\")

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
kcoreaddons_desktop_to_json(calligra_shape_threed calligra_shape_threed.desktop)

target_link_libraries(calligra_shape_threed flake KF5::KDELibs4Support)
install(TARGETS calligra_shape_threed DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
