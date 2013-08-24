include_directories( ${KOMAIN_INCLUDES} ${FLAKE_INCLUDES} )

add_subdirectory(pics)

set(pathshapes_PART_SRCS
    ellipse/EllipseShape.cpp
    ellipse/EllipseShapeFactory.cpp
    ellipse/EllipseShapeConfigWidget.cpp
    ellipse/EllipseShapeConfigCommand.cpp
    spiral/SpiralShape.cpp
    spiral/SpiralShapeFactory.cpp
    spiral/SpiralShapeConfigWidget.cpp
    spiral/SpiralShapeConfigCommand.cpp
    star/StarShape.cpp
    star/StarShapeFactory.cpp
    star/StarShapeConfigWidget.cpp
    star/StarShapeConfigCommand.cpp
    rectangle/RectangleShape.cpp
    rectangle/RectangleShapeFactory.cpp
    rectangle/RectangleShapeConfigWidget.cpp
    rectangle/RectangleShapeConfigCommand.cpp
    enhancedpath/EnhancedPathShape.cpp
    enhancedpath/EnhancedPathShapeFactory.cpp
    enhancedpath/EnhancedPathCommand.cpp
    enhancedpath/EnhancedPathParameter.cpp
    enhancedpath/EnhancedPathFormula.cpp
    enhancedpath/EnhancedPathHandle.cpp
    PathShapesPlugin.cpp
)

kde4_add_ui_files(pathshapes_PART_SRCS
    star/StarShapeConfigWidget.ui
    rectangle/RectangleShapeConfigWidget.ui
    ellipse/EllipseShapeConfigWidget.ui
    spiral/SpiralShapeConfigWidget.ui
)
kde4_add_plugin(calligra_shape_paths ${pathshapes_PART_SRCS})

target_link_libraries(calligra_shape_paths flake kowidgets)

install(TARGETS calligra_shape_paths  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES calligra_shape_paths.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
