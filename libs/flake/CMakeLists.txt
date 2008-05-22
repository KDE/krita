include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${KOMAIN_INCLUDES} ${PIGMENT_INCLUDES})

if(KDE4_BUILD_TESTS)
    # only with this definition will the FLAKE_TEST_EXPORT macro do something
    add_definitions(-DCOMPILING_TESTS)
endif(KDE4_BUILD_TESTS)


add_subdirectory(tests)
add_subdirectory(pics)

set(flake_SRCS
    KoCanvasBase.cpp
    KoCanvasObserver.cpp
    KoCanvasResourceProvider.cpp
    KoPathShape.cpp
    KoPathPoint.cpp
    KoPathSegment.cpp
    KoSelection.cpp
    KoShape.cpp
    KoShapeContainer.cpp
    KoShapeGroup.cpp
    KoShapeManagerPaintingStrategy.cpp
    KoShapeManager.cpp
    KoFrameShape.cpp
    KoTool.cpp
    KoCanvasController.cpp
    KoCanvasController_p.cpp
    KoShapeRegistry.cpp
    KoToolFactory.cpp
    KoPathShapeFactory.cpp
    KoShapeFactory.cpp
    KoShapeUserData.cpp
    KoPointGroup.cpp
    KoParameterShape.cpp
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
    KoSvgPathParser.cpp
    KoPathShapeLoader.cpp
    ShapeDeleter_p.cpp
    KoShapeBorderModel.cpp
    KoShapeBorderFactory.cpp
    KoShapeBorderRegistry.cpp
    KoLineBorder.cpp
    KoLineBorderFactory.cpp
    KoShapeStyleWriter.cpp
    KoShapeConfigWidgetBase.cpp
    KoDrag.cpp
    KoDragOdfSaveHelper.cpp
    KoShapeOdfSaveHelper.cpp
    KoShapePaste.cpp
    KoConnectionShape.cpp
    KoConnectionShapeFactory.cpp
    KoConnectionShapeConfigWidget.cpp
    KoSnapGuide.cpp
    KoSnapStrategy.cpp
    KoSnapData.cpp
    SnapGuideConfigWidget.cpp
    KoShapeShadow.cpp
    KoDevice.cpp
    KoDeviceEvent.cpp
    KoDeviceRegistry.cpp
    commands/KoShapeGroupCommand.cpp
    commands/KoShapeAlignCommand.cpp
    commands/KoShapeBackgroundCommand.cpp
    commands/KoShapeBorderCommand.cpp
    commands/KoShapeCreateCommand.cpp
    commands/KoShapeDeleteCommand.cpp
    commands/KoShapeDistributeCommand.cpp
    commands/KoShapeLockCommand.cpp
    commands/KoShapeMoveCommand.cpp
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
    commands/KoPathFillRuleCommand.cpp
    commands/KoConnectionShapeTypeCommand.cpp
    commands/KoShapeShadowCommand.cpp
    commands/KoPathReverseCommand.cpp
    tools/KoCreatePathTool.cpp
    tools/KoCreatePathToolFactory.cpp
    tools/KoCreateShapeStrategy.cpp
    tools/KoPathToolFactory.cpp
    tools/KoPathTool.cpp
    tools/KoPathToolSelection.cpp
    tools/KoPathToolHandle.cpp
    tools/PathToolOptionWidget.cpp
    tools/KoPathPointRubberSelectStrategy.cpp
    tools/KoPathPointMoveStrategy.cpp
    tools/KoPathConnectionPointStrategy.cpp
    tools/KoPathControlPointMoveStrategy.cpp
    tools/KoParameterChangeStrategy.cpp
    tools/KoZoomTool.cpp
    tools/KoZoomToolFactory.cpp
    tools/KoZoomToolWidget.cpp
    tools/KoZoomStrategy.cpp
    tools/KoPanTool.cpp
    tools/KoPanToolFactory.cpp
    tools/KoInteractionTool.cpp
    tools/KoInteractionStrategy.cpp
    tools/KoCreateShapesTool.cpp
    tools/KoCreateShapesToolFactory.cpp
    tools/KoShapeRubberSelectStrategy.cpp
)

kde4_add_ui_files(flake_SRCS
    tools/PathToolOptionWidgetBase.ui
    KoConnectionShapeConfigWidget.ui
    SnapGuideConfigWidget.ui
    tools/KoZoomToolWidget.ui
)


kde4_add_library(flake SHARED ${flake_SRCS})

target_link_libraries(flake ${KDE4_KDEUI_LIBS} komain kstore pigmentcms)

if(HAVE_OPENGL)
    target_link_libraries(flake ${OPENGL_LIBRARIES} ${QT_QTOPENGL_LIBRARY})
endif(HAVE_OPENGL)

set_target_properties(flake PROPERTIES VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION} )

########### install files ###############

install(TARGETS flake  DESTINATION ${LIB_INSTALL_DIR})
install( FILES flake.desktop flaketool.desktop flakeshape.desktop flakedevice.desktop DESTINATION ${SERVICETYPES_INSTALL_DIR})

install(
    FILES
    flake_export.h
    DESTINATION
    ${INCLUDE_INSTALL_DIR}
)
