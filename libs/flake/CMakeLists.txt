include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} )

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
)


kde4_automoc(${flake_SRCS})

kde4_add_library(flake ${flake_SRCS})

kde4_install_libtool_file( ${LIB_INSTALL_DIR} flake )

install_targets(${LIB_INSTALL_DIR} flake )


