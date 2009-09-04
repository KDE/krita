include_directories( ${KOMAIN_INCLUDES} ${FLAKE_INCLUDES}  )

add_subdirectory(pics)

set(pathshapes_PART_SRCS
    ellipse/KoEllipseShape.cpp
    ellipse/KoEllipseShapeFactory.cpp
    ellipse/EllipseShapeConfigWidget.cpp
    ellipse/EllipseShapeConfigCommand.cpp
    spiral/KoSpiralShape.cpp
    spiral/KoSpiralShapeFactory.cpp
    spiral/SpiralShapeConfigWidget.cpp
    spiral/SpiralShapeConfigCommand.cpp
    star/KoStarShape.cpp
    star/KoStarShapeFactory.cpp
    star/StarShapeConfigWidget.cpp
    star/StarShapeConfigCommand.cpp
    rectangle/KoRectangleShape.cpp
    rectangle/KoRectangleShapeFactory.cpp
    rectangle/RectangleShapeConfigWidget.cpp
    rectangle/RectangleShapeConfigCommand.cpp
    arrow/KoArrowShape.cpp
    arrow/KoArrowShapeFactory.cpp
    enhancedpath/KoEnhancedPathShape.cpp
    enhancedpath/KoEnhancedPathShapeFactory.cpp
    enhancedpath/KoEnhancedPathCommand.cpp
    enhancedpath/KoEnhancedPathParameter.cpp
    enhancedpath/KoEnhancedPathFormula.cpp
    enhancedpath/KoEnhancedPathHandle.cpp
    PathShapesPlugin.cpp
)

kde4_add_ui_files(pathshapes_PART_SRCS
    star/StarShapeConfigWidget.ui
    rectangle/RectangleShapeConfigWidget.ui
    ellipse/EllipseShapeConfigWidget.ui
    spiral/SpiralShapeConfigWidget.ui
)
kde4_add_plugin(pathshapes ${pathshapes_PART_SRCS})

target_link_libraries(pathshapes flake kowidgets)

install(TARGETS pathshapes  DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES pathshapes.desktop DESTINATION ${SERVICES_INSTALL_DIR} )
