project(kritaflake)

include_directories(
    ${CMAKE_SOURCE_DIR}/libs/flake/commands
    ${CMAKE_SOURCE_DIR}/libs/flake/tools
    ${CMAKE_SOURCE_DIR}/libs/flake/svg
    ${CMAKE_SOURCE_DIR}/libs/flake/text
    ${CMAKE_BINARY_DIR}/libs/flake
)

add_subdirectory(styles)
add_subdirectory(tests)
add_subdirectory(resources/tests)

set(kritaflake_SRCS
    KoGradientHelper.cpp
    KoFlake.cpp
    KoCanvasBase.cpp
    KoResourceManager_p.cpp
    KoDerivedResourceConverter.cpp
    KoActiveCanvasResourceDependency.cpp
    KoResourceUpdateMediator.cpp
    KoCanvasResourceProvider.cpp
    KoDocumentResourceManager.cpp
    KoCanvasObserverBase.cpp
    KoCanvasSupervisor.cpp
    KoDockFactoryBase.cpp
    KoDockRegistry.cpp
    KoDataCenterBase.cpp
    KoInsets.cpp
    KoPathShape.cpp
    KoPathPoint.cpp
    KoPathSegment.cpp
    KoSelection.cpp
    KoSelectedShapesProxy.cpp
    KoSelectedShapesProxySimple.cpp
    KoShape.cpp
    KoShapeAnchor.cpp
    KoShapeControllerBase.cpp
    KoShapeApplicationData.cpp
    KoShapeContainer.cpp
    KoShapeContainerModel.cpp
    KoShapeGroup.cpp
    KoShapeManager.cpp
    KoShapePaintingContext.cpp
    KoFrameShape.cpp
    KoMarker.cpp
    KoMarkerCollection.cpp
    KoToolBase.cpp
    KoCanvasController.cpp
    KoCanvasControllerWidget.cpp
    KoCanvasControllerWidgetViewport_p.cpp
    KoShapeRegistry.cpp
    KoDeferredShapeFactoryBase.cpp
    KoToolFactoryBase.cpp
    KoPathShapeFactory.cpp
    KoShapeFactoryBase.cpp
    KoShapeUserData.cpp
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
    KoPathShapeLoader.cpp
    KoShapeStrokeModel.cpp
    KoShapeStroke.cpp
    KoShapeBackground.cpp
    KoColorBackground.cpp
    KoGradientBackground.cpp
    KoMeshGradientBackground.cpp
    KoHatchBackground.cpp
    KoPatternBackground.cpp
    KoVectorPatternBackground.cpp
    KoShapeFillWrapper.cpp
    KoShapeFillResourceConnector.cpp
    KoShapeConfigWidgetBase.cpp
    KoDrag.cpp
    KoSvgPaste.cpp
    KoSnapGuide.cpp
    KoSnapProxy.cpp
    KoSnapStrategy.cpp
    KoSnapData.cpp
    KoShapeShadow.cpp
    KoSharedLoadingData.cpp
    KoSharedSavingData.cpp
    KoViewConverter.cpp
    KoInputDeviceHandler.cpp
    KoInputDeviceHandlerEvent.cpp
    KoInputDeviceHandlerRegistry.cpp
    KoImageData.cpp
    KoImageData_p.cpp
    KoImageCollection.cpp
    KoFilterEffect.cpp
    KoFilterEffectStack.cpp
    KoFilterEffectFactoryBase.cpp
    KoFilterEffectRegistry.cpp
    KoFilterEffectConfigWidgetBase.cpp
    KoFilterEffectRenderContext.cpp
    KoFilterEffectLoadingContext.cpp
    KoTextShapeDataBase.cpp
    KoTosContainer.cpp
    KoTosContainerModel.cpp
    KoClipPath.cpp
    KoClipMask.cpp
    KoClipMaskPainter.cpp
    KoCurveFit.cpp

    commands/KoShapeGroupCommand.cpp
    commands/KoShapeAlignCommand.cpp
    commands/KoShapeBackgroundCommand.cpp
    commands/KoShapeCreateCommand.cpp
    commands/KoShapeDeleteCommand.cpp
    commands/KoShapeDistributeCommand.cpp
    commands/KoShapeLockCommand.cpp
    commands/KoShapeMoveCommand.cpp
    commands/KoShapeResizeCommand.cpp
    commands/KoShapeShearCommand.cpp
    commands/KoShapeSizeCommand.cpp
    commands/KoShapeStrokeCommand.cpp
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
    commands/KoSubpathRemoveCommand.cpp
    commands/KoSubpathJoinCommand.cpp
    commands/KoParameterHandleMoveCommand.cpp
    commands/KoParameterToPathCommand.cpp
    commands/KoShapeTransformCommand.cpp
    commands/KoPathFillRuleCommand.cpp
    commands/KoShapeShadowCommand.cpp
    commands/KoPathReverseCommand.cpp
    commands/KoShapeRenameCommand.cpp
    commands/KoShapeRunAroundCommand.cpp
    commands/KoPathPointMergeCommand.cpp
    commands/KoShapeTransparencyCommand.cpp
    commands/KoShapeClipCommand.cpp
    commands/KoShapeUnclipCommand.cpp
    commands/KoPathShapeMarkerCommand.cpp
    commands/KoMultiPathPointMergeCommand.cpp
    commands/KoMultiPathPointJoinCommand.cpp
    commands/KoKeepShapesSelectedCommand.cpp
    commands/KoPathMergeUtils.cpp
    commands/KoAddRemoveShapeCommands.cpp

    html/HtmlSavingContext.cpp
    html/HtmlWriter.cpp

    tools/KoPathToolFactory.cpp
    tools/KoPathTool.cpp
    tools/KoPathToolSelection.cpp
    tools/KoPathToolHandle.cpp
    tools/PathToolOptionWidget.cpp
    tools/KoPathPointRubberSelectStrategy.cpp
    tools/KoPathPointMoveStrategy.cpp
    tools/KoPathControlPointMoveStrategy.cpp
    tools/KoParameterChangeStrategy.cpp
    tools/KoZoomTool.cpp
    tools/KoZoomToolFactory.cpp
    tools/KoZoomToolWidget.cpp
    tools/KoZoomStrategy.cpp
    tools/KoInteractionTool.cpp
    tools/KoInteractionStrategy.cpp
    tools/KoInteractionStrategyFactory.cpp
    tools/KoShapeRubberSelectStrategy.cpp
    tools/KoPathSegmentChangeStrategy.cpp

    svg/KoShapePainter.cpp
    svg/SvgUtil.cpp
    svg/SvgGraphicContext.cpp
    svg/SvgSavingContext.cpp
    svg/SvgWriter.cpp
    svg/SvgStyleWriter.cpp
    svg/SvgShape.cpp
    svg/SvgParser.cpp
    svg/SvgStyleParser.cpp
    svg/SvgGradientHelper.cpp
    svg/SvgFilterHelper.cpp
    svg/SvgCssHelper.cpp
    svg/SvgClipPathHelper.cpp
    svg/SvgLoadingContext.cpp
    svg/SvgShapeFactory.cpp
    svg/parsers/SvgTransformParser.cpp
    svg/SvgMeshGradient.cpp
    svg/SvgMeshPatch.cpp
    svg/SvgMeshArray.cpp

    text/KoSvgText.cpp
    text/KoSvgTextProperties.cpp
    text/KoSvgTextChunkShape.cpp
    text/KoSvgTextShape.cpp
    text/KoSvgTextShapeMarkupConverter.cpp

    resources/KoSvgSymbolCollectionResource.cpp
    resources/KoGamutMask.cpp

    FlakeDebug.cpp
    tests/MockShapes.cpp
)

if (KSeExpr_FOUND)
    set(kritaflake_SRCS ${kritaflake_SRCS}
        resources/KisSeExprScript.cpp
    )
endif()

ki18n_wrap_ui(kritaflake_SRCS
    tools/PathToolOptionWidgetBase.ui
    tools/KoZoomToolWidget.ui
)

add_library(kritaflake SHARED ${kritaflake_SRCS})
generate_export_header(kritaflake BASE_NAME kritaflake)

target_include_directories(kritaflake
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/commands>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tools>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/svg>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/text>
)

target_link_libraries(kritaflake 
    kritapigment 
    kritawidgetutils 
    kritacommand 

    KF5::WidgetsAddons 
    KF5::ConfigCore KF5::I18n 

    Qt5::Svg KF5::CoreAddons 
    Qt5::Gui 
    Qt5::Xml

    KDAB::kddockwidgets
)

set_target_properties(kritaflake PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)

install(TARGETS kritaflake  ${INSTALL_TARGETS_DEFAULT_ARGS})

