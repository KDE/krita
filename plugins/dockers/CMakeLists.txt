project(calligradockers)

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

add_library(calligra_docker_defaults MODULE ${calligradockers_SRCS})
kcoreaddons_desktop_to_json(calligra_docker_defaults calligra_docker_defaults.desktop)

target_link_libraries(calligra_docker_defaults kowidgets)

install(TARGETS calligra_docker_defaults DESTINATION ${PLUGIN_INSTALL_DIR})
