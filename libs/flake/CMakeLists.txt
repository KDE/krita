include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${KOMAIN_INCLUDES} ${PIGMENT_INCLUDES})

if(KDE4_BUILD_TESTS)
    # only with this definition will the FLAKE_TEST_EXPORT macro do something
    add_definitions(-DCOMPILING_TESTS)
endif(KDE4_BUILD_TESTS)


add_subdirectory(tests)
add_subdirectory(pics)

set(flake_SRCS
    KoCanvasBase.cpp
    KoCanvasResourceProvider.cpp
    KoInteractionStrategy.cpp
    KoInteractionTool.cpp
    KoPathShape.cpp
    KoPathPoint.cpp
    KoSelection.cpp
    KoShape.cpp
    KoShapeContainer.cpp
    KoShapeGroup.cpp
    KoShapeConnection.cpp
    KoShapeManager.cpp
    KoShapeMoveStrategy.cpp
    KoShapeResizeStrategy.cpp
    KoShapeRotateStrategy.cpp
    KoShapeShearStrategy.cpp
    KoShapeRubberSelectStrategy.cpp
    KoTool.cpp
    KoCanvasController.cpp
    KoCanvasController_p.cpp
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
    KoPathPointRubberSelectStrategy.cpp
    KoPathPointMoveStrategy.cpp
    KoPathControlPointMoveStrategy.cpp
    KoPointGroup.cpp
    KoParameterShape.cpp
    KoParameterChangeStrategy.cpp
    KoPointerEvent.cpp
    KoShapeController.cpp
    KoToolSelection.cpp
    KoShapeLayer.cpp
    KoPostscriptPaintDevice.cpp
    KoInputDevice.cpp
    KoToolManager_p.cpp
    KoToolManager.cpp
    KoToolRegistry.cpp
    KoToolProxy.cpp
    KoShapeSavingContext.cpp
    KoShapeLoadingContext.cpp
    KoPanTool.cpp
    KoPanToolFactory.cpp
    KoZoomTool.cpp
    KoZoomToolFactory.cpp
    KoZoomStrategy.cpp
    KoSvgPathParser.cpp
    KoPathShapeLoader.cpp
    ShapeDeleter_p.cpp
    KoShapeBorderModel.cpp
    KoShapeBorderFactory.cpp
    KoShapeBorderRegistry.cpp
    KoLineBorder.cpp
    KoLineBorderFactory.cpp
    KoShapeStyleWriter.cpp
    commands/KoShapeGroupCommand.cpp
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
    commands/KoShapeUngroupCommand.cpp
    commands/KoShapeReorderCommand.cpp
    commands/KoShapeKeepAspectRatioCommand.cpp
    commands/KoPathBaseCommand.cpp
    commands/KoPathPointMoveCommand.cpp
    commands/KoPathControlPointMoveCommand.cpp
    commands/KoPathPointTypeCommand.cpp
    commands/KoPathPointRemoveCommand.cpp
    commands/KoPathPointInsertCommand.cpp
    commands/KoPathSegmentBreakCommand.cpp
    commands/KoPathBreakAtPointCommand.cpp
    commands/KoPathSegmentTypeCommand.cpp
    commands/KoPathCombineCommand.cpp
    commands/KoPathSeparateCommand.cpp
    commands/KoSubpathRemoveCommand.cpp
    commands/KoSubpathJoinCommand.cpp
    commands/KoParameterHandleMoveCommand.cpp
    commands/KoParameterToPathCommand.cpp
    commands/KoShapeTransformCommand.cpp
)

kde4_add_library(flake SHARED ${flake_SRCS})

target_link_libraries(flake ${KDE4_KDEUI_LIBS} komain kstore pigment)

if(HAVE_OPENGL)
    target_link_libraries(flake ${OPENGL_LIBRARIES} ${QT_QTOPENGL_LIBRARY})
endif(HAVE_OPENGL)

set_target_properties(flake PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )

########### install files ###############

install(TARGETS flake  DESTINATION ${LIB_INSTALL_DIR})
install( FILES flake.desktop flaketool.desktop flakeshape.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

