include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${KOFFICECORE_INCLUDES})

add_subdirectory(tests)

set(flake_SRCS
    KoCommand.cpp
    KoInteractionStrategy.cpp
    KoInteractionTool.cpp
    KoLineBorder.cpp
    KoPathShape.cpp
    KoRectangleShape.cpp
    KoSelection.cpp
    KoShape.cpp
    KoShapeContainer.cpp
    KoShapeGroup.cpp
    KoShapeManager.cpp
    KoShapeMoveStrategy.cpp
    KoShapeResizeStrategy.cpp
    KoShapeRotateStrategy.cpp
    KoShapeShearStrategy.cpp
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
    KoShapeUserData.cpp
)

kde4_automoc(${flake_SRCS})

kde4_add_library(flake SHARED ${flake_SRCS})

target_link_libraries(flake ${KDE4_KDECORE_LIBS} ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KDEUI_LIBS} kofficecore)

set_target_properties(flake PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )

########### install files ###############

install(TARGETS flake  DESTINATION ${LIB_INSTALL_DIR})
install( FILES flaketool.desktop flakeshape.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

kde4_install_icons( ${DATA_INSTALL_DIR}/koffice/icons crystalsvg)

install( FILES rotate.png shear.png DESTINATION ${DATA_INSTALL_DIR}/koffice/icons)

