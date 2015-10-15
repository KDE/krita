project(calligradockers)

add_definitions(-DTRANSLATION_DOMAIN=\"krita-dockers\")

include_directories(
     ${FLAKE_INCLUDES}
     ${KOWIDGETS_INCLUDES}     
)

set(calligradockers_SRCS

    snapguidedocker/SnapGuideDocker.cpp
    snapguidedocker/SnapGuideDockerFactory.cpp

    shapeproperties/ShapePropertiesDocker.cpp
    shapeproperties/ShapePropertiesDockerFactory.cpp

    shapecollection/CollectionShapeFactory.cpp
    shapecollection/ShapeCollectionDocker.cpp
    shapecollection/CollectionItemModel.cpp
    shapecollection/OdfCollectionLoader.cpp

    Plugin.cpp
)

add_library(krita_docker_defaults MODULE ${calligradockers_SRCS})
kcoreaddons_desktop_to_json(krita_docker_defaults calligra_docker_defaults.desktop)

target_link_libraries(krita_docker_defaults kritawidgets)

install(TARGETS krita_docker_defaults DESTINATION ${CALLIGRA_PLUGIN_INSTALL_DIR})
