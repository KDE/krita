project( defaulttools )

set ( defaulttools_SRCS
    Plugin.cpp
    defaulttool/DefaultTool.cpp
    defaulttool/DefaultToolFactory.cpp
    defaulttool/DefaultToolTabbedWidget.cpp
    defaulttool/DefaultToolGeometryWidget.cpp
    defaulttool/ShapeMoveStrategy.cpp
    defaulttool/ShapeResizeStrategy.cpp
    defaulttool/ShapeRotateStrategy.cpp
    defaulttool/ShapeShearStrategy.cpp
    defaulttool/ShapeGradientEditStrategy.cpp
    defaulttool/SelectionDecorator.cpp
    defaulttool/ShapeMeshGradientEditStrategy.cpp

    defaulttool/KoShapeGradientHandles.cpp
    defaulttool/KoShapeMeshGradientHandles.cpp

    referenceimagestool/ToolReferenceImages.cpp
    referenceimagestool/ToolReferenceImagesWidget.cpp
    referenceimagestool/KisReferenceImageCollection.cpp
    referenceimagestool/kisreferenceimagecropdecorator.cpp
    referenceimagestool/KisReferenceImageCropStrategy.cpp
)

ki18n_wrap_ui(defaulttools_SRCS
    defaulttool/DefaultToolGeometryWidget.ui
    referenceimagestool/WdgToolOptions.ui
)

qt5_add_resources(defaulttools_SRCS defaulttools.qrc)

add_library(krita_flaketools MODULE ${defaulttools_SRCS})
target_link_libraries(krita_flaketools kritaflake kritawidgets kritaui)
install(TARGETS krita_flaketools DESTINATION ${KRITA_PLUGIN_INSTALL_DIR})
