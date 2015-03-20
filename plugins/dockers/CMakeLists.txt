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

target_link_libraries(calligra_docker_defaults kowidgets)

install(TARGETS calligra_docker_defaults DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES calligra_docker_defaults.desktop DESTINATION ${SERVICES_INSTALL_DIR}/calligra )
