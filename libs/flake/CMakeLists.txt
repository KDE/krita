include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )

add_subdirectory(tests)

set(flake_SRCS
    KoCommand.cpp
    KoInteractionStrategy.cpp
    KoInteractionTool.cpp
    KoLineBorder.cpp
    KoPathShape.cpp
    KoRectangleShape.cpp
    KoRepaintManager.cpp
    KoSelection.cpp
    KoShape.cpp
    KoShapeContainer.cpp
    KoShapeGroup.cpp
    KoShapeManager.cpp
    KoShapeMoveStrategy.cpp
    KoShapeResizeStrategy.cpp
    KoShapeRotateStrategy.cpp
    KoShapeRubberSelectStrategy.cpp
    KoTool.cpp
    KoCanvasView.cpp
)

kde4_automoc(${flake_SRCS})

kde4_add_library(flake SHARED ${flake_SRCS})

target_link_libraries(flake ${KDE4_KDEUI_LIBS})

set_target_properties(flake PROPERTIES VERSION 1.0.0 SOVERSION 1 )


install_targets(${LIB_INSTALL_DIR} flake )


