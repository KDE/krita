project(flake)

include_directories(${FLAKE_INCLUDES} ${QT_INCLUDES})

if(KDE4_BUILD_TESTS)
    # only with this definition will the FLAKE_TEST_EXPORT macro do something
    add_definitions(-DCOMPILING_TESTS)
endif(KDE4_BUILD_TESTS)

add_subdirectory(tests)
add_subdirectory(pics)

set(flake_SRCS
    KoFlake.cpp
    KoCanvasBase.cpp
    KoCanvasObserver.cpp
    KoCanvasResourceProvider.cpp
    KoCutController.cpp
    KoCopyController.cpp
    KoDockRegistry.cpp
    KoInsets.cpp
    KoPasteController.cpp
    KoGuidesData.cpp
    KoPathShape.cpp
    KoPathPoint.cpp
    KoPathSegment.cpp
    KoSelection.cpp
    KoShape.cpp
    KoShapeApplicationData.cpp
    KoShapeContainer.cpp
    KoShapeContainerModel.cpp
    KoChildrenData.cpp
    KoShapeGroup.cpp
    KoShapeManagerPaintingStrategy.cpp
    KoShapeManager.cpp
    KoShapePainter.cpp
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
    KoLoadingShapeUpdater.cpp
    KoSvgPathParser.cpp
    KoPathShapeLoader.cpp
    ShapeDeleter_p.cpp
    KoShapeBorderModel.cpp
    KoShapeBorderFactory.cpp
    KoShapeBorderRegistry.cpp
    KoLineBorder.cpp
    KoLineBorderFactory.cpp
    KoShapeBackground.cpp
    KoColorBackground.cpp
    KoGradientBackground.cpp
    KoPatternBackground.cpp
    KoShapeConfigWidgetBase.cpp
    KoDrag.cpp
    KoDragOdfSaveHelper.cpp
    KoShapeOdfSaveHelper.cpp
    KoShapePaste.cpp
    KoConnectionShape.cpp
    KoConnectionShapeLoadingUpdater.cpp
    KoConnectionShapeFactory.cpp
    KoConnectionShapeConfigWidget.cpp
    KoSnapGuide.cpp
    KoSnapProxy.cpp
    KoSnapStrategy.cpp
    KoSnapData.cpp
    SnapGuideConfigWidget.cpp
    KoShapeShadow.cpp
    KoViewConverter.cpp
    KoDevice.cpp
    KoDeviceEvent.cpp
    KoDeviceRegistry.cpp
    KoEventAction.cpp
    KoEventActionData.cpp
    KoEventActionFactory.cpp
    KoEventActionRegistry.cpp
    KoEventActionWidget.cpp
    KoImageData.cpp
    KoImageData_p.cpp
    KoImageCollection.cpp
    KoOdfWorkaround.cpp
    KoFilterEffect.cpp
    KoFilterEffectStack.cpp
    KoFilterEffectFactory.cpp
    KoFilterEffectRegistry.cpp
    KoFilterEffectConfigWidgetBase.cpp
    KoFilterEffectRenderContext.cpp
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
    commands/KoEventActionAddCommand.cpp
    commands/KoEventActionRemoveCommand.cpp
    commands/KoShapeRenameCommand.cpp
    commands/KoPathPointMergeCommand.cpp
    commands/KoShapeTransparencyCommand.cpp
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
    tools/KoPathSegmentChangeStrategy.cpp
    tools/KoGuidesTool.cpp
)

kde4_add_ui_files(flake_SRCS
    tools/PathToolOptionWidgetBase.ui
    KoConnectionShapeConfigWidget.ui
    SnapGuideConfigWidget.ui
    tools/KoZoomToolWidget.ui
)

kde4_add_library(flake SHARED ${flake_SRCS})

if(NOT NO_PIGMENT)
    set(EXTRA_LIBS pigmentcms)
else(NOT NO_PIGMENT)
    set(EXTRA_LIBS koplugin)
endif(NOT NO_PIGMENT)

target_link_libraries(flake ${KDE4_KDEUI_LIBS} ${EXTRA_LIBS} koodf)

target_link_libraries(flake LINK_INTERFACE_LIBRARIES ${EXTRA_LIBS} koodf)

if(HAVE_OPENGL)
    target_link_libraries(flake ${OPENGL_LIBRARIES} ${QT_QTOPENGL_LIBRARY})
endif(HAVE_OPENGL)

set_target_properties(flake PROPERTIES
    VERSION ${GENERIC_KOFFICE_LIB_VERSION} SOVERSION ${GENERIC_KOFFICE_LIB_SOVERSION}
)

########### install files ###############

install(TARGETS flake  ${INSTALL_TARGETS_DEFAULT_ARGS})
install( FILES 
    flake.desktop 
    flaketool.desktop 
    flakeshape.desktop 
    flakedevice.desktop 
    presentationeventaction.desktop 
    scripteventaction.desktop 
    filtereffect.desktop
    DESTINATION ${SERVICETYPES_INSTALL_DIR})

install(
    FILES
    flake_export.h
    FlakeColor.h
    KoCanvasBase.h
    KoCanvasController.h
    KoCanvasObserver.h
    KoCanvasObserverProvider.h
    KoCanvasResourceProvider.h
    KoCopyController.h
    KoCutController.h
    KoDataCenter.h
    KoDockFactory.h
    KoDockRegistry.h
    KoDrag.h
    KoDragOdfSaveHelper.h
    KoFlake.h
    KoFrameShape.h
    KoGuidesData.h
    KoImageCollection.h
    KoImageData.h
    KoInputDevice.h
    KoInsets.h
    KoPathSegment.h
    KoPointerEvent.h
    KoSelection.h
    KoShape.h
    KoShapeBorderModel.h
    KoToolSelection.h
    KoShapeContainer.h
    KoShapeContainerModel.h
    KoChildrenData.h
    KoShapeController.h
    KoShapeControllerBase.h
    KoShapeFactory.h
    KoShapeGroup.h
    KoShapeLayer.h
    KoShapePainter.h
    KoShapeLoadingContext.h
    KoShapeOdfSaveHelper.h
    KoLoadingShapeUpdater.h
    KoShapeConfigWidgetBase.h
    KoShapeManager.h
    KoShapeRegistry.h
    KoShapeSavingContext.h
    KoShapeUserData.h
    KoShapeApplicationData.h
    KoTool.h
    KoToolManager.h
    KoToolFactory.h
    KoToolProxy.h
    KoToolRegistry.h
    commands/KoEventActionAddCommand.h
    commands/KoEventActionRemoveCommand.h
    commands/KoParameterToPathCommand.h
    commands/KoPathCombineCommand.h
    commands/KoPathControlPointMoveCommand.h
    commands/KoPathFillRuleCommand.h
    commands/KoPathPointInsertCommand.h
    commands/KoPathPointMergeCommand.h
    commands/KoPathPointMoveCommand.h
    commands/KoPathPointRemoveCommand.h
    commands/KoPathPointTypeCommand.h
    commands/KoPathReverseCommand.h
    commands/KoPathSegmentTypeCommand.h
    commands/KoPathSeparateCommand.h
    commands/KoShapeAlignCommand.h
    commands/KoShapeBackgroundCommand.h
    commands/KoShapeBorderCommand.h
    commands/KoShapeCreateCommand.h
    commands/KoShapeDeleteCommand.h
    commands/KoShapeDistributeCommand.h
    commands/KoShapeGroupCommand.h
    commands/KoShapeKeepAspectRatioCommand.h
    commands/KoShapeLockCommand.h
    commands/KoShapeMoveCommand.h
    commands/KoShapeRenameCommand.h
    commands/KoShapeReorderCommand.h
    commands/KoShapeShadowCommand.h
    commands/KoShapeShearCommand.h
    commands/KoShapeSizeCommand.h
    commands/KoShapeTransformCommand.h
    commands/KoShapeUngroupCommand.h
    commands/KoSubpathRemoveCommand.h
    tools/KoInteractionTool.h
    KoViewConverter.h
    DESTINATION
    ${INCLUDE_INSTALL_DIR} COMPONENT Devel
)
