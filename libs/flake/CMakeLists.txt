project(kritaflake)

include_directories(
    ${CMAKE_SOURCE_DIR}/libs/flake/commands
    ${CMAKE_SOURCE_DIR}/libs/flake/tools
    ${CMAKE_SOURCE_DIR}/libs/flake/svg
    ${CMAKE_SOURCE_DIR}/libs/flake/text
    ${CMAKE_SOURCE_DIR}/libs/flake/xsimd
    ${CMAKE_SOURCE_DIR}/libs/flake/resources
    ${CMAKE_BINARY_DIR}/libs/flake
)

add_subdirectory(styles)
add_subdirectory(tests)
add_subdirectory(resources/tests)

set(DISABLE_CLIP_MASK_PAINTER_ON_MACOS FALSE)
if (APPLE)
    set(DISABLE_CLIP_MASK_PAINTER_ON_MACOS TRUE)
endif()

# generate krita_xsimd_macos_workaround.h
configure_file(krita_xsimd_macos_workaround.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/krita_xsimd_macos_workaround.h)

if(NOT DISABLE_CLIP_MASK_PAINTER_ON_MACOS)
    ko_compile_for_all_implementations(__per_arch_alpha_applicator_factory_objs xsimd/KoClipMaskApplicatorFactoryImpl.cpp)
else()
    set(__per_arch_alpha_applicator_factory_objs xsimd/KoClipMaskApplicatorFactoryImpl.cpp)
endif()

set(kritaflake_SRCS
    KoGradientHelper.cpp
    KoFlake.cpp
    KoCanvasBase.cpp
    KoResourceManager_p.cpp
    KoDerivedResourceConverter.cpp
    KoActiveCanvasResourceDependency.cpp
    KoAbstractCanvasResourceInterface.cpp
    KoResourceUpdateMediator.cpp
    KoCanvasResourceProvider.cpp
    KoDocumentResourceManager.cpp
    KoCanvasObserverBase.cpp
    KoCanvasSupervisor.cpp
    KoDockFactoryBase.cpp
    KoDockRegistry.cpp
    KoInsets.cpp
    KoPathShape.cpp
    KoPathPoint.cpp
    KoPathSegment.cpp
    KoSelection.cpp
    KoSelectedShapesProxy.cpp
    KoSelectedShapesProxySimple.cpp
    KoShape.cpp
    KoShapeAnchor.cpp
    KoShapeBulkActionLock.cpp
    KoShapeControllerBase.cpp
    KoShapeApplicationData.cpp
    KoShapeContainer.cpp
    KoShapeContainerModel.cpp
    KoShapeGroup.cpp
    KoShapeManager.cpp
    KoMarker.cpp
    KoMarkerCollection.cpp
    KoToolBase.cpp
    KoCanvasController.cpp
    KoZoomMode.cpp
    KoZoomState.cpp
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
    KoViewTransformStillPoint.cpp
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
    KoClipMaskApplicatorBase.cpp
    ${__per_arch_alpha_applicator_factory_objs}
    KoClipMaskPainter.cpp
    KoClipPath.cpp
    KoClipMask.cpp
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
    commands/KoShapePaintOrderCommand.cpp
    commands/KoShapeMergeTextPropertiesCommand.cpp
    commands/KoSvgConvertTextTypeCommand.cpp
    commands/KoSvgTextAddRemoveShapeCommands.cpp
    commands/KoSvgTextReorderShapeInsideCommand.cpp
    commands/KoSvgTextFlipShapeContourTypeCommand.cpp

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
    svg/parsers/SvgTransformParser.cpp
    svg/SvgMeshGradient.cpp
    svg/SvgMeshPatch.cpp
    svg/SvgMeshArray.cpp

    text/KisTofuGlyph.cpp
    text/KoSvgText.cpp
    text/KoSvgTextProperties.cpp
    text/KoSvgTextContentElement.cpp
    text/KoSvgTextShape.cpp
    text/KoSvgTextLoader.cpp
    text/KoSvgTextShape_p_glyphs.cpp
    text/KoSvgTextShape_p_layout.cpp
    text/KoSvgTextShape_p_output.cpp
    text/KoSvgTextShapeLayoutFunc_inShape.cpp
    text/KoSvgTextShapeLayoutFunc_lines.cpp
    text/KoSvgTextShapeMarkupConverter.cpp
    text/KoFontRegistry.cpp
    text/KoCssTextUtils.cpp
    text/KoPolygonUtils.cpp
    text/KoSvgTextPropertyData.cpp
    text/KoSvgTextPropertiesInterface.h
    text/lager/KoSvgTextPropertiesModel.cpp
    text/lager/CssLengthPercentageModel.cpp
    text/lager/LineHeightModel.cpp
    text/lager/TextIndentModel.cpp
    text/lager/TabSizeModel.cpp
    text/lager/TextTransformModel.cpp
    text/lager/CssFontStyleModel.cpp
    text/lager/FontVariantLigaturesModel.cpp
    text/lager/FontVariantNumericModel.cpp
    text/lager/FontVariantEastAsianModel.cpp
    text/KoFFWWSConverter.cpp
    text/KoFontChangeTracker.cpp
    text/KoWritingSystemUtils.cpp
    text/KoFontGlyphModel.cpp
    text/KoOpenTypeFeatureInfoFactory.cpp
    text/data/KoUnicodeBlockData.cpp
    text/KoSvgTextShapeOutlineHelper.cpp

    resources/KoSvgSymbolCollectionResource.cpp
    resources/KoGamutMask.cpp
    resources/KoFontFamily.cpp
    resources/KoFontStorage.cpp
    resources/KoCssStylePreset.cpp

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

kis_add_library(kritaflake SHARED ${kritaflake_SRCS})
generate_export_header(kritaflake BASE_NAME kritaflake)

target_include_directories(kritaflake
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/commands>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tools>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/svg>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/text>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/resources>
)

set_target_properties(libraqm PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(kritaflake
    PUBLIC
        kritapigment
        kritawidgetutils
        kritacommand
        KF${KF_MAJOR}::WidgetsAddons
        Qt${QT_MAJOR_VERSION}::Svg
        KF${KF_MAJOR}::CoreAddons
        KF${KF_MAJOR}::ConfigCore
        KF${KF_MAJOR}::I18n
        Qt${QT_MAJOR_VERSION}::Gui
        Qt${QT_MAJOR_VERSION}::Xml
        Boost::boost
        Freetype::Freetype
        HarfBuzz::HarfBuzz
    PRIVATE
        Raqm::Raqm
        Fontconfig::Fontconfig
        libunibreak::libunibreak
)

if (QT_MAJOR_VERSION STREQUAL "6")
target_link_libraries(kritaflake
    PRIVATE
        Qt${QT_MAJOR_VERSION}::OpenGLWidgets
)
endif()

set_target_properties(kritaflake PROPERTIES
    VERSION ${GENERIC_KRITA_LIB_VERSION} SOVERSION ${GENERIC_KRITA_LIB_SOVERSION}
)

install(TARGETS kritaflake  ${INSTALL_TARGETS_DEFAULT_ARGS})

