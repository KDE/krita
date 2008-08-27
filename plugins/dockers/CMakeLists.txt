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

    shapeselector/Canvas.cpp
    shapeselector/DragCanvasStrategy.cpp
    shapeselector/FolderBorder.cpp
    shapeselector/FolderShape.cpp
    shapeselector/FolderShapeModel.cpp
    shapeselector/GroupShape.cpp
    shapeselector/IconShape.cpp
    shapeselector/ItemStore.cpp
    shapeselector/MoveFolderStrategy.cpp
    shapeselector/RightClickStrategy.cpp
    shapeselector/SelectStrategy.cpp
    shapeselector/ShapeSelector.cpp
    shapeselector/ShapeSelectorFactory.cpp
    shapeselector/TemplateShape.cpp
    shapeselector/ZoomHandler.cpp
    shapeselector/ClipboardProxyShape.cpp
    shapeselector/ResizeFolderStrategy.cpp

    Plugin.cpp
)

add_subdirectory( strokedocker/pics )

#kde4_add_ui_files( kofficedockers_SRCS
#)

kde4_add_plugin(kofficedockers ${kofficedockers_SRCS})

target_link_libraries(
    kofficedockers
    koguiutils
    komain
    flake
    kstore
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KIO_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QT3SUPPORT_LIBRARY}
    )

install(TARGETS kofficedockers DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES kofficedockers.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
