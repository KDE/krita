project(kofficedockers)

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${KOGUIUTILS_INCLUDES} )

set(kofficedockers_SRCS

    strokedocker/StrokeDocker.cpp
    strokedocker/StrokeDockerFactory.cpp

    shapeproperties/ShapePropertiesDocker.cpp
    shapeproperties/ShapePropertiesDockerFactory.cpp

    shapecollection/KoCollectionShapeFactory.cpp
    shapecollection/KoShapeCollectionDocker.cpp
    shapecollection/KoCollectionItemModel.cpp
    shapecollection/KoOdfCollectionLoader.cpp

    shadowdocker/ShadowDocker.cpp
    shadowdocker/ShadowDockerFactory.cpp

    Plugin.cpp
)

add_subdirectory( strokedocker/pics )

#kde4_add_ui_files( kofficedockers_SRCS
#)

kde4_add_plugin(kofficedockers ${kofficedockers_SRCS})

target_link_libraries(kofficedockers koguiutils komain)

install(TARGETS kofficedockers DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES kofficedockers.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
