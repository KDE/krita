project(calligradockers)

include_directories(${KDE4_INCLUDE_DIR}
     ${FLAKE_INCLUDES}
     ${KOPAGEAPP_INCLUDES}
)

set(calligradockers_SRCS

    strokedocker/StrokeDocker.cpp
    strokedocker/StrokeDockerFactory.cpp

    snapguidedocker/SnapGuideDocker.cpp
    snapguidedocker/SnapGuideDockerFactory.cpp

    shapeproperties/ShapePropertiesDocker.cpp
    shapeproperties/ShapePropertiesDockerFactory.cpp

    shapecollection/CollectionShapeFactory.cpp
    shapecollection/ShapeCollectionDocker.cpp
    shapecollection/CollectionItemModel.cpp
    shapecollection/OdfCollectionLoader.cpp

    shadowdocker/ShadowDocker.cpp
    shadowdocker/ShadowDockerFactory.cpp

    styledocker/StyleDockerFactory.cpp
    styledocker/StyleDocker.cpp
    styledocker/StrokeFillWidget.cpp
    styledocker/StylePreview.cpp
    styledocker/StyleButtonBox.cpp

    Plugin.cpp
)

add_subdirectory( strokedocker/pics )

set(calligradockers_SRCS ${calligradockers_SRCS}
    colordocker/ColorDocker.h
    colordocker/ColorDockerFactory.h
    colordocker/ColorDocker.cpp
    colordocker/ColorDockerFactory.cpp
)
set(EXTRA_LIBS pigmentcms kowidgets)


kde4_add_plugin(calligradockers ${calligradockers_SRCS})

target_link_libraries(calligradockers komain ${EXTRA_LIBS})

install(TARGETS calligradockers DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES calligradockers.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
