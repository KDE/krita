include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${KOFFICECORE_INCLUDES} ${PIGMENT_INCLUDES})

add_subdirectory(tests)

set(flake_SRCS
    KoCanvasBase.cpp
    KoCanvasResourceProvider.cpp
    KoInteractionStrategy.cpp
    KoInteractionTool.cpp
    KoLineBorder.cpp
    KoPathShape.cpp
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
    KoPathShapeFactory.cpp
    KoShapeFactory.cpp
    KoShapeUserData.cpp
    KoCreatePathTool.cpp
    KoCreatePathToolFactory.cpp
    KoPathToolFactory.cpp
    KoPathTool.cpp
    KoPathCommand.cpp
    KoPathPointRubberSelectStrategy.cpp
    KoPathPointMoveStrategy.cpp
    KoPathControlPointMoveStrategy.cpp
    KoParameterShape.cpp
    KoParameterChangeStrategy.cpp
    KoPointerEvent.cpp
    KoShapeController.cpp
    KoToolSelection.cpp
    KoShapeLayer.cpp
    commands/KoGroupShapesCommand.cpp
    commands/KoShapeAlignCommand.cpp
    commands/KoShapeBackgroundCommand.cpp
    commands/KoShapeBorderCommand.cpp
    commands/KoShapeCreateCommand.cpp
    commands/KoShapeDeleteCommand.cpp
    commands/KoShapeDistributeCommand.cpp
    commands/KoShapeLockCommand.cpp
    commands/KoShapeMoveCommand.cpp
    commands/KoShapeRotateCommand.cpp
    commands/KoShapeShearCommand.cpp
    commands/KoShapeSizeCommand.cpp
    commands/KoUngroupShapesCommand.cpp
    commands/KoShapeReorderCommand.cpp
    commands/KoShapeKeepAspectRatioCommand.cpp
)

kde4_automoc(${flake_SRCS})

kde4_add_library(flake SHARED ${flake_SRCS})

target_link_libraries(flake ${KDE4_KDEUI_LIBS} kofficecore pigment)

set_target_properties(flake PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )

########### install files ###############

install(TARGETS flake  DESTINATION ${LIB_INSTALL_DIR})
install( FILES flake.desktop flaketool.desktop flakeshape.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

kde4_install_icons( ${DATA_INSTALL_DIR}/koffice/icons )

install( FILES rotate.png shear.png DESTINATION ${DATA_INSTALL_DIR}/koffice/icons)

