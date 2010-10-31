project(kofficedockers)

include_directories(${KDE4_INCLUDE_DIR}
     ${FLAKE_INCLUDES}
     ${KOPAGEAPP_INCLUDES}
     ${CMAKE_SOURCE_DIR}/libs/widgets/colorwidgets
)

set(kofficedockers_SRCS

    strokedocker/StrokeDocker.cpp
    strokedocker/StrokeDockerFactory.cpp

    shapeproperties/ShapePropertiesDocker.cpp
    shapeproperties/ShapePropertiesDockerFactory.cpp

    shapecollection/CollectionShapeFactory.cpp
    shapecollection/ShapeCollectionDocker.cpp
    shapecollection/CollectionItemModel.cpp
    shapecollection/OdfCollectionLoader.cpp

    shadowdocker/ShadowDocker.cpp
    shadowdocker/ShadowDockerFactory.cpp

    styledocker/StyleButtonBox.cpp
    styledocker/StylePreview.cpp
    styledocker/StyleDocker.cpp
    styledocker/StyleDockerFactory.cpp

    Plugin.cpp
)

add_subdirectory( strokedocker/pics )

set(kofficedockers_SRCS ${kofficedockers_SRCS}
    colordocker/ColorDocker.h
    colordocker/ColorDockerFactory.h
    colordocker/ColorDocker.cpp
    colordocker/ColorDockerFactory.cpp
)
set(EXTRA_LIBS pigmentcms kowidgets)


kde4_add_plugin(kofficedockers ${kofficedockers_SRCS})

target_link_libraries(kofficedockers komain ${EXTRA_LIBS})

install(TARGETS kofficedockers DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES kofficedockers.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
