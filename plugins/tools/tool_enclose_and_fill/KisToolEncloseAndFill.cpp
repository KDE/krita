/*
 * KDE. Krita Project.
 *
 * SPDX-FileCopyrightText: 2022 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <kis_debug.h>
#include <klocalizedstring.h>

#include <QCheckBox>
#include <QPushButton>

#include <KisOptionButtonStrip.h>
#include <KisOptionCollectionWidget.h>
#include <KoGroupButton.h>

#include <ksharedconfig.h>

#include <KoCanvasBase.h>
#include <KoPointerEvent.h>

#include <kis_layer.h>
#include <kis_painter.h>
#include <resources/KoPattern.h>
#include <kis_selection.h>

#include <KisViewManager.h>
#include <canvas/kis_canvas2.h>
#include <widgets/kis_cmb_composite.h>
#include <kis_slider_spin_box.h>
#include <kis_cursor.h>
#include "kis_resources_snapshot.h"
#include <kis_color_filter_combo.h>
#include <KisAngleSelector.h>
#include <KoGroupButton.h>
#include <kis_color_button.h>
#include <kis_color_label_selector_widget.h>
#include <kis_cmb_composite.h>
#include <kis_image_animation_interface.h>

#include <kis_stroke_strategy_undo_command_based.h>
#include <commands_new/kis_processing_command.h>
#include <commands_new/kis_update_command.h>
#include <kis_command_utils.h>
#include <functional>
#include <kis_group_layer.h>
#include <kis_layer_utils.h>

#include <KisSpinBoxI18nHelper.h>
#include <KisPart.h>
#include <KisDocument.h>
#include <kis_dummies_facade.h>
#include <KoShapeControllerBase.h>
#include <kis_shape_controller.h>
#include <kis_canvas_resource_provider.h>

#include <KoCompositeOpRegistry.h>

#include <processing/KisEncloseAndFillProcessingVisitor.h>

#include "KisToolEncloseAndFill.h"
#include "subtools/KisRectangleEnclosingProducer.h"
#include "subtools/KisEllipseEnclosingProducer.h"
#include "subtools/KisPathEnclosingProducer.h"
#include "subtools/KisLassoEnclosingProducer.h"
#include "subtools/KisBrushEnclosingProducer.h"

KisToolEncloseAndFill::KisToolEncloseAndFill(KoCanvasBase * canvas)
    : KisDynamicDelegatedTool<KisToolShape>(canvas, QCursor())
{
    setObjectName("tool_enclose_and_fill");
}

KisToolEncloseAndFill::~KisToolEncloseAndFill()
{}

void KisToolEncloseAndFill::resetCursorStyle()
{
    KisDynamicDelegatedTool::resetCursorStyle();
    overrideCursorIfNotEditable();
}

void KisToolEncloseAndFill::activate(const QSet<KoShape*> &shapes)
{
    KisDynamicDelegatedTool::activate(shapes);
    m_configGroup = KSharedConfig::openConfig()->group(toolId());
    KisCanvas2 *kisCanvas = static_cast<KisCanvas2*>(canvas());
    KisCanvasResourceProvider *resourceProvider = kisCanvas->viewManager()->canvasResourceProvider();
    if (resourceProvider) {
        connect(resourceProvider,
                SIGNAL(sigNodeChanged(const KisNodeSP)),
                this,
                SLOT(slot_currentNodeChanged(const KisNodeSP)));
        slot_currentNodeChanged(currentNode());
    }
}

void KisToolEncloseAndFill::deactivate()
{
    m_referencePaintDevice = nullptr;
    m_referenceNodeList = nullptr;
    KisCanvas2 *kisCanvas = static_cast<KisCanvas2*>(canvas());
    KisCanvasResourceProvider *resourceProvider = kisCanvas->viewManager()->canvasResourceProvider();
    if (resourceProvider) {
        disconnect(resourceProvider,
                   SIGNAL(sigNodeChanged(const KisNodeSP)),
                   this,
                   SLOT(slot_currentNodeChanged(const KisNodeSP)));
    }
    slot_currentNodeChanged(nullptr);
    KisDynamicDelegatedTool::deactivate();
}

void KisToolEncloseAndFill::setupEnclosingSubtool()
{
    if (delegateTool()) {
        delegateTool()->deactivate();
    }

    if (m_enclosingMethod == Ellipse) {
        KisEllipseEnclosingProducer *newDelegateTool = new KisEllipseEnclosingProducer(canvas());
        setDelegateTool(reinterpret_cast<KisDynamicDelegateTool<KisToolShape>*>(newDelegateTool));
        setCursor(newDelegateTool->cursor());
    } else if (m_enclosingMethod == Path) {
        KisPathEnclosingProducer *newDelegateTool = new KisPathEnclosingProducer(canvas());
        setDelegateTool(reinterpret_cast<KisDynamicDelegateTool<KisToolShape>*>(newDelegateTool));
        setCursor(newDelegateTool->cursor());
    } else if (m_enclosingMethod == Lasso) {
        KisLassoEnclosingProducer *newDelegateTool = new KisLassoEnclosingProducer(canvas());
        setDelegateTool(reinterpret_cast<KisDynamicDelegateTool<KisToolShape>*>(newDelegateTool));
        setCursor(newDelegateTool->cursor());
    } else if (m_enclosingMethod == Brush) {
        KisBrushEnclosingProducer *newDelegateTool = new KisBrushEnclosingProducer(canvas());
        setDelegateTool(reinterpret_cast<KisDynamicDelegateTool<KisToolShape>*>(newDelegateTool));
        setCursor(newDelegateTool->cursor());
    } else {
        KisRectangleEnclosingProducer *newDelegateTool = new KisRectangleEnclosingProducer(canvas());
        setDelegateTool(reinterpret_cast<KisDynamicDelegateTool<KisToolShape>*>(newDelegateTool));
        setCursor(newDelegateTool->cursor());
    }

    connect(delegateTool(), SIGNAL(enclosingMaskProduced(KisPixelSelectionSP)), SLOT(slot_delegateTool_enclosingMaskProduced(KisPixelSelectionSP)));

    if (isActivated()) {
        delegateTool()->activate(QSet<KoShape*>());
    }
}

bool KisToolEncloseAndFill::subtoolHasUserInteractionRunning() const
{
    if (!delegateTool()) {
        return false;
    }
    
    if (m_enclosingMethod == Rectangle) {
        return reinterpret_cast<KisRectangleEnclosingProducer*>(delegateTool())->hasUserInteractionRunning();
    } else if (m_enclosingMethod == Ellipse) {
        return reinterpret_cast<KisEllipseEnclosingProducer*>(delegateTool())->hasUserInteractionRunning();
    } else if (m_enclosingMethod == Path) {
        return reinterpret_cast<KisPathEnclosingProducer*>(delegateTool())->hasUserInteractionRunning();
    } else if (m_enclosingMethod == Lasso) {
        return reinterpret_cast<KisLassoEnclosingProducer*>(delegateTool())->hasUserInteractionRunning();
    } else if (m_enclosingMethod == Brush) {
        return reinterpret_cast<KisBrushEnclosingProducer*>(delegateTool())->hasUserInteractionRunning();
    }
    return false;
}

void KisToolEncloseAndFill::beginPrimaryAction(KoPointerEvent *event)
{
    // cannot use enclose and fill tool on non-painting layers.
    // this logic triggers with multiple layer types like vector layer, clone layer, file layer, group layer
    if (currentNode().isNull() || currentNode()->inherits("KisShapeLayer") || nodePaintAbility() != NodePaintAbility::PAINT) {
        KisCanvas2 * kiscanvas = static_cast<KisCanvas2*>(canvas());
        kiscanvas->viewManager()->
                showFloatingMessage(
                    i18n("You cannot use this tool with the selected layer type"),
                    QIcon(), 2000, KisFloatingMessage::Medium, Qt::AlignCenter);
        event->ignore();
        return;
    }

    if (!nodeEditable()) {
        event->ignore();
        return;
    }

    KisDynamicDelegatedTool::beginPrimaryAction(event);
}

void KisToolEncloseAndFill::activateAlternateAction(AlternateAction action)
{
    if (subtoolHasUserInteractionRunning()) {
        if (delegateTool()) {
            delegateTool()->activatePrimaryAction();
        }
        return;
    }
    KisDynamicDelegatedTool::activateAlternateAction(action);
}

void KisToolEncloseAndFill::deactivateAlternateAction(AlternateAction action)
{
    if (subtoolHasUserInteractionRunning()) {
        return;
    }
    KisDynamicDelegatedTool::deactivateAlternateAction(action);
}

void KisToolEncloseAndFill::beginAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (subtoolHasUserInteractionRunning()) {
        if (delegateTool()) {
            delegateTool()->beginPrimaryAction(event);
        }
        return;
    }
    KisDynamicDelegatedTool::beginAlternateAction(event, action);
    m_alternateActionStarted = true;
}

void KisToolEncloseAndFill::continueAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (subtoolHasUserInteractionRunning()) {
        if (delegateTool()) {
            delegateTool()->continuePrimaryAction(event);
        }
        return;
    }
    if (!m_alternateActionStarted) {
        return;
    }
    KisDynamicDelegatedTool::continueAlternateAction(event, action);
}

void KisToolEncloseAndFill::endAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (subtoolHasUserInteractionRunning()) {
        if (delegateTool()) {
            delegateTool()->endPrimaryAction(event);
        }
        return;
    }
    if (!m_alternateActionStarted) {
        return;
    }
    KisDynamicDelegatedTool::endAlternateAction(event, action);
    m_alternateActionStarted = false;
}

void KisToolEncloseAndFill::beginAlternateDoubleClickAction(KoPointerEvent *event, AlternateAction action)
{
    if (subtoolHasUserInteractionRunning()) {
        if (delegateTool()) {
            delegateTool()->beginPrimaryDoubleClickAction(event);
        }
        return;
    }
    KisDynamicDelegatedTool::beginAlternateDoubleClickAction(event, action);
}

void KisToolEncloseAndFill::slot_delegateTool_enclosingMaskProduced(KisPixelSelectionSP enclosingMask)
{
    KisStrokeStrategyUndoCommandBased *strategy =
            new KisStrokeStrategyUndoCommandBased(kundo2_i18n("Enclose and Fill"), false, image().data());
    strategy->setSupportsWrapAroundMode(true);
    m_fillStrokeId = image()->startStroke(strategy);
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_fillStrokeId);

    m_dirtyRect.reset(new QRect);

    KisResourcesSnapshotSP resources =
        new KisResourcesSnapshot(image(), currentNode(), this->canvas()->resourceManager());

    if (m_reference == CurrentLayer) {
        m_referencePaintDevice = currentNode()->paintDevice();
    } else if (m_reference == AllLayers) {
        m_referencePaintDevice = currentImage()->projection();
    } else if (m_reference == ColorLabeledLayers) {
        if (!m_referenceNodeList) {
            m_referencePaintDevice = KisMergeLabeledLayersCommand::createRefPaintDevice(image(), "Enclose and Fill Tool Reference Result Paint Device");
            m_referenceNodeList.reset(new KisMergeLabeledLayersCommand::ReferenceNodeInfoList);
        }
        KisPaintDeviceSP newReferencePaintDevice = KisMergeLabeledLayersCommand::createRefPaintDevice(image(), "Enclose and Fill Tool Reference Result Paint Device");
        KisMergeLabeledLayersCommand::ReferenceNodeInfoListSP newReferenceNodeList(new KisMergeLabeledLayersCommand::ReferenceNodeInfoList);
        const int currentTime = image()->animationInterface()->currentTime();
        image()->addJob(
            m_fillStrokeId,
            new KisStrokeStrategyUndoCommandBased::Data(
                KUndo2CommandSP(new KisMergeLabeledLayersCommand(
                    image(),
                    m_referenceNodeList,
                    newReferenceNodeList,
                    m_referencePaintDevice,
                    newReferencePaintDevice,
                    m_selectedColorLabels,
                    KisMergeLabeledLayersCommand::GroupSelectionPolicy_SelectIfColorLabeled,
                    m_previousTime != currentTime,
                    m_useActiveLayer ? currentNode() : nullptr
                )),
                false,
                KisStrokeJobData::SEQUENTIAL,
                KisStrokeJobData::EXCLUSIVE
            )
        );
        m_referencePaintDevice = newReferencePaintDevice;
        m_referenceNodeList = newReferenceNodeList;
        m_previousTime = currentTime;
    }

    if (m_reference != ColorLabeledLayers) {
        // Reset this so that the device from color labeled layers gets
        // regenerated when that mode is selected again
        m_referenceNodeList.reset();
    }

    QTransform transform;
    transform.rotate(m_patternRotation);
    const qreal normalizedScale = m_patternScale * 0.01;
    transform.scale(normalizedScale, normalizedScale);
    resources->setFillTransform(transform);

    KisProcessingVisitorSP visitor =
        new KisEncloseAndFillProcessingVisitor(m_referencePaintDevice,
                                               enclosingMask,
                                               resources->activeSelection(),
                                               resources,
                                               m_regionSelectionMethod,
                                               m_regionSelectionColor,
                                               m_regionSelectionInvert,
                                               m_regionSelectionIncludeContourRegions,
                                               false,
                                               m_fillThreshold,
                                               m_fillOpacitySpread,
                                               m_closeGap,
                                               m_antiAlias,
                                               m_expand,
                                               m_stopGrowingAtDarkestPixel,
                                               m_feather,
                                               m_useSelectionAsBoundary,
                                               m_fillType == FillWithPattern,
                                               false,
                                               m_fillType == FillWithBackgroundColor,
                                               m_useCustomBlendingOptions,
                                               m_customOpacity / 100.0,
                                               m_customCompositeOp,
                                               m_dirtyRect);

    image()->addJob(
        m_fillStrokeId,
        new KisStrokeStrategyUndoCommandBased::Data(
            KUndo2CommandSP(new KisProcessingCommand(visitor, currentNode())),
            false,
            KisStrokeJobData::SEQUENTIAL,
            KisStrokeJobData::EXCLUSIVE
        )
    );

    image()->addJob(
        m_fillStrokeId,
        new KisStrokeStrategyUndoCommandBased::Data(
            KUndo2CommandSP(new KisUpdateCommand(currentNode(), m_dirtyRect, image().data())),
            false,
            KisStrokeJobData::SEQUENTIAL,
            KisStrokeJobData::EXCLUSIVE
        )
    );

    image()->endStroke(m_fillStrokeId);

    m_fillStrokeId = nullptr;
    m_dirtyRect = nullptr;
}

int KisToolEncloseAndFill::flags() const
{
    return KisDynamicDelegatedTool::flags() | KisTool::FLAG_USES_CUSTOM_SIZE | KisTool::FLAG_USES_CUSTOM_PRESET;
}

QWidget* KisToolEncloseAndFill::createOptionWidget()
{
    loadConfiguration();

    // Create widgets
    KisOptionButtonStrip *optionButtonStripEnclosingMethod =
        new KisOptionButtonStrip;
    m_buttonEnclosingMethodRectangle =
        optionButtonStripEnclosingMethod->addButton(
            KisIconUtils::loadIcon("tool_rect_selection"));
    m_buttonEnclosingMethodEllipse =
        optionButtonStripEnclosingMethod->addButton(
            KisIconUtils::loadIcon("tool_elliptical_selection"));
    m_buttonEnclosingMethodPath = optionButtonStripEnclosingMethod->addButton(
        KisIconUtils::loadIcon("tool_path_selection"));
    m_buttonEnclosingMethodLasso = optionButtonStripEnclosingMethod->addButton(
        KisIconUtils::loadIcon("tool_outline_selection"));
    m_buttonEnclosingMethodBrush = optionButtonStripEnclosingMethod->addButton(
        KisIconUtils::loadIcon("krita_tool_freehand"));
    m_buttonEnclosingMethodLasso->setChecked(true);

    m_comboBoxRegionSelectionMethod = new QComboBox;
    m_comboBoxRegionSelectionMethod->addItem(
        regionSelectionMethodToUserString(RegionSelectionMethod::SelectAllRegions),
        static_cast<int>(RegionSelectionMethod::SelectAllRegions)
    );
    m_comboBoxRegionSelectionMethod->addItem(
        regionSelectionMethodToUserString(RegionSelectionMethod::SelectRegionsFilledWithSpecificColor),
        static_cast<int>(RegionSelectionMethod::SelectRegionsFilledWithSpecificColor)
    );
    m_comboBoxRegionSelectionMethod->addItem(
        regionSelectionMethodToUserString(RegionSelectionMethod::SelectRegionsFilledWithTransparent),
        static_cast<int>(RegionSelectionMethod::SelectRegionsFilledWithTransparent)
    );
    m_comboBoxRegionSelectionMethod->addItem(
        regionSelectionMethodToUserString(RegionSelectionMethod::SelectRegionsFilledWithSpecificColorOrTransparent),
        static_cast<int>(RegionSelectionMethod::SelectRegionsFilledWithSpecificColorOrTransparent)
    );
    m_comboBoxRegionSelectionMethod->addItem(
        regionSelectionMethodToUserString(RegionSelectionMethod::SelectAllRegionsExceptFilledWithSpecificColor),
        static_cast<int>(RegionSelectionMethod::SelectAllRegionsExceptFilledWithSpecificColor)
    );
    m_comboBoxRegionSelectionMethod->addItem(
        regionSelectionMethodToUserString(RegionSelectionMethod::SelectAllRegionsExceptFilledWithTransparent),
        static_cast<int>(RegionSelectionMethod::SelectAllRegionsExceptFilledWithTransparent)
    );
    m_comboBoxRegionSelectionMethod->addItem(
        regionSelectionMethodToUserString(
            RegionSelectionMethod::SelectAllRegionsExceptFilledWithSpecificColorOrTransparent
        ),
        static_cast<int>(RegionSelectionMethod::SelectAllRegionsExceptFilledWithSpecificColorOrTransparent)
    );
    m_comboBoxRegionSelectionMethod->addItem(
        regionSelectionMethodToUserString(RegionSelectionMethod::SelectRegionsSurroundedBySpecificColor),
        static_cast<int>(RegionSelectionMethod::SelectRegionsSurroundedBySpecificColor)
    );
    m_comboBoxRegionSelectionMethod->addItem(
        regionSelectionMethodToUserString(RegionSelectionMethod::SelectRegionsSurroundedByTransparent),
        static_cast<int>(RegionSelectionMethod::SelectRegionsSurroundedByTransparent)
    );
    m_comboBoxRegionSelectionMethod->addItem(
        regionSelectionMethodToUserString(RegionSelectionMethod::SelectRegionsSurroundedBySpecificColorOrTransparent),
        static_cast<int>(RegionSelectionMethod::SelectRegionsSurroundedBySpecificColorOrTransparent)
    );
    m_comboBoxRegionSelectionMethod->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_comboBoxRegionSelectionMethod->setMinimumContentsLength(15);
    m_comboBoxRegionSelectionMethod->view()->setMinimumWidth(
        m_comboBoxRegionSelectionMethod->view()->sizeHintForColumn(0)
    );
    m_buttonRegionSelectionColor = new KisColorButton;
    m_checkBoxRegionSelectionInvert =
        new QCheckBox(
            i18nc("The 'invert' checkbox in enclose and fill tool",
                  "Invert")
        );
    m_checkBoxRegionSelectionIncludeContourRegions =
        new QCheckBox(
            i18nc("The 'include contour regions' checkbox in enclose and fill tool",
                  "Include contour regions")
        );
    m_checkBoxRegionSelectionIncludeContourRegions->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

    KisOptionButtonStrip *optionButtonStripFillWith = new KisOptionButtonStrip;
    m_buttonFillWithFG = optionButtonStripFillWith->addButton(
        KisIconUtils::loadIcon("object-order-lower-calligra"));
    m_buttonFillWithBG = optionButtonStripFillWith->addButton(
        KisIconUtils::loadIcon("object-order-raise-calligra"));
    m_buttonFillWithPattern =
        optionButtonStripFillWith->addButton(KisIconUtils::loadIcon("pattern"));
    m_buttonFillWithFG->setChecked(true);
    m_sliderPatternScale = new KisDoubleSliderSpinBox;
    m_sliderPatternScale->setRange(0, 10000, 2);
    m_sliderPatternScale->setSoftMaximum(500);
    KisSpinBoxI18nHelper::setText(m_sliderPatternScale,
                                  i18nc("The pattern 'scale' spinbox in enclose and fill tool options; {n} is the "
                                        "number value, % is the percent sign",
                                        "Scale: {n}%"));
    m_angleSelectorPatternRotation = new KisAngleSelector;
    m_angleSelectorPatternRotation->setFlipOptionsMode(KisAngleSelector::FlipOptionsMode_ContextMenu);
    m_angleSelectorPatternRotation->setIncreasingDirection(KisAngleGauge::IncreasingDirection_Clockwise);
    m_checkBoxCustomBlendingOptions = new QCheckBox(i18n("Use custom blending options"));
    m_checkBoxCustomBlendingOptions->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    m_sliderCustomOpacity = new KisSliderSpinBox;
    m_sliderCustomOpacity->setRange(0, 100);
    KisSpinBoxI18nHelper::setText(m_sliderCustomOpacity,
                                  i18nc("{n} is the number value, % is the percent sign", "Opacity: {n}%"));
    m_comboBoxCustomCompositeOp = new KisCompositeOpComboBox;

    m_sliderFillThreshold = new KisSliderSpinBox;
    m_sliderFillThreshold->setPrefix(i18nc("The 'threshold' spinbox prefix in enclose and fill tool options", "Threshold: "));
    m_sliderFillThreshold->setRange(1, 100);
    m_sliderFillOpacitySpread = new KisSliderSpinBox;
    KisSpinBoxI18nHelper::setText(
        m_sliderFillOpacitySpread,
        i18nc("The 'spread' spinbox in enclose and fill tool options; {n} is the number value, % is the percent sign",
              "Spread: {n}%"));
    m_sliderFillOpacitySpread->setRange(0, 100);
    m_sliderCloseGap = new KisSliderSpinBox;
    m_sliderCloseGap->setPrefix(i18nc("The 'close gap' spinbox prefix in enclose and fill tool options", "Close Gap: "));
    m_sliderCloseGap->setRange(0, 32);
    m_sliderCloseGap->setSuffix(i18n(" px"));
    m_checkBoxSelectionAsBoundary =
        new QCheckBox(
            i18nc("The 'use selection as boundary' checkbox in enclose and fill tool to use selection borders as boundary when filling",
                  "Use selection as boundary")
        );
    m_checkBoxSelectionAsBoundary->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

    m_checkBoxAntiAlias = new QCheckBox(i18nc("The anti-alias checkbox in enclose and fill tool options", "Anti-aliasing"));
    KisOptionCollectionWidget *containerGrow = new KisOptionCollectionWidget;
    m_sliderExpand = new KisSliderSpinBox;
    m_sliderExpand->setPrefix(i18nc("The 'grow/shrink' spinbox prefix in enclose and fill tool options", "Grow: "));
    m_sliderExpand->setRange(-400, 400);
    m_sliderExpand->setSoftRange(-40, 40);
    m_sliderExpand->setSuffix(i18n(" px"));
    m_buttonStopGrowingAtDarkestPixel = new QToolButton;
    m_buttonStopGrowingAtDarkestPixel->setAutoRaise(true);
    m_buttonStopGrowingAtDarkestPixel->setCheckable(true);
    m_buttonStopGrowingAtDarkestPixel->setIcon(KisIconUtils::loadIcon("stop-at-boundary"));
    containerGrow->appendWidget("sliderExpand", m_sliderExpand);
    containerGrow->appendWidget("buttonStopGrowingAtDarkestPixel", m_buttonStopGrowingAtDarkestPixel);
    containerGrow->setOrientation(Qt::Horizontal);
    m_sliderFeather = new KisSliderSpinBox;
    m_sliderFeather->setPrefix(i18nc("The 'feather' spinbox prefix in enclose and fill tool options", "Feather: "));
    m_sliderFeather->setRange(0, 400);
    m_sliderFeather->setSoftRange(0, 40);
    m_sliderFeather->setSuffix(i18n(" px"));

    KisOptionButtonStrip *optionButtonStripReference = new KisOptionButtonStrip;
    m_buttonReferenceCurrent = optionButtonStripReference->addButton(
        KisIconUtils::loadIcon("current-layer"));
    m_buttonReferenceAll = optionButtonStripReference->addButton(
        KisIconUtils::loadIcon("all-layers"));
    m_buttonReferenceLabeled =
        optionButtonStripReference->addButton(KisIconUtils::loadIcon("tag"));
    m_buttonReferenceCurrent->setChecked(true);
    m_widgetLabels = new KisColorLabelSelectorWidget;
    m_widgetLabels->setExclusive(false);
    m_widgetLabels->setButtonSize(20);
    m_widgetLabels->setButtonWrapEnabled(true);
    m_widgetLabels->setMouseDragEnabled(true);
    m_checkBoxUseActiveLayer = new QCheckBox(i18n("Use active layer"));

    QPushButton *buttonReset = new QPushButton(i18nc("The 'reset' button in enclose and fill tool options", "Reset"));

    // Set the tooltips
    m_buttonEnclosingMethodRectangle->setToolTip(i18n("Rectangle"));
    m_buttonEnclosingMethodEllipse->setToolTip(i18n("Ellipse"));
    m_buttonEnclosingMethodPath->setToolTip(i18n("Bezier Curve"));
    m_buttonEnclosingMethodLasso->setToolTip(i18n("Lasso"));
    m_buttonEnclosingMethodBrush->setToolTip(i18n("Brush"));
    m_comboBoxRegionSelectionMethod->setToolTip(regionSelectionMethodToUserString(m_regionSelectionMethod));
    m_checkBoxRegionSelectionInvert->setToolTip(i18n("Enable to fill opposite regions instead"));
    m_checkBoxRegionSelectionIncludeContourRegions->setToolTip(i18n("Enable to also fill shapes that touch the contour of the enclosing region"));
    m_buttonFillWithFG->setToolTip(i18n("Foreground color"));
    m_buttonFillWithBG->setToolTip(i18n("Background color"));
    m_buttonFillWithPattern->setToolTip(i18n("Pattern"));
    m_sliderPatternScale->setToolTip(i18n("Set the scale of the pattern"));
    m_angleSelectorPatternRotation->setToolTip(i18n("Set the rotation of the pattern"));
    m_checkBoxCustomBlendingOptions->setToolTip(i18n("Set custom blending options instead of using the brush ones"));
    m_sliderCustomOpacity->setToolTip(i18n("Set a custom opacity for the fill"));
    m_comboBoxCustomCompositeOp->setToolTip(i18n("Set a custom blend mode for the fill"));

    m_sliderFillThreshold->setToolTip(i18n("Set the color similarity tolerance of the fill. Increasing threshold increases the range of similar colors to be filled."));
    m_sliderFillOpacitySpread->setToolTip(i18n("Set the extent of the opaque portion of the fill. Decreasing spread decreases opacity of fill areas depending on color similarity."));
    m_sliderCloseGap->setToolTip(i18n("Close gaps in lines up to the set amount"));
    m_checkBoxSelectionAsBoundary->setToolTip(i18n("Set if the contour of the active selection should be treated as a boundary when filling the region"));

    m_checkBoxAntiAlias->setToolTip(i18n("Smooths the edges of the fill"));
    m_sliderExpand->setToolTip(i18n("Grow or shrink the fill by the set amount"));
    m_buttonStopGrowingAtDarkestPixel->setToolTip(i18n("Stop growing at the darkest and/or most opaque pixels"));
    m_sliderFeather->setToolTip(i18n("Blur the fill by the set amount"));

    m_buttonReferenceCurrent->setToolTip(i18n("Fill regions found from the active layer"));
    m_buttonReferenceAll->setToolTip(i18n("Fill regions found from the merging of all layers"));
    m_buttonReferenceLabeled->setToolTip(i18n("Fill regions found from the merging of layers with specific color labels"));
    m_checkBoxUseActiveLayer->setToolTip(i18n("Includes the active layer in regions found from merging of layers with specific color labels"));

    buttonReset->setToolTip(i18n("Reset the options to their default values"));

    // Construct the option widget
    m_optionWidget = new KisOptionCollectionWidget;
    m_optionWidget->setContentsMargins(0, 10, 0, 10);
    m_optionWidget->setSeparatorsVisible(true);

    KisOptionCollectionWidgetWithHeader *sectionEnclosingMethod =
        new KisOptionCollectionWidgetWithHeader(
            i18nc("The 'enclosing method' section label in enclose and fill tool options", "Enclosing method")
        );
    sectionEnclosingMethod->setPrimaryWidget(optionButtonStripEnclosingMethod);
    m_optionWidget->appendWidget("sectionEnclosingMethod", sectionEnclosingMethod);

    KisOptionCollectionWidget *widgetLabelsGroup = new KisOptionCollectionWidget;
    widgetLabelsGroup->appendWidget("labelWidget", m_widgetLabels);
    widgetLabelsGroup->appendWidget("checkBoxUseActiveLayer", m_checkBoxUseActiveLayer);
    widgetLabelsGroup->setWidgetsMargin(0);

    KisOptionCollectionWidgetWithHeader *sectionReference =
        new KisOptionCollectionWidgetWithHeader(
            i18nc("The 'reference' section label in enclose and fill tool options", "Reference")
        );
    sectionReference->setPrimaryWidget(optionButtonStripReference);
    sectionReference->appendWidget("widgetLabels", widgetLabelsGroup);
    sectionReference->setWidgetVisible("widgetLabels", false);
    m_optionWidget->appendWidget("sectionReference", sectionReference);


    KisOptionCollectionWidgetWithHeader *sectionWhatToFill =
        new KisOptionCollectionWidgetWithHeader(
            i18nc("The 'target regions' section label in enclose and fill tool options", "Target regions")
        );
    sectionWhatToFill->setPrimaryWidget(m_comboBoxRegionSelectionMethod);
    sectionWhatToFill->appendWidget("buttonRegionSelectionColor", m_buttonRegionSelectionColor);
    sectionWhatToFill->appendWidget("checkBoxRegionSelectionInvert", m_checkBoxRegionSelectionInvert);
    sectionWhatToFill->appendWidget("checkBoxRegionSelectionIncludeContourRegions", m_checkBoxRegionSelectionIncludeContourRegions);
    m_optionWidget->appendWidget("sectionWhatToFill", sectionWhatToFill);

    KisOptionCollectionWidgetWithHeader *sectionFillWith =
        new KisOptionCollectionWidgetWithHeader(
            i18nc("The 'fill source' section label in enclose and fill tool options", "Fill source")
        );
    sectionFillWith->setPrimaryWidget(optionButtonStripFillWith);
    sectionFillWith->appendWidget("sliderPatternScale", m_sliderPatternScale);
    sectionFillWith->appendWidget("angleSelectorPatternRotation", m_angleSelectorPatternRotation);
    sectionFillWith->appendWidget("checkBoxCustomBlendingOptions", m_checkBoxCustomBlendingOptions);
    sectionFillWith->appendWidget("sliderCustomOpacity", m_sliderCustomOpacity);
    sectionFillWith->appendWidget("comboBoxCustomCompositeOp", m_comboBoxCustomCompositeOp);
    sectionFillWith->setWidgetVisible("sliderPatternScale", false);
    sectionFillWith->setWidgetVisible("angleSelectorPatternRotation", false);
    m_optionWidget->appendWidget("sectionFillWith", sectionFillWith);

    KisOptionCollectionWidgetWithHeader *sectionRegionExtent =
        new KisOptionCollectionWidgetWithHeader(
            i18nc("The 'fill extent' section label in enclose and fill tool options", "Fill extent")
        );
    sectionRegionExtent->appendWidget("sliderThreshold", m_sliderFillThreshold);
    sectionRegionExtent->appendWidget("sliderSpread", m_sliderFillOpacitySpread);
    sectionRegionExtent->appendWidget("sliderCloseGap", m_sliderCloseGap);
    sectionRegionExtent->appendWidget("checkBoxSelectionAsBoundary", m_checkBoxSelectionAsBoundary);
    m_optionWidget->appendWidget("sectionRegionExtent", sectionRegionExtent);

    KisOptionCollectionWidgetWithHeader *sectionAdjustments =
        new KisOptionCollectionWidgetWithHeader(
            i18nc("The 'adjustments' section label in enclose and fill tool options", "Adjustments")
        );
    sectionAdjustments->appendWidget("checkBoxAntiAlias", m_checkBoxAntiAlias);
    sectionAdjustments->appendWidget("containerGrow", containerGrow);
    sectionAdjustments->appendWidget("sliderFeather", m_sliderFeather);
    m_optionWidget->appendWidget("sectionAdjustments", sectionAdjustments);

    m_optionWidget->appendWidget("buttonReset", buttonReset);

    // Initialize widgets
    if (m_enclosingMethod == Rectangle) {
        m_buttonEnclosingMethodRectangle->setChecked(true);
    } else if (m_enclosingMethod == Ellipse) {
        m_buttonEnclosingMethodEllipse->setChecked(true);
    } else if (m_enclosingMethod == Path) {
        m_buttonEnclosingMethodPath->setChecked(true);
    } else if (m_enclosingMethod == Lasso) {
        m_buttonEnclosingMethodLasso->setChecked(true);
    } else {
        m_buttonEnclosingMethodBrush->setChecked(true);
    }
    m_comboBoxRegionSelectionMethod->setCurrentIndex(m_comboBoxRegionSelectionMethod->findData(static_cast<int>(m_regionSelectionMethod)));
    m_buttonRegionSelectionColor->setColor(m_regionSelectionColor);
    sectionWhatToFill->setWidgetVisible(
        "buttonRegionSelectionColor",
        m_regionSelectionMethod == RegionSelectionMethod::SelectRegionsFilledWithSpecificColor ||
        m_regionSelectionMethod == RegionSelectionMethod::SelectRegionsFilledWithSpecificColorOrTransparent ||
        m_regionSelectionMethod == RegionSelectionMethod::SelectAllRegionsExceptFilledWithSpecificColor ||
        m_regionSelectionMethod == RegionSelectionMethod::SelectAllRegionsExceptFilledWithSpecificColorOrTransparent ||
        m_regionSelectionMethod == RegionSelectionMethod::SelectRegionsSurroundedBySpecificColor ||
        m_regionSelectionMethod == RegionSelectionMethod::SelectRegionsSurroundedBySpecificColorOrTransparent
    );
    m_checkBoxRegionSelectionInvert->setChecked(m_regionSelectionInvert);
    m_checkBoxRegionSelectionIncludeContourRegions->setChecked(m_regionSelectionIncludeContourRegions);
    sectionWhatToFill->setWidgetVisible(
        "checkBoxRegionSelectionIncludeContourRegions",
        m_regionSelectionMethod == RegionSelectionMethod::SelectAllRegions ||
        m_regionSelectionMethod == RegionSelectionMethod::SelectRegionsFilledWithSpecificColor ||
        m_regionSelectionMethod == RegionSelectionMethod::SelectRegionsFilledWithTransparent ||
        m_regionSelectionMethod == RegionSelectionMethod::SelectRegionsFilledWithSpecificColorOrTransparent ||
        m_regionSelectionMethod == RegionSelectionMethod::SelectAllRegionsExceptFilledWithSpecificColor ||
        m_regionSelectionMethod == RegionSelectionMethod::SelectAllRegionsExceptFilledWithTransparent ||
        m_regionSelectionMethod == RegionSelectionMethod::SelectAllRegionsExceptFilledWithSpecificColorOrTransparent
    );
    if (m_fillType == FillWithBackgroundColor) {
        m_buttonFillWithBG->setChecked(true);
    } else if (m_fillType == FillWithPattern) {
        m_buttonFillWithPattern->setChecked(true);
        sectionFillWith->setWidgetVisible("sliderPatternScale", true);
        sectionFillWith->setWidgetVisible("angleSelectorPatternRotation", true);
    }
    m_sliderPatternScale->setValue(m_patternScale);
    m_angleSelectorPatternRotation->setAngle(m_patternRotation);
    m_checkBoxCustomBlendingOptions->setChecked(m_useCustomBlendingOptions);
    m_sliderCustomOpacity->setValue(m_customOpacity);
    slot_colorSpaceChanged(currentNode() && currentNode()->paintDevice()
                           ? currentNode()->paintDevice()->colorSpace()
                           : nullptr);
    m_comboBoxCustomCompositeOp->selectCompositeOp(KoID(m_customCompositeOp));
    if (!m_useCustomBlendingOptions) {
        sectionFillWith->setWidgetVisible("sliderCustomOpacity", false);
        sectionFillWith->setWidgetVisible("comboBoxCustomCompositeOp", false);
    }
    m_sliderFillThreshold->setValue(m_fillThreshold);
    m_sliderFillOpacitySpread->setValue(m_fillOpacitySpread);
    m_sliderCloseGap->setValue(m_closeGap);
    m_checkBoxSelectionAsBoundary->setChecked(m_useSelectionAsBoundary);
    m_checkBoxAntiAlias->setChecked(m_antiAlias);
    m_sliderExpand->setValue(m_expand);
    m_buttonStopGrowingAtDarkestPixel->setChecked(m_stopGrowingAtDarkestPixel);
    m_sliderFeather->setValue(m_feather);
    if (m_reference == AllLayers) {
        m_buttonReferenceAll->setChecked(true);
    } else if (m_reference == ColorLabeledLayers) {
        m_buttonReferenceLabeled->setChecked(true);
        sectionReference->setWidgetVisible("widgetLabels", true);
    }
    m_widgetLabels->setSelection(m_selectedColorLabels);
    m_checkBoxUseActiveLayer->setChecked(m_useActiveLayer);

    // Make connections
    connect(optionButtonStripEnclosingMethod,
            SIGNAL(buttonToggled(KoGroupButton *, bool)),
            SLOT(slot_optionButtonStripEnclosingMethod_buttonToggled(
                KoGroupButton *,
                bool)));
    connect(m_comboBoxRegionSelectionMethod,
            SIGNAL(currentIndexChanged(int)),
            SLOT(slot_comboBoxRegionSelectionMethod_currentIndexChanged(int)));
    connect(m_buttonRegionSelectionColor,
            SIGNAL(changed(const KoColor&)),
            SLOT(slot_buttonRegionSelectionColor_changed(const KoColor&)));
    connect(m_checkBoxRegionSelectionInvert,
            SIGNAL(toggled(bool)),
            SLOT(slot_checkBoxRegionSelectionInvert_toggled(bool)));
    connect(m_checkBoxRegionSelectionIncludeContourRegions,
            SIGNAL(toggled(bool)),
            SLOT(slot_checkBoxRegionSelectionIncludeContourRegions_toggled(bool)));
    connect(optionButtonStripFillWith,
            SIGNAL(buttonToggled(KoGroupButton *, bool)),
            SLOT(slot_optionButtonStripFillWith_buttonToggled(KoGroupButton *, bool)));
    connect(m_sliderPatternScale,
            SIGNAL(valueChanged(double)),
            SLOT(slot_sliderPatternScale_valueChanged(double)));
    connect(m_angleSelectorPatternRotation,
            SIGNAL(angleChanged(double)),
            SLOT(slot_angleSelectorPatternRotation_angleChanged(double)));
    connect(m_checkBoxCustomBlendingOptions,
            SIGNAL(toggled(bool)),
            SLOT(slot_checkBoxUseCustomBlendingOptions_toggled(bool)));
    connect(m_sliderCustomOpacity,
            SIGNAL(valueChanged(int)),
            SLOT(slot_sliderCustomOpacity_valueChanged(int)));
    connect(m_comboBoxCustomCompositeOp,
            SIGNAL(currentIndexChanged(int)),
            SLOT(slot_comboBoxCustomCompositeOp_currentIndexChanged(int)));
    connect(m_sliderFillThreshold,
            SIGNAL(valueChanged(int)),
            SLOT(slot_sliderFillThreshold_valueChanged(int)));
    connect(m_sliderFillOpacitySpread,
            SIGNAL(valueChanged(int)),
            SLOT(slot_sliderFillOpacitySpread_valueChanged(int)));
    connect(m_sliderCloseGap,
            SIGNAL(valueChanged(int)),
            SLOT(slot_sliderCloseGap_valueChanged(int)));
    connect(m_checkBoxSelectionAsBoundary,
            SIGNAL(toggled(bool)),
            SLOT(slot_checkBoxSelectionAsBoundary_toggled(bool)));
    connect(m_checkBoxAntiAlias,
            SIGNAL(toggled(bool)),
            SLOT(slot_checkBoxAntiAlias_toggled(bool)));
    connect(m_sliderExpand,
            SIGNAL(valueChanged(int)),
            SLOT(slot_sliderExpand_valueChanged(int)));
    connect(m_buttonStopGrowingAtDarkestPixel,
            SIGNAL(toggled(bool)),
            SLOT(slot_buttonStopGrowingAtDarkestPixel_toggled(bool)));
    connect(m_sliderFeather,
            SIGNAL(valueChanged(int)),
            SLOT(slot_sliderFeather_valueChanged(int)));
    connect(optionButtonStripReference,
            SIGNAL(buttonToggled(KoGroupButton *, bool)),
            SLOT(slot_optionButtonStripReference_buttonToggled(KoGroupButton *, bool)));
    connect(m_widgetLabels,
            SIGNAL(selectionChanged()),
            SLOT(slot_widgetLabels_selectionChanged()));
    connect(m_checkBoxUseActiveLayer,
            SIGNAL(toggled(bool)),
            SLOT(slot_checkBoxUseActiveLayer_toggled(bool)));
    connect(buttonReset,
            SIGNAL(clicked()),
            SLOT(slot_buttonReset_clicked()));
    
    return m_optionWidget;
}

void KisToolEncloseAndFill::loadConfiguration()
{
    m_enclosingMethod = loadEnclosingMethodFromConfig();
    m_regionSelectionMethod = loadRegionSelectionMethodFromConfig();
    m_regionSelectionColor = loadRegionSelectionColorFromConfig();
    m_regionSelectionInvert = m_configGroup.readEntry<bool>("regionSelectionInvert", false);
    m_regionSelectionIncludeContourRegions = m_configGroup.readEntry<bool>("regionSelectionIncludeContourRegions", false);
    {
        const QString fillTypeStr = m_configGroup.readEntry<QString>("fillWith", "");
        if (fillTypeStr == "foregroundColor") {
            m_fillType = FillWithForegroundColor;
        } else if (fillTypeStr == "backgroundColor") {
            m_fillType = FillWithBackgroundColor;
        } else if (fillTypeStr == "pattern") {
            m_fillType = FillWithPattern;
        } else {
            if (m_configGroup.readEntry<bool>("usePattern", false)) {
                m_fillType = FillWithPattern;
            } else {
                m_fillType = FillWithForegroundColor;
            }
        }
    }
    m_patternScale = m_configGroup.readEntry<qreal>("patternScale", 100.0);
    m_patternRotation = m_configGroup.readEntry<qreal>("patternRotate", 0.0);
    m_useCustomBlendingOptions = m_configGroup.readEntry<bool>("useCustomBlendingOptions", false);
    m_customOpacity = qBound(0, m_configGroup.readEntry<int>("customOpacity", 100), 100);
    m_customCompositeOp = m_configGroup.readEntry<QString>("customCompositeOp", COMPOSITE_OVER);
    if (KoCompositeOpRegistry::instance().getKoID(m_customCompositeOp).id().isNull()) {
        m_customCompositeOp = COMPOSITE_OVER;
    }
    m_fillThreshold = m_configGroup.readEntry<int>("fillThreshold", 8);
    m_fillOpacitySpread = m_configGroup.readEntry<int>("fillOpacitySpread", 100);
    m_closeGap = m_configGroup.readEntry<int>("closeGapAmount", 0);
    m_useSelectionAsBoundary = m_configGroup.readEntry<bool>("useSelectionAsBoundary", true);
    m_antiAlias = m_configGroup.readEntry<bool>("antiAlias", false);
    m_expand = m_configGroup.readEntry<int>("expand", 0);
    m_stopGrowingAtDarkestPixel = m_configGroup.readEntry<bool>("stopGrowingAtDarkestPixel", false);
    m_feather = m_configGroup.readEntry<int>("feather", 0);
    {
        const QString sampleLayersModeStr = m_configGroup.readEntry<QString>("reference", "currentLayer");
        if (sampleLayersModeStr == "allLayers") {
            m_reference = AllLayers;
        } else if (sampleLayersModeStr == "colorLabeledLayers") {
            m_reference = ColorLabeledLayers;
        } else {
            m_reference = CurrentLayer;
        }
    }
    {
        const QStringList colorLabelsStr = m_configGroup.readEntry<QString>("colorLabels", "").split(',', Qt::SkipEmptyParts);

        m_selectedColorLabels.clear();
        for (const QString &colorLabelStr : colorLabelsStr) {
            bool ok;
            const int colorLabel = colorLabelStr.toInt(&ok);
            if (ok) {
                m_selectedColorLabels << colorLabel;
            }
        }
    }
    m_useActiveLayer = m_configGroup.readEntry<bool>("useActiveLayer", false);

    setupEnclosingSubtool();
}

KisToolEncloseAndFill::EnclosingMethod KisToolEncloseAndFill::loadEnclosingMethodFromConfig() const
{
    return configStringToEnclosingMethod(m_configGroup.readEntry("enclosingMethod", enclosingMethodToConfigString(defaultEnclosingMethod())));
}

void KisToolEncloseAndFill::saveEnclosingMethodToConfig(EnclosingMethod enclosingMethod)
{
    m_configGroup.writeEntry("enclosingMethod", enclosingMethodToConfigString(enclosingMethod));
}

QString KisToolEncloseAndFill::enclosingMethodToConfigString(EnclosingMethod enclosingMethod) const
{
    switch (enclosingMethod) {
        case Rectangle: return "rectangle";
        case Ellipse: return "ellipse";
        case Path: return "path";
        case Brush: return "brush";
        default: return "lasso";
    }
}

KisToolEncloseAndFill::EnclosingMethod KisToolEncloseAndFill::configStringToEnclosingMethod(const QString &configString) const
{
    if (configString == "rectangle") {
        return Rectangle;
    } else if (configString == "ellipse") {
        return Ellipse;
    } else if (configString == "path") {
        return Path;
    } else if (configString == "brush") {
        return Brush;
    }
    return Lasso;
}

QString KisToolEncloseAndFill::regionSelectionMethodToUserString(RegionSelectionMethod regionSelectionMethod) const
{
    if (regionSelectionMethod == RegionSelectionMethod::SelectAllRegions) {
        return i18nc("Region selection method in enclose and fill tool",
                     "All");
    } else if (regionSelectionMethod == RegionSelectionMethod::SelectRegionsFilledWithSpecificColor) {
        return i18nc("Region selection method in enclose and fill tool",
                     "Specific color");
    } else if (regionSelectionMethod == RegionSelectionMethod::SelectRegionsFilledWithTransparent) {
        return i18nc("Region selection method in enclose and fill tool",
                     "Transparency");
    } else if (regionSelectionMethod == RegionSelectionMethod::SelectRegionsFilledWithSpecificColorOrTransparent) {
        return i18nc("Region selection method in enclose and fill tool",
                     "Specific color or transparency");
    } else if (regionSelectionMethod == RegionSelectionMethod::SelectAllRegionsExceptFilledWithSpecificColor) {
        return i18nc("Region selection method in enclose and fill tool",
                     "All, excluding a specific color");
    } else if (regionSelectionMethod == RegionSelectionMethod::SelectAllRegionsExceptFilledWithTransparent) {
        return i18nc("Region selection method in enclose and fill tool",
                     "All, excluding transparency");
    } else if (regionSelectionMethod == RegionSelectionMethod::SelectAllRegionsExceptFilledWithSpecificColorOrTransparent) {
        return i18nc("Region selection method in enclose and fill tool",
                     "All, excluding a specific color or transparency");
    } else if (regionSelectionMethod == RegionSelectionMethod::SelectRegionsSurroundedBySpecificColor) {
        return i18nc("Region selection method in enclose and fill tool",
                     "Any surrounded by a specific color");
    } else if (regionSelectionMethod == RegionSelectionMethod::SelectRegionsSurroundedByTransparent) {
        return i18nc("Region selection method in enclose and fill tool",
                     "Any surrounded by transparency");
    } else if (regionSelectionMethod == RegionSelectionMethod::SelectRegionsSurroundedBySpecificColorOrTransparent) {
        return i18nc("Region selection method in enclose and fill tool",
                     "Any surrounded by a specific color or transparency");
    }
    return QString();
}

KisToolEncloseAndFill::RegionSelectionMethod KisToolEncloseAndFill::loadRegionSelectionMethodFromConfig() const
{
    return configStringToRegionSelectionMethod(m_configGroup.readEntry("regionSelectionMethod", regionSelectionMethodToConfigString(defaultRegionSelectionMethod())));
}

void KisToolEncloseAndFill::saveRegionSelectionMethodToConfig(RegionSelectionMethod regionSelectionMethod)
{
    m_configGroup.writeEntry("regionSelectionMethod", regionSelectionMethodToConfigString(regionSelectionMethod));
}

QString KisToolEncloseAndFill::regionSelectionMethodToConfigString(RegionSelectionMethod regionSelectionMethod) const
{
    if (regionSelectionMethod == RegionSelectionMethod::SelectAllRegions) {
        return "allRegions";
    } else if (regionSelectionMethod == RegionSelectionMethod::SelectRegionsFilledWithSpecificColor) {
        return "regionsFilledWithSpecificColor";
    } else if (regionSelectionMethod == RegionSelectionMethod::SelectRegionsFilledWithTransparent) {
        return "regionsFilledWithTransparent";
    } else if (regionSelectionMethod == RegionSelectionMethod::SelectRegionsFilledWithSpecificColorOrTransparent) {
        return "regionsFilledWithSpecificColorOrTransparent";
    } else if (regionSelectionMethod == RegionSelectionMethod::SelectAllRegionsExceptFilledWithSpecificColor) {
        return "allRegionsExceptFilledWithSpecificColor";
    } else if (regionSelectionMethod == RegionSelectionMethod::SelectAllRegionsExceptFilledWithTransparent) {
        return "allRegionsExceptFilledWithTransparent";
    } else if (regionSelectionMethod == RegionSelectionMethod::SelectAllRegionsExceptFilledWithSpecificColorOrTransparent) {
        return "allRegionsExceptFilledWithSpecificColorOrTransparent";
    } else if (regionSelectionMethod == RegionSelectionMethod::SelectRegionsSurroundedBySpecificColor) {
        return "regionsSurroundedBySpecificColor";
    } else if (regionSelectionMethod == RegionSelectionMethod::SelectRegionsSurroundedByTransparent) {
        return "regionsSurroundedByTransparent";
    } else if (regionSelectionMethod == RegionSelectionMethod::SelectRegionsSurroundedBySpecificColorOrTransparent) {
        return "regionsSurroundedBySpecificColorOrTransparent";
    }
    return QString();
}

KisToolEncloseAndFill::RegionSelectionMethod KisToolEncloseAndFill::configStringToRegionSelectionMethod(const QString &configString) const
{
    if (configString == "regionsFilledWithSpecificColor") {
        return RegionSelectionMethod::SelectRegionsFilledWithSpecificColor;
    } else if (configString == "regionsFilledWithTransparent") {
        return RegionSelectionMethod::SelectRegionsFilledWithTransparent;
    } else if (configString == "regionsFilledWithSpecificColorOrTransparent") {
        return RegionSelectionMethod::SelectRegionsFilledWithSpecificColorOrTransparent;
    } else if (configString == "allRegionsExceptFilledWithSpecificColor") {
        return RegionSelectionMethod::SelectAllRegionsExceptFilledWithSpecificColor;
    } else if (configString == "allRegionsExceptFilledWithTransparent") {
        return RegionSelectionMethod::SelectAllRegionsExceptFilledWithTransparent;
    } else if (configString == "allRegionsExceptFilledWithSpecificColorOrTransparent") {
        return RegionSelectionMethod::SelectAllRegionsExceptFilledWithSpecificColorOrTransparent;
    } else if (configString == "regionsSurroundedBySpecificColor") {
        return RegionSelectionMethod::SelectRegionsSurroundedBySpecificColor;
    } else if (configString == "regionsSurroundedByTransparent") {
        return RegionSelectionMethod::SelectRegionsSurroundedByTransparent;
    } else if (configString == "regionsSurroundedBySpecificColorOrTransparent") {
        return RegionSelectionMethod::SelectRegionsSurroundedBySpecificColorOrTransparent;
    }
    return RegionSelectionMethod::SelectAllRegions;
}

KoColor KisToolEncloseAndFill::loadRegionSelectionColorFromConfig()
{
    const QString xmlColor = m_configGroup.readEntry("regionSelectionColor", QString());
    QDomDocument doc;
    if (doc.setContent(xmlColor)) {
        QDomElement e = doc.documentElement().firstChild().toElement();
        QString channelDepthID = doc.documentElement().attribute("channeldepth", Integer16BitsColorDepthID.id());
        bool ok;
        if (e.hasAttribute("space") || e.tagName().toLower() == "srgb") {
            return KoColor::fromXML(e, channelDepthID, &ok);
        } else if (doc.documentElement().hasAttribute("space") || doc.documentElement().tagName().toLower() == "srgb"){
            return KoColor::fromXML(doc.documentElement(), channelDepthID, &ok);
        }
    }
    return KoColor();
}

KisToolEncloseAndFill::Reference KisToolEncloseAndFill::loadReferenceFromConfig() const
{
    if (m_configGroup.hasKey("reference")) {
        return configStringToReference(m_configGroup.readEntry("reference", referenceToConfigString(defaultReference())));
    } else {
        bool sampleMerged = m_configGroup.readEntry("sampleMerged", false);
        return sampleMerged ? AllLayers : CurrentLayer;
    }
    return CurrentLayer;
}

void KisToolEncloseAndFill::saveReferenceToConfig(Reference reference)
{
    m_configGroup.writeEntry("reference", referenceToConfigString(reference));
}

QString KisToolEncloseAndFill::referenceToConfigString(Reference reference) const
{
    if (reference == AllLayers) {
        return "allLayers";
    } else if (reference == ColorLabeledLayers) {
        return "colorLabeledLayers";
    }
    return "currentLayer";
}

KisToolEncloseAndFill::Reference KisToolEncloseAndFill::configStringToReference(const QString &configString) const
{
    if (configString == "allLayers") {
        return AllLayers;
    } else if (configString == "colorLabeledLayers") {
        return ColorLabeledLayers;
    }
    return CurrentLayer;
}

void KisToolEncloseAndFill::slot_optionButtonStripEnclosingMethod_buttonToggled(
    KoGroupButton *button,
    bool checked)
{
    if (!checked) {
        return;
    }

    if (button == m_buttonEnclosingMethodRectangle) {
        m_enclosingMethod = Rectangle;
    } else if (button == m_buttonEnclosingMethodEllipse) {
        m_enclosingMethod = Ellipse;
    } else if (button == m_buttonEnclosingMethodPath) {
        m_enclosingMethod = Path;
    } else if (button == m_buttonEnclosingMethodLasso) {
        m_enclosingMethod = Lasso;
    } else {
        m_enclosingMethod = Brush;
    }

    saveEnclosingMethodToConfig(m_enclosingMethod);
    setupEnclosingSubtool();
}

void KisToolEncloseAndFill::slot_comboBoxRegionSelectionMethod_currentIndexChanged(int)
{
    m_regionSelectionMethod = static_cast<RegionSelectionMethod>(m_comboBoxRegionSelectionMethod->currentData().toInt());

    KisOptionCollectionWidgetWithHeader *sectionWhatToFill =
        m_optionWidget->widgetAs<KisOptionCollectionWidgetWithHeader*>("sectionWhatToFill");
    sectionWhatToFill->setWidgetVisible("buttonRegionSelectionColor",
        m_regionSelectionMethod == RegionSelectionMethod::SelectRegionsFilledWithSpecificColor ||
        m_regionSelectionMethod == RegionSelectionMethod::SelectRegionsFilledWithSpecificColorOrTransparent ||
        m_regionSelectionMethod == RegionSelectionMethod::SelectAllRegionsExceptFilledWithSpecificColor ||
        m_regionSelectionMethod == RegionSelectionMethod::SelectAllRegionsExceptFilledWithSpecificColorOrTransparent ||
        m_regionSelectionMethod == RegionSelectionMethod::SelectRegionsSurroundedBySpecificColor ||
        m_regionSelectionMethod == RegionSelectionMethod::SelectRegionsSurroundedBySpecificColorOrTransparent
    );
    sectionWhatToFill->setWidgetVisible(
        "checkBoxRegionSelectionIncludeContourRegions",
        m_regionSelectionMethod == RegionSelectionMethod::SelectAllRegions ||
        m_regionSelectionMethod == RegionSelectionMethod::SelectRegionsFilledWithSpecificColor ||
        m_regionSelectionMethod == RegionSelectionMethod::SelectRegionsFilledWithTransparent ||
        m_regionSelectionMethod == RegionSelectionMethod::SelectRegionsFilledWithSpecificColorOrTransparent ||
        m_regionSelectionMethod == RegionSelectionMethod::SelectAllRegionsExceptFilledWithSpecificColor ||
        m_regionSelectionMethod == RegionSelectionMethod::SelectAllRegionsExceptFilledWithTransparent ||
        m_regionSelectionMethod == RegionSelectionMethod::SelectAllRegionsExceptFilledWithSpecificColorOrTransparent
    );

    m_comboBoxRegionSelectionMethod->setToolTip(m_comboBoxRegionSelectionMethod->currentText());

    saveRegionSelectionMethodToConfig(m_regionSelectionMethod);
}

void KisToolEncloseAndFill::slot_buttonRegionSelectionColor_changed(const KoColor &color)
{
    if (color == m_regionSelectionColor) {
        return;
    }
    m_regionSelectionColor = color;
    m_configGroup.writeEntry("regionSelectionColor", color.toXML());
}

void KisToolEncloseAndFill::slot_checkBoxRegionSelectionInvert_toggled(bool checked)
{
    if (checked == m_regionSelectionInvert) {
        return;
    }
    m_regionSelectionInvert = checked;
    m_configGroup.writeEntry("regionSelectionInvert", checked);
}

void KisToolEncloseAndFill::slot_checkBoxRegionSelectionIncludeContourRegions_toggled(bool checked)
{
    if (checked == m_regionSelectionIncludeContourRegions) {
        return;
    }
    m_regionSelectionIncludeContourRegions = checked;
    m_configGroup.writeEntry("regionSelectionIncludeContourRegions", checked);
}

void KisToolEncloseAndFill::slot_optionButtonStripFillWith_buttonToggled(
    KoGroupButton *button,
    bool checked)
{
    if (!checked) {
        return;
    }
    const bool visible = button == m_buttonFillWithPattern;
    KisOptionCollectionWidgetWithHeader *sectionFillWith =
        m_optionWidget->widgetAs<KisOptionCollectionWidgetWithHeader*>("sectionFillWith");
    sectionFillWith->setWidgetVisible("sliderPatternScale", visible);
    sectionFillWith->setWidgetVisible("angleSelectorPatternRotation", visible);
    
    m_fillType = button == m_buttonFillWithFG ? FillWithForegroundColor
                                              : (button == m_buttonFillWithBG ? FillWithBackgroundColor : FillWithPattern);

    m_configGroup.writeEntry(
        "fillWith",
        button == m_buttonFillWithFG ? "foregroundColor" : (button == m_buttonFillWithBG ? "backgroundColor" : "pattern")
    );
}

void KisToolEncloseAndFill::slot_sliderPatternScale_valueChanged(double value)
{
    if (value == m_patternScale) {
        return;
    }
    m_patternScale = value;
    m_configGroup.writeEntry("patternScale", value);
}

void KisToolEncloseAndFill::slot_angleSelectorPatternRotation_angleChanged(double value)
{
    if (value == m_patternRotation) {
        return;
    }
    m_patternRotation = value;
    m_configGroup.writeEntry("patternRotate", value);
}

void KisToolEncloseAndFill::slot_checkBoxUseCustomBlendingOptions_toggled(bool checked)
{
    KisOptionCollectionWidgetWithHeader *sectionFillWith =
        m_optionWidget->widgetAs<KisOptionCollectionWidgetWithHeader*>("sectionFillWith");
    sectionFillWith->setWidgetVisible("sliderCustomOpacity", checked);
    sectionFillWith->setWidgetVisible("comboBoxCustomCompositeOp", checked);
    m_useCustomBlendingOptions = checked;
    m_configGroup.writeEntry("useCustomBlendingOptions", checked);
}

void KisToolEncloseAndFill::slot_sliderCustomOpacity_valueChanged(int value)
{
    if (value == m_customOpacity) {
        return;
    }
    m_customOpacity = value;
    m_configGroup.writeEntry("customOpacity", value);
}

void KisToolEncloseAndFill::slot_comboBoxCustomCompositeOp_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    const QString compositeOpId = m_comboBoxCustomCompositeOp->selectedCompositeOp().id();
    if (compositeOpId == m_customCompositeOp) {
        return;
    }
    m_customCompositeOp = compositeOpId;
    m_configGroup.writeEntry("customCompositeOp", compositeOpId);
}

void KisToolEncloseAndFill::slot_sliderFillThreshold_valueChanged(int value)
{
    if (value == m_fillThreshold) {
        return;
    }
    m_fillThreshold = value;
    m_configGroup.writeEntry("fillThreshold", value);
}

void KisToolEncloseAndFill::slot_sliderFillOpacitySpread_valueChanged(int value)
{
    if (value == m_fillOpacitySpread) {
        return;
    }
    m_fillOpacitySpread = value;
    m_configGroup.writeEntry("fillOpacitySpread", value);
}

void KisToolEncloseAndFill::slot_sliderCloseGap_valueChanged(int value)
{
    if (value == m_closeGap) {
        return;
    }
    m_closeGap = value;
    m_configGroup.writeEntry("closeGapAmount", value);
}

void KisToolEncloseAndFill::slot_checkBoxSelectionAsBoundary_toggled(bool checked)
{
    if (checked == m_useSelectionAsBoundary) {
        return;
    }
    m_useSelectionAsBoundary = checked;
    m_configGroup.writeEntry("useSelectionAsBoundary", checked);
}

void KisToolEncloseAndFill::slot_checkBoxAntiAlias_toggled(bool checked)
{
    if (checked == m_antiAlias) {
        return;
    }
    m_antiAlias = checked;
    m_configGroup.writeEntry("antiAlias", checked);
}

void KisToolEncloseAndFill::slot_sliderExpand_valueChanged(int value)
{
    if (value == m_expand) {
        return;
    }
    m_expand = value;
    m_configGroup.writeEntry("expand", value);
}

void KisToolEncloseAndFill::slot_buttonStopGrowingAtDarkestPixel_toggled(bool enabled)
{
    if (enabled == m_stopGrowingAtDarkestPixel) {
        return;
    }
    m_stopGrowingAtDarkestPixel = enabled;
    m_configGroup.writeEntry("stopGrowingAtDarkestPixel", enabled);
}

void KisToolEncloseAndFill::slot_sliderFeather_valueChanged(int value)
{
    if (value == m_feather) {
        return;
    }
    m_feather = value;
    m_configGroup.writeEntry("feather", value);
}

void KisToolEncloseAndFill::slot_optionButtonStripReference_buttonToggled(
    KoGroupButton *button,
    bool checked)
{
    if (!checked) {
        return;
    }
    KisOptionCollectionWidgetWithHeader *sectionReference =
        m_optionWidget->widgetAs<KisOptionCollectionWidgetWithHeader*>("sectionReference");
    sectionReference->setWidgetVisible("widgetLabels", button == m_buttonReferenceLabeled);
    
    m_reference = button == m_buttonReferenceCurrent ? CurrentLayer
                                                     : (button == m_buttonReferenceAll ? AllLayers : ColorLabeledLayers);

    m_configGroup.writeEntry(
        "reference",
        button == m_buttonReferenceCurrent ? "currentLayer" : (button == m_buttonReferenceAll ? "allLayers" : "colorLabeledLayers")
    );
}

void KisToolEncloseAndFill::slot_widgetLabels_selectionChanged()
{
    QList<int> labels = m_widgetLabels->selection();
    if (labels == m_selectedColorLabels) {
        return;
    }
    m_selectedColorLabels = labels;
    if (labels.isEmpty()) {
        return;
    }
    QString colorLabels = QString::number(labels.first());
    for (int i = 1; i < labels.size(); ++i) {
        colorLabels += "," + QString::number(labels[i]);
    }
    m_configGroup.writeEntry("colorLabels", colorLabels);
}

void KisToolEncloseAndFill::slot_buttonReset_clicked()
{
    m_buttonEnclosingMethodLasso->setChecked(true);
    m_comboBoxRegionSelectionMethod->setCurrentIndex(
        m_comboBoxRegionSelectionMethod->findData(static_cast<int>(RegionSelectionMethod::SelectAllRegions))
    );
    m_buttonRegionSelectionColor->setColor(KoColor());
    m_checkBoxRegionSelectionInvert->setChecked(false);
    m_checkBoxRegionSelectionIncludeContourRegions->setChecked(false);
    m_buttonFillWithFG->setChecked(true);
    m_sliderPatternScale->setValue(100.0);
    m_angleSelectorPatternRotation->setAngle(0.0);
    m_checkBoxCustomBlendingOptions->setChecked(false);
    m_sliderCustomOpacity->setValue(100);
    m_comboBoxCustomCompositeOp->selectCompositeOp(KoID(COMPOSITE_OVER));
    m_sliderFillThreshold->setValue(8);
    m_sliderFillOpacitySpread->setValue(100);
    m_sliderCloseGap->setValue(0);
    m_checkBoxSelectionAsBoundary->setChecked(true);
    m_checkBoxAntiAlias->setChecked(false);
    m_sliderExpand->setValue(0);
    m_buttonStopGrowingAtDarkestPixel->setChecked(false);
    m_sliderFeather->setValue(0);
    m_buttonReferenceCurrent->setChecked(true);
    m_widgetLabels->setSelection({});
}

void KisToolEncloseAndFill::slot_currentNodeChanged(const KisNodeSP node)
{
    if (m_previousNode && m_previousNode->paintDevice()) {
        disconnect(m_previousNode->paintDevice().data(),
                   SIGNAL(colorSpaceChanged(const KoColorSpace*)),
                   this,
                   SLOT(slot_colorSpaceChanged(const KoColorSpace*)));
    }
    if (node && node->paintDevice()) {
        connect(node->paintDevice().data(),
                SIGNAL(colorSpaceChanged(const KoColorSpace*)),
                this,
                SLOT(slot_colorSpaceChanged(const KoColorSpace*)));
        slot_colorSpaceChanged(node->paintDevice()->colorSpace());
    }
    m_previousNode = node;
}

void KisToolEncloseAndFill::slot_colorSpaceChanged(const KoColorSpace *colorSpace)
{
    if (!m_comboBoxCustomCompositeOp) {
        return;
    }
    const KoColorSpace *compositionSpace = colorSpace;
    if (currentNode() && currentNode()->paintDevice()) {
        // Currently, composition source is enough to determine the available blending mode,
        // because either destination is the same (paint layers), or composition happens
        // in source space (masks).
        compositionSpace = currentNode()->paintDevice()->compositionSourceColorSpace();
    }
    m_comboBoxCustomCompositeOp->validate(compositionSpace);
}

void KisToolEncloseAndFill::slot_checkBoxUseActiveLayer_toggled(bool checked)
{
    if (checked == m_useActiveLayer) {
        return;
    }
    m_useActiveLayer = checked;
    m_configGroup.writeEntry("useActiveLayer", checked);
}
