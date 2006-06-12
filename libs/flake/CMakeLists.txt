include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${KOFFICECORE_INCLUDES})

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
    KoCanvasController.cpp
    KoCreateShapeStrategy.cpp
    KoCreateShapesTool.cpp
    KoShapeRegistry.cpp
    KoToolFactory.cpp
    KoCreateShapesToolFactory.cpp
    KoInteractionToolFactory.cpp
    KoRectangleShapeFactory.cpp
    KoPathShapeFactory.cpp
    KoShapeFactory.cpp
)

kde4_automoc(${flake_SRCS})

kde4_add_library(flake SHARED ${flake_SRCS})

target_link_libraries(flake ${KDE4_KDECORE_LIBS} ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KDEUI_LIBS})

set_target_properties(flake PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )


install_targets(${LIB_INSTALL_DIR} flake )
install_files( ${SERVICETYPES_INSTALL_DIR} FILES flaketool.desktop flakeshape.desktop)

