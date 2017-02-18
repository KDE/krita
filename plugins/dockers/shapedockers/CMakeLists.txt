project(calligradockers)

set(calligradockers_SRCS
    shapecollection/CollectionShapeFactory.cpp
    shapecollection/ShapeCollectionDocker.cpp
    shapecollection/CollectionItemModel.cpp
    shapecollection/OdfCollectionLoader.cpp

    Plugin.cpp
)

add_library(krita_docker_defaults MODULE ${calligradockers_SRCS})

target_link_libraries(krita_docker_defaults kritawidgets)

install(TARGETS krita_docker_defaults DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
