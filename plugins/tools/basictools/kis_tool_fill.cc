/*
*  kis_tool_fill.cc - part of Krayon
*
*  SPDX-FileCopyrightText: 2000 John Califf <jcaliff@compuzone.net>
*  SPDX-FileCopyrightText: 2004 Boudewijn Rempt <boud@valdyas.org>
*  SPDX-FileCopyrightText: 2004 Bart Coppens <kde@bartcoppens.be>
*
*  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kis_tool_fill.h"

#include <kis_debug.h>
#include <klocalizedstring.h>

#include <QSlider>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>

#include <KisOptionButtonStrip.h>
#include <KisOptionCollectionWidget.h>
#include <KoGroupButton.h>

#include <ksharedconfig.h>

#include <KoCanvasBase.h>
#include <KoPointerEvent.h>

#include <kis_layer.h>
#include <resources/KoPattern.h>
#include <kis_selection.h>

#include <KisViewManager.h>
#include <canvas/kis_canvas2.h>
#include <widgets/kis_cmb_composite.h>
#include <kis_slider_spin_box.h>
#include <kis_canvas_resource_provider.h>
#include <kis_cursor.h>
#include <kis_color_filter_combo.h>
#include <KisAngleSelector.h>
#include <kis_color_label_selector_widget.h>
#include <kis_color_button.h>
#include <kis_cmb_composite.h>

#include <processing/fill_processing_visitor.h>
#include <kis_command_utils.h>
#include <kis_layer_utils.h>
#include <krita_utils.h>
#include <kis_stroke_strategy_undo_command_based.h>
#include <commands_new/kis_processing_command.h>
#include <commands_new/kis_update_command.h>
#include <kis_fill_painter.h>
#include <kis_selection_filters.h>

#include <KisPart.h>
#include <KisDocument.h>
#include <kis_dummies_facade.h>
#include <KoShapeControllerBase.h>
#include <kis_shape_controller.h>
#include <kis_image_animation_interface.h>
#include <kis_canvas_resource_provider.h>
#include <KisSpinBoxI18nHelper.h>

#include "kis_icon_utils.h"

KisToolFill::KisToolFill(KoCanvasBase * canvas)
    : KisToolPaint(canvas, KisCursor::load("tool_fill_cursor.png", 6, 6))
    , m_fillMask(nullptr)
    , m_referencePaintDevice(nullptr)
    , m_referenceNodeList(nullptr)
    , m_previousTime(0)
    , m_compressorFillUpdate(150, KisSignalCompressor::FIRST_ACTIVE)
    , m_dirtyRect(nullptr)
    , m_fillStrokeId(nullptr)
{
    setObjectName("tool_fill");
    connect(&m_compressorFillUpdate, SIGNAL(timeout()), SLOT(slotUpdateFill()));

    KisCanvas2 *kritaCanvas = dynamic_cast<KisCanvas2*>(canvas);

    connect(kritaCanvas->viewManager()->canvasResourceProvider(), SIGNAL(sigEffectiveCompositeOpChanged()), SLOT(resetCursorStyle()));
}

KisToolFill::~KisToolFill()
{
}

void KisToolFill::resetCursorStyle()
{
    if (isEraser() && !m_useCustomBlendingOptions) {
        useCursor(KisCursor::load("tool_fill_eraser_cursor.png", 6, 6));
    } else {
        KisToolPaint::resetCursorStyle();
    }

    overrideCursorIfNotEditable();
}

void KisToolFill::activate(const QSet<KoShape*> &shapes)
{
    KisToolPaint::activate(shapes);
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

void KisToolFill::deactivate()
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
    KisToolPaint::deactivate();
}

void KisToolFill::beginPrimaryAction(KoPointerEvent *event)
{
    // cannot use fill tool on non-painting layers.
    // this logic triggers with multiple layer types like vector layer, clone layer, file layer, group layer
    if (currentNode().isNull() || currentNode()->inherits("KisShapeLayer") || nodePaintAbility()!=NodePaintAbility::PAINT ) {
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

    m_fillStartWidgetPosition = event->pos();
    const QPoint lastImagePosition = convertToImagePixelCoordFloored(event);

    if (!currentNode() ||
        (!image()->wrapAroundModePermitted() &&
         !image()->bounds().contains(lastImagePosition))) {
        return;
    }
    
    // Switch the fill mode if shift or alt modifiers are pressed
    if (event->modifiers() == Qt::ShiftModifier) {
        if (m_fillMode == FillMode_FillSimilarRegions) {
            m_effectiveFillMode = FillMode_FillSelection;
        } else {
            m_effectiveFillMode = FillMode_FillSimilarRegions;
        }
    } else if (event->modifiers() == Qt::AltModifier) {
        if (m_fillMode == FillMode_FillContiguousRegion) {
            m_effectiveFillMode = FillMode_FillSelection;
        } else {
            m_effectiveFillMode = FillMode_FillContiguousRegion;
        }
    } else {
        m_effectiveFillMode = m_fillMode;
    }

    m_seedPoints.append(lastImagePosition);
    beginFilling(lastImagePosition);
    m_isFilling = true;

    slotUpdateFill();
}

void KisToolFill::continuePrimaryAction(KoPointerEvent *event)
{
    if (!m_isFilling || m_effectiveFillMode != FillMode_FillContiguousRegion ||
        m_continuousFillMode == ContinuousFillMode_DoNotUse) {
        return;
    }
    
    if (!m_isDragging) {
        const int dragDistanceSquared =
            pow2(event->pos().x() - m_fillStartWidgetPosition.x()) +
            pow2(event->pos().y() - m_fillStartWidgetPosition.y());

        if (dragDistanceSquared < minimumDragDistanceSquared) {
            return;
        }

        m_isDragging = true;
    }

    const QPoint newImagePosition = convertToImagePixelCoordFloored(event);
    m_seedPoints.append(newImagePosition);

    m_compressorFillUpdate.start();
}

void KisToolFill::endPrimaryAction(KoPointerEvent *)
{
    if (m_isFilling) {
        m_compressorFillUpdate.stop();
        endFilling();
    }

    m_isFilling = false;
    m_isDragging = false;
    m_seedPoints.clear();
}

void KisToolFill::beginAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (action == ChangeSize) {
        beginPrimaryAction(event);
        return;
    }
    KisToolPaint::beginAlternateAction(event, action);
}

void KisToolFill::continueAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (action == ChangeSize) {
        continuePrimaryAction(event);
        return;
    }
    KisToolPaint::continueAlternateAction(event, action);
}

void KisToolFill::endAlternateAction(KoPointerEvent *event, AlternateAction action)
{
    if (action == ChangeSize) {
        endPrimaryAction(event);
        return;
    }
    KisToolPaint::endAlternateAction(event, action);
}

void KisToolFill::beginFilling(const QPoint &seedPoint)
{
    setMode(KisTool::PAINT_MODE);

    KisStrokeStrategyUndoCommandBased *strategy =
            new KisStrokeStrategyUndoCommandBased(kundo2_i18n("Flood Fill"), false, image().data());
    strategy->setSupportsWrapAroundMode(true);
    m_fillStrokeId = image()->startStroke(strategy);
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_fillStrokeId);

    m_resourcesSnapshot = new KisResourcesSnapshot(image(), currentNode(), this->canvas()->resourceManager());

    KisPaintDeviceSP referencePaintDevice = nullptr;
    if (m_effectiveFillMode != FillMode_FillSelection) {
        if (m_reference == Reference_CurrentLayer) {
            referencePaintDevice = currentNode()->paintDevice();
        } else if (m_reference == Reference_AllLayers) {
            referencePaintDevice = currentImage()->projection();
        } else if (m_reference == Reference_ColorLabeledLayers) {
            if (!m_referenceNodeList) {
                referencePaintDevice = KisMergeLabeledLayersCommand::createRefPaintDevice(image(), "Fill Tool Reference Result Paint Device");
                m_referenceNodeList.reset(new KisMergeLabeledLayersCommand::ReferenceNodeInfoList);
            } else {
                referencePaintDevice = m_referencePaintDevice;
            }
            KisPaintDeviceSP newReferencePaintDevice = KisMergeLabeledLayersCommand::createRefPaintDevice(image(), "Fill Tool Reference Result Paint Device");
            KisMergeLabeledLayersCommand::ReferenceNodeInfoListSP newReferenceNodeList(new KisMergeLabeledLayersCommand::ReferenceNodeInfoList);
            const int currentTime = image()->animationInterface()->currentTime();
            image()->addJob(
                m_fillStrokeId,
                new KisStrokeStrategyUndoCommandBased::Data(
                    KUndo2CommandSP(new KisMergeLabeledLayersCommand(
                        image(),
                        m_referenceNodeList,
                        newReferenceNodeList,
                        referencePaintDevice,
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
            referencePaintDevice = newReferencePaintDevice;
            m_referenceNodeList = newReferenceNodeList;
            m_previousTime = currentTime;
        }

        QSharedPointer<KoColor> referenceColor(new KoColor);
        if (m_reference == Reference_ColorLabeledLayers) {
            // We need to obtain the reference color from the reference paint
            // device, but it is produced in a stroke, so we must get the color
            // after the device is ready. So we get it in the stroke
            image()->addJob(
                m_fillStrokeId,
                new KisStrokeStrategyUndoCommandBased::Data(
                    KUndo2CommandSP(new KisCommandUtils::LambdaCommand(
                        [referencePaintDevice, referenceColor, seedPoint]() -> KUndo2Command*
                        {
                            *referenceColor = referencePaintDevice->pixel(seedPoint);
                            return 0;
                        }
                    )),
                    false,
                    KisStrokeJobData::SEQUENTIAL,
                    KisStrokeJobData::EXCLUSIVE
                )
            );
        } else {
            // Here the reference device is already ready, so we obtain the
            // reference color directly
            *referenceColor = referencePaintDevice->pixel(seedPoint);
            // Reset this so that the device from color labeled layers gets
            // regenerated when that mode is selected again
            m_referenceNodeList.reset();
        }

        m_referencePaintDevice = referencePaintDevice;
        m_referenceColor = referenceColor;

        m_fillMask = new KisSelection;
    }

    m_dirtyRect.reset(new QRect);
    m_transform.reset();
    m_transform.rotate(m_patternRotation);
    const qreal normalizedScale = m_patternScale * 0.01;
    m_transform.scale(normalizedScale, normalizedScale);
    m_resourcesSnapshot->setFillTransform(m_transform);
}

void KisToolFill::addFillingOperation(const QPoint &seedPoint)
{
    const QVector<QPoint> seedPoints({seedPoint});
    addFillingOperation(seedPoints);
}

void KisToolFill::addFillingOperation(const QVector<QPoint> &seedPoints)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_fillStrokeId);

    const qreal customOpacity = m_customOpacity / 100.0;

    if (m_effectiveFillMode != FillMode_FillSimilarRegions) {
        FillProcessingVisitor *visitor =  new FillProcessingVisitor(m_referencePaintDevice,
                                                                    m_resourcesSnapshot->activeSelection(),
                                                                    m_resourcesSnapshot);

        const bool blendingOptionsAreNoOp = m_useCustomBlendingOptions
                                            ? (qFuzzyCompare(customOpacity, OPACITY_OPAQUE_F) &&
                                               m_customCompositeOp == COMPOSITE_OVER)
                                            : (qFuzzyCompare(m_resourcesSnapshot->opacity(), OPACITY_OPAQUE_F) &&
                                               m_resourcesSnapshot->compositeOpId() == COMPOSITE_OVER);

        const bool useFastMode = !m_resourcesSnapshot->activeSelection() &&
                                 blendingOptionsAreNoOp &&
                                 m_fillType != FillType_FillWithPattern &&
                                 m_opacitySpread == 100 &&
                                 m_useSelectionAsBoundary == false &&
                                 !m_antiAlias && m_sizemod == 0 && m_feather == 0 &&
                                 m_closeGap == 0 &&
                                 m_reference == Reference_CurrentLayer;

        visitor->setSeedPoints(seedPoints);
        visitor->setUseFastMode(useFastMode);
        visitor->setSelectionOnly(m_effectiveFillMode == FillMode_FillSelection);
        visitor->setUseBgColor(m_fillType == FillType_FillWithBackgroundColor);
        visitor->setUsePattern(m_fillType == FillType_FillWithPattern);
        visitor->setUseCustomBlendingOptions(m_useCustomBlendingOptions);
        if (m_useCustomBlendingOptions) {
            visitor->setCustomOpacity(customOpacity);
            visitor->setCustomCompositeOp(m_customCompositeOp);
        }
        visitor->setRegionFillingMode(
            m_contiguousFillMode == ContiguousFillMode_FloodFill
            ? KisFillPainter::RegionFillingMode_FloodFill
            : KisFillPainter::RegionFillingMode_BoundaryFill
        );
        if (m_contiguousFillMode == ContiguousFillMode_BoundaryFill) {
            visitor->setRegionFillingBoundaryColor(m_contiguousFillBoundaryColor);
        }
        visitor->setFillThreshold(m_threshold);
        visitor->setOpacitySpread(m_opacitySpread);
        visitor->setCloseGap(m_closeGap);
        visitor->setUseSelectionAsBoundary(m_useSelectionAsBoundary);
        visitor->setAntiAlias(m_antiAlias);
        visitor->setSizeMod(m_sizemod);
        visitor->setStopGrowingAtDarkestPixel(m_stopGrowingAtDarkestPixel);
        visitor->setFeather(m_feather);
        if (m_isDragging) {
            visitor->setContinuousFillMode(
                m_continuousFillMode == ContinuousFillMode_FillAnyRegion
                ? FillProcessingVisitor::ContinuousFillMode_FillAnyRegion
                : FillProcessingVisitor::ContinuousFillMode_FillSimilarRegions
            );
            visitor->setContinuousFillMask(m_fillMask);
            visitor->setContinuousFillReferenceColor(m_referenceColor);
        }
        visitor->setOutDirtyRect(m_dirtyRect);

        image()->addJob(
            m_fillStrokeId,
            new KisStrokeStrategyUndoCommandBased::Data(
                KUndo2CommandSP(new KisProcessingCommand(visitor, currentNode())),
                false,
                KisStrokeJobData::SEQUENTIAL,
                KisStrokeJobData::EXCLUSIVE
            )
        );
    } else {
        KisSelectionSP fillMask = m_fillMask;
        QSharedPointer<KisProcessingVisitor::ProgressHelper>
            progressHelper(new KisProcessingVisitor::ProgressHelper(currentNode()));

        {
            KisSelectionSP selection = m_resourcesSnapshot->activeSelection();
            KisFillPainter painter;
            QRect bounds = currentImage()->bounds();
            if (selection) {
                bounds = bounds.intersected(selection->projection()->selectedRect());
            }

            painter.setFillThreshold(m_threshold);
            painter.setOpacitySpread(m_opacitySpread);
            painter.setAntiAlias(m_antiAlias);
            painter.setSizemod(m_sizemod);
            painter.setStopGrowingAtDarkestPixel(m_stopGrowingAtDarkestPixel);
            painter.setFeather(m_feather);

            QVector<KisStrokeJobData*> jobs =
                painter.createSimilarColorsSelectionJobs(
                    fillMask->pixelSelection(), m_referenceColor, m_referencePaintDevice,
                    bounds, selection ? selection->projection() : nullptr, progressHelper
                );

            for (KisStrokeJobData *job : jobs) {
                image()->addJob(m_fillStrokeId, job);
            }
        }

        {
            FillProcessingVisitor *visitor =  new FillProcessingVisitor(nullptr,
                                                                        fillMask,
                                                                        m_resourcesSnapshot);

            visitor->setSeedPoints(seedPoints);
            visitor->setSelectionOnly(true);
            visitor->setUseBgColor(m_fillType == FillType_FillWithBackgroundColor);
            visitor->setUsePattern(m_fillType == FillType_FillWithPattern);
            visitor->setUseCustomBlendingOptions(m_useCustomBlendingOptions);
            if (m_useCustomBlendingOptions) {
                visitor->setCustomOpacity(customOpacity);
                visitor->setCustomCompositeOp(m_customCompositeOp);
            }
            visitor->setOutDirtyRect(m_dirtyRect);
            visitor->setProgressHelper(progressHelper);

            image()->addJob(
                m_fillStrokeId,
                new KisStrokeStrategyUndoCommandBased::Data(
                    KUndo2CommandSP(new KisProcessingCommand(visitor, currentNode())),
                    false,
                    KisStrokeJobData::SEQUENTIAL,
                    KisStrokeJobData::EXCLUSIVE
                )
            );
        }
    }
}

void KisToolFill::addUpdateOperation()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_fillStrokeId);

    image()->addJob(
        m_fillStrokeId,
        new KisStrokeStrategyUndoCommandBased::Data(
            KUndo2CommandSP(new KisUpdateCommand(currentNode(), m_dirtyRect, image().data())),
            false,
            KisStrokeJobData::SEQUENTIAL,
            KisStrokeJobData::EXCLUSIVE
        )
    );
}

void KisToolFill::endFilling()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_fillStrokeId);
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);

    setMode(KisTool::HOVER_MODE);
    image()->endStroke(m_fillStrokeId);
    m_fillStrokeId = nullptr;
    m_fillMask = nullptr;
    m_dirtyRect = nullptr;
}

void KisToolFill::slotUpdateFill()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_fillStrokeId);

    if (m_effectiveFillMode == FillMode_FillContiguousRegion) {
        addFillingOperation(KritaUtils::rasterizePolylineDDA(m_seedPoints));
        // clear to not re-add the segments, but retain the last point to maintain continuity
        m_seedPoints = {m_seedPoints.last()};
    } else {
        addFillingOperation(m_seedPoints.last());
    }
    addUpdateOperation();
}

QWidget* KisToolFill::createOptionWidget()
{
    loadConfiguration();

    // Create widgets
    KisOptionButtonStrip *optionButtonStripWhatToFill =
        new KisOptionButtonStrip;
    m_buttonWhatToFillSelection = optionButtonStripWhatToFill->addButton(
        KisIconUtils::loadIcon("tool_outline_selection"));
    m_buttonWhatToFillContiguous = optionButtonStripWhatToFill->addButton(
        KisIconUtils::loadIcon("contiguous-selection"));
    m_buttonWhatToFillSimilar = optionButtonStripWhatToFill->addButton(
        KisIconUtils::loadIcon("similar-selection"));
    m_buttonWhatToFillContiguous->setChecked(true);

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
    KisSpinBoxI18nHelper::setText(
        m_sliderPatternScale,
        i18nc("The pattern 'scale' spinbox in fill tool options; {n} is the number value, % is the percent sign",
              "Scale: {n}%"));
    m_angleSelectorPatternRotation = new KisAngleSelector;
    m_angleSelectorPatternRotation->setFlipOptionsMode(KisAngleSelector::FlipOptionsMode_ContextMenu);
    m_angleSelectorPatternRotation->setIncreasingDirection(KisAngleGauge::IncreasingDirection_Clockwise);
    m_checkBoxCustomBlendingOptions = new QCheckBox(i18n("Use custom blending options"));
    m_sliderCustomOpacity = new KisSliderSpinBox;
    m_sliderCustomOpacity->setRange(0, 100);
    KisSpinBoxI18nHelper::setText(m_sliderCustomOpacity,
                                  i18nc("{n} is the number value, % is the percent sign", "Opacity: {n}%"));
    m_comboBoxCustomCompositeOp = new KisCompositeOpComboBox;

    KisOptionButtonStrip *optionButtonStripContiguousFillMode = new KisOptionButtonStrip;
    m_buttonContiguousFillModeFloodFill = optionButtonStripContiguousFillMode->addButton(
        KisIconUtils::loadIcon("region-filling-flood-fill"));
    m_buttonContiguousFillModeBoundaryFill = optionButtonStripContiguousFillMode->addButton(
        KisIconUtils::loadIcon("region-filling-boundary-fill"));
    m_buttonContiguousFillModeFloodFill->setChecked(true);
    m_buttonContiguousFillBoundaryColor = new KisColorButton;
    m_sliderThreshold = new KisSliderSpinBox;
    m_sliderThreshold->setPrefix(i18nc("The 'threshold' spinbox prefix in fill tool options", "Threshold: "));
    m_sliderThreshold->setRange(1, 100);
    m_sliderSpread = new KisSliderSpinBox;
    m_sliderSpread->setRange(0, 100);
    KisSpinBoxI18nHelper::setText(
        m_sliderSpread,
        i18nc("The 'spread' spinbox in fill tool options; {n} is the number value, % is the percent sign",
              "Spread: {n}%"));

    m_sliderCloseGap = new KisSliderSpinBox;
    m_sliderCloseGap->setPrefix(i18nc("The 'close gap' spinbox prefix in fill tool options", "Close Gap: "));
    m_sliderCloseGap->setRange(0, 32);
    m_sliderCloseGap->setSuffix(i18n(" px"));

    m_checkBoxSelectionAsBoundary =
        new QCheckBox(
            i18nc("The 'use selection as boundary' checkbox in fill tool to use selection borders as boundary when filling",
                  "Use selection as boundary")
        );
    m_checkBoxSelectionAsBoundary->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

    m_checkBoxAntiAlias = new QCheckBox(i18nc("The anti-alias checkbox in fill tool options", "Anti-aliasing"));

    KisOptionCollectionWidget *containerGrow = new KisOptionCollectionWidget;
    m_sliderGrow = new KisSliderSpinBox;
    m_sliderGrow->setPrefix(i18nc("The 'grow/shrink' spinbox prefix in fill tool options", "Grow: "));
    m_sliderGrow->setRange(-400, 400);
    m_sliderGrow->setSoftRange(-40, 40);
    m_sliderGrow->setSuffix(i18n(" px"));
    m_buttonStopGrowingAtDarkestPixel = new QToolButton;
    m_buttonStopGrowingAtDarkestPixel->setAutoRaise(true);
    m_buttonStopGrowingAtDarkestPixel->setCheckable(true);
    m_buttonStopGrowingAtDarkestPixel->setIcon(KisIconUtils::loadIcon("stop-at-boundary"));
    containerGrow->appendWidget("sliderGrow", m_sliderGrow);
    containerGrow->appendWidget("buttonStopGrowingAtDarkestPixel", m_buttonStopGrowingAtDarkestPixel);
    containerGrow->setOrientation(Qt::Horizontal);
    m_sliderFeather = new KisSliderSpinBox;
    m_sliderFeather->setPrefix(i18nc("The 'feather' spinbox prefix in fill tool options", "Feather: "));
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

    KisOptionButtonStrip *optionButtonStripDragFill =
        new KisOptionButtonStrip;
    m_buttonDragFillDoNotUse = optionButtonStripDragFill->addButton(
        KisIconUtils::loadIcon("dialog-cancel"));
    m_buttonDragFillAny = optionButtonStripDragFill->addButton(
        KisIconUtils::loadIcon("different-regions"));
    m_buttonDragFillSimilar = optionButtonStripDragFill->addButton(
        KisIconUtils::loadIcon("similar-regions"));
    m_buttonDragFillAny->setChecked(true);

    QPushButton *buttonReset = new QPushButton(i18nc("The 'reset' button in fill tool options", "Reset"));

    // Set the tooltips
    m_buttonWhatToFillSelection->setToolTip(i18n("Fill the active selection, or the entire canvas"));
    m_buttonWhatToFillContiguous->setToolTip(i18n("Fill a contiguous region"));
    m_buttonWhatToFillSimilar->setToolTip(i18n("Fill all regions of a similar color"));

    m_buttonFillWithFG->setToolTip(i18n("Foreground color"));
    m_buttonFillWithBG->setToolTip(i18n("Background color"));
    m_buttonFillWithPattern->setToolTip(i18n("Pattern"));
    m_sliderPatternScale->setToolTip(i18n("Set the scale of the pattern"));
    m_angleSelectorPatternRotation->setToolTip(i18n("Set the rotation of the pattern"));
    m_checkBoxCustomBlendingOptions->setToolTip(i18n("Set custom blending options instead of using the brush ones"));
    m_sliderCustomOpacity->setToolTip(i18n("Set a custom opacity for the fill"));
    m_comboBoxCustomCompositeOp->setToolTip(i18n("Set a custom blend mode for the fill"));

    m_buttonContiguousFillModeFloodFill->setToolTip(i18n("Fill regions similar in color to the clicked region"));
    m_buttonContiguousFillModeBoundaryFill->setToolTip(i18n("Fill all regions until a specific boundary color"));
    m_buttonContiguousFillBoundaryColor->setToolTip(i18n("Boundary color"));
    m_sliderThreshold->setToolTip(i18n("Set the color similarity tolerance of the fill. Increasing threshold increases the range of similar colors to be filled."));
    m_sliderSpread->setToolTip(i18n("Set the extent of the opaque portion of the fill. Decreasing spread decreases opacity of fill areas depending on color similarity."));
    m_sliderCloseGap->setToolTip(i18n("Close gaps in lines up to the set amount"));
    m_checkBoxSelectionAsBoundary->setToolTip(i18n("Set if the contour of the active selection should be treated as a boundary when filling the region"));

    m_checkBoxAntiAlias->setToolTip(i18n("Smooths the edges of the fill"));
    m_sliderGrow->setToolTip(i18n("Grow or shrink the fill by the set amount"));
    m_buttonStopGrowingAtDarkestPixel->setToolTip(i18n("Stop growing at the darkest and/or most opaque pixels"));
    m_sliderFeather->setToolTip(i18n("Blur the fill by the set amount"));

    m_buttonReferenceCurrent->setToolTip(i18n("Fill regions found from the active layer"));
    m_buttonReferenceAll->setToolTip(i18n("Fill regions found from the merging of all layers"));
    m_buttonReferenceLabeled->setToolTip(i18n("Fill regions found from the merging of layers with specific color labels"));
    m_checkBoxUseActiveLayer->setToolTip(i18n("Includes the active layer in regions found from merging of layers with specific color labels"));

    m_buttonDragFillDoNotUse->setToolTip(i18n("Dragging will not fill different regions"));
    m_buttonDragFillAny->setToolTip(i18n("Dragging will fill regions of any color"));
    m_buttonDragFillSimilar->setToolTip(i18n("Dragging will fill only regions similar in color to the initial region (useful for filling line-art)"));

    buttonReset->setToolTip(i18n("Reset the options to their default values"));

    // Construct the option widget
    m_optionWidget = new KisOptionCollectionWidget;
    m_optionWidget->setContentsMargins(0, 10, 0, 10);
    m_optionWidget->setSeparatorsVisible(true);

    KisOptionCollectionWidgetWithHeader *sectionWhatToFill =
        new KisOptionCollectionWidgetWithHeader(
            i18nc("The 'fill mode' section label in fill tool options", "Fill mode")
        );
    sectionWhatToFill->setPrimaryWidget(optionButtonStripWhatToFill);
    m_optionWidget->appendWidget("sectionWhatToFill", sectionWhatToFill);

    KisOptionCollectionWidgetWithHeader *sectionFillWith =
        new KisOptionCollectionWidgetWithHeader(
            i18nc("The 'fill source' section label in fill tool options", "Fill source")
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

    KisOptionCollectionWidget *widgetLabelsGroup = new KisOptionCollectionWidget;
    widgetLabelsGroup->appendWidget("labelWidget", m_widgetLabels);
    widgetLabelsGroup->appendWidget("checkBoxUseActiveLayer", m_checkBoxUseActiveLayer);
    widgetLabelsGroup->setWidgetsMargin(0);

    KisOptionCollectionWidgetWithHeader *sectionReference =
        new KisOptionCollectionWidgetWithHeader(
            i18nc("The 'reference' section label in fill tool options", "Reference")
        );
    sectionReference->setPrimaryWidget(optionButtonStripReference);
    sectionReference->appendWidget("widgetLabels", widgetLabelsGroup);
    sectionReference->setWidgetVisible("widgetLabels", false);
    m_optionWidget->appendWidget("sectionReference", sectionReference);

    KisOptionCollectionWidgetWithHeader *sectionRegionExtent =
        new KisOptionCollectionWidgetWithHeader(
            i18nc("The 'fill extent' section label in fill tool options", "Fill extent")
        );
    sectionRegionExtent->setPrimaryWidget(optionButtonStripContiguousFillMode);
    sectionRegionExtent->appendWidget("buttonContiguousFillBoundaryColor", m_buttonContiguousFillBoundaryColor);
    sectionRegionExtent->setWidgetVisible("buttonContiguousFillBoundaryColor", false);
    sectionRegionExtent->appendWidget("sliderThreshold", m_sliderThreshold);
    sectionRegionExtent->appendWidget("sliderSpread", m_sliderSpread);
    sectionRegionExtent->appendWidget("sliderCloseGap", m_sliderCloseGap);
    sectionRegionExtent->appendWidget("checkBoxSelectionAsBoundary", m_checkBoxSelectionAsBoundary);
    m_optionWidget->appendWidget("sectionRegionExtent", sectionRegionExtent);

    KisOptionCollectionWidgetWithHeader *sectionAdjustments =
        new KisOptionCollectionWidgetWithHeader(
            i18nc("The 'adjustments' section label in fill tool options", "Adjustments")
        );
    sectionAdjustments->appendWidget("checkBoxAntiAlias", m_checkBoxAntiAlias);
    sectionAdjustments->appendWidget("containerGrow", containerGrow);
    sectionAdjustments->appendWidget("sliderFeather", m_sliderFeather);
    m_optionWidget->appendWidget("sectionAdjustments", sectionAdjustments);

    KisOptionCollectionWidgetWithHeader *sectionDragFill =
        new KisOptionCollectionWidgetWithHeader(
            i18nc("The 'drag-fill mode' section label in fill tool options", "Drag-fill mode")
        );
    sectionDragFill->setPrimaryWidget(optionButtonStripDragFill);
    m_optionWidget->appendWidget("sectionDragFill", sectionDragFill);

    m_optionWidget->appendWidget("buttonReset", buttonReset);

    // Initialize widgets
    if (m_fillMode == FillMode_FillSelection) {
        m_buttonWhatToFillSelection->setChecked(true);
        m_optionWidget->setWidgetVisible("sectionRegionExtent", false);
        m_optionWidget->setWidgetVisible("sectionAdjustments", false);
        m_optionWidget->setWidgetVisible("sectionReference", false);
        m_optionWidget->setWidgetVisible("sectionDragFill", false);
    } else if (m_fillMode == FillMode_FillSimilarRegions) {
        m_buttonWhatToFillSimilar->setChecked(true);
        m_optionWidget->setWidgetVisible("sectionDragFill", false);
        sectionRegionExtent->setWidgetVisible("checkBoxSelectionAsBoundary", false);
        sectionRegionExtent->setWidgetVisible("sliderCloseGap", false);
    }
    sectionRegionExtent->setPrimaryWidgetVisible(m_fillMode == FillMode_FillContiguousRegion);
    if (m_fillType == FillType_FillWithBackgroundColor) {
        m_buttonFillWithBG->setChecked(true);
    } else if (m_fillType == FillType_FillWithPattern) {
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
    if (m_contiguousFillMode == ContiguousFillMode_BoundaryFill) {
        m_buttonContiguousFillModeBoundaryFill->setChecked(true);
        sectionRegionExtent->setWidgetVisible("buttonContiguousFillBoundaryColor",
                                              m_fillMode == FillMode_FillContiguousRegion);
    }
    m_buttonContiguousFillBoundaryColor->setColor(m_contiguousFillBoundaryColor);
    m_sliderThreshold->setValue(m_threshold);
    m_sliderSpread->setValue(m_opacitySpread);
    m_sliderCloseGap->setValue(m_closeGap);
    m_checkBoxSelectionAsBoundary->setChecked(m_useSelectionAsBoundary);
    m_checkBoxAntiAlias->setChecked(m_antiAlias);
    m_sliderGrow->setValue(m_sizemod);
    m_buttonStopGrowingAtDarkestPixel->setChecked(m_stopGrowingAtDarkestPixel);
    m_sliderFeather->setValue(m_feather);
    if (m_reference == Reference_AllLayers) {
        m_buttonReferenceAll->setChecked(true);
    } else if (m_reference == Reference_ColorLabeledLayers) {
        m_buttonReferenceLabeled->setChecked(true);
        sectionReference->setWidgetVisible("widgetLabels", true);
    }
    if (m_continuousFillMode == ContinuousFillMode_DoNotUse) {
        m_buttonDragFillDoNotUse->setChecked(true);
    } else if (m_continuousFillMode == ContinuousFillMode_FillSimilarRegions) {
        m_buttonDragFillSimilar->setChecked(true);
    }
    m_widgetLabels->setSelection(m_selectedColorLabels);
    m_checkBoxUseActiveLayer->setChecked(m_useActiveLayer);

    // Make connections
    connect(optionButtonStripWhatToFill,
            SIGNAL(buttonToggled(KoGroupButton *, bool)),
            SLOT(slot_optionButtonStripWhatToFill_buttonToggled(KoGroupButton *, bool)));
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
    connect(optionButtonStripContiguousFillMode,
            SIGNAL(buttonToggled(KoGroupButton *, bool)),
            SLOT(slot_optionButtonStripContiguousFillMode_buttonToggled(KoGroupButton *, bool)));
    connect(m_buttonContiguousFillBoundaryColor,
            SIGNAL(changed(const KoColor&)),
            SLOT(slot_buttonContiguousFillBoundaryColor_changed(const KoColor&)));
    connect(m_sliderThreshold, SIGNAL(valueChanged(int)), SLOT(slot_sliderThreshold_valueChanged(int)));
    connect(m_sliderSpread, SIGNAL(valueChanged(int)), SLOT(slot_sliderSpread_valueChanged(int)));
    connect(m_sliderCloseGap, SIGNAL(valueChanged(int)), SLOT(slot_sliderCloseGap_valueChanged(int)));
    connect(m_checkBoxSelectionAsBoundary,
            SIGNAL(toggled(bool)),
            SLOT(slot_checkBoxSelectionAsBoundary_toggled(bool)));
    connect(m_checkBoxAntiAlias, SIGNAL(toggled(bool)), SLOT(slot_checkBoxAntiAlias_toggled(bool)));
    connect(m_sliderGrow, SIGNAL(valueChanged(int)), SLOT(slot_sliderGrow_valueChanged(int)));
    connect(m_buttonStopGrowingAtDarkestPixel,
            SIGNAL(toggled(bool)),
            SLOT(slot_buttonStopGrowingAtDarkestPixel_toggled(bool)));
    connect(m_sliderFeather, SIGNAL(valueChanged(int)), SLOT(slot_sliderFeather_valueChanged(int)));
    connect(optionButtonStripReference,
            SIGNAL(buttonToggled(KoGroupButton *, bool)),
            SLOT(slot_optionButtonStripReference_buttonToggled(KoGroupButton *, bool)));
    connect(m_widgetLabels, SIGNAL(selectionChanged()), SLOT(slot_widgetLabels_selectionChanged()));
    connect(m_checkBoxUseActiveLayer, SIGNAL(toggled(bool)), SLOT(slot_checkBoxUseActiveLayer_toggled(bool)));
    connect(
        optionButtonStripDragFill,
        SIGNAL(buttonToggled(KoGroupButton *, bool)),
        SLOT(slot_optionButtonStripDragFill_buttonToggled(KoGroupButton *, bool)));
    connect(buttonReset, SIGNAL(clicked()), SLOT(slot_buttonReset_clicked()));
    
    return m_optionWidget;
}

void KisToolFill::loadConfiguration()
{
    {
        const QString whatToFillStr = m_configGroup.readEntry<QString>("whatToFill", "");
        if (whatToFillStr == "fillSelection") {
            m_fillMode = FillMode_FillSelection;
        } else if (whatToFillStr == "fillContiguousRegion") {
            m_fillMode = FillMode_FillContiguousRegion;
        } else if (whatToFillStr == "fillSimilarRegions") {
            m_fillMode = FillMode_FillSimilarRegions;
        } else {
            if (m_configGroup.readEntry<bool>("fillSelection", false)) {
                m_fillMode = FillMode_FillSelection;
            } else {
                m_fillMode = FillMode_FillContiguousRegion;
            }
        }
    }
    {
        const QString fillTypeStr = m_configGroup.readEntry<QString>("fillWith", "");
        if (fillTypeStr == "foregroundColor") {
            m_fillType = FillType_FillWithForegroundColor;
        } else if (fillTypeStr == "backgroundColor") {
            m_fillType = FillType_FillWithBackgroundColor;
        } else if (fillTypeStr == "pattern") {
            m_fillType = FillType_FillWithPattern;
        } else {
            if (m_configGroup.readEntry<bool>("usePattern", false)) {
                m_fillType = FillType_FillWithPattern;
            } else {
                m_fillType = FillType_FillWithForegroundColor;
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
    {
        const QString contiguousFillModeStr = m_configGroup.readEntry<QString>("contiguousFillMode", "");
        m_contiguousFillMode = contiguousFillModeStr == "boundaryFill"
                               ? ContiguousFillMode_BoundaryFill
                               : ContiguousFillMode_FloodFill;
    }
    m_contiguousFillBoundaryColor = loadContiguousFillBoundaryColorFromConfig();
    m_threshold = m_configGroup.readEntry<int>("thresholdAmount", 8);
    m_opacitySpread = m_configGroup.readEntry<int>("opacitySpread", 100);
    m_closeGap = m_configGroup.readEntry<int>("closeGapAmount", 0);
    m_useSelectionAsBoundary = m_configGroup.readEntry<bool>("useSelectionAsBoundary", true);
    m_antiAlias = m_configGroup.readEntry<bool>("antiAlias", false);
    m_sizemod = m_configGroup.readEntry<int>("growSelection", 0);
    m_stopGrowingAtDarkestPixel = m_configGroup.readEntry<bool>("stopGrowingAtDarkestPixel", false);
    m_feather = m_configGroup.readEntry<int>("featherAmount", 0);
    {
        const QString sampleLayersModeStr = m_configGroup.readEntry<QString>("sampleLayersMode", "");
        if (sampleLayersModeStr == "currentLayer") {
            m_reference = Reference_CurrentLayer;
        } else if (sampleLayersModeStr == "allLayers") {
            m_reference = Reference_AllLayers;
        } else if (sampleLayersModeStr == "colorLabeledLayers") {
            m_reference = Reference_ColorLabeledLayers;
        } else {
            if (m_configGroup.readEntry<bool>("sampleMerged", false)) {
                m_reference = Reference_AllLayers;
            } else {
                m_reference = Reference_CurrentLayer;
            }
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
        m_useActiveLayer = m_configGroup.readEntry<bool>("useActiveLayer", false);
    }
    {
        const QString continuousFillModeStr = m_configGroup.readEntry<QString>("continuousFillMode", "fillAnyRegion");
        if (continuousFillModeStr == "doNotUse") {
            m_continuousFillMode = ContinuousFillMode_DoNotUse;
        } else if (continuousFillModeStr == "fillSimilarRegions") {
            m_continuousFillMode = ContinuousFillMode_FillSimilarRegions;
        } else {
            m_continuousFillMode = ContinuousFillMode_FillAnyRegion;
        }
    }
}

KoColor KisToolFill::loadContiguousFillBoundaryColorFromConfig()
{
    const QString xmlColor = m_configGroup.readEntry("contiguousFillBoundaryColor", QString());
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

void KisToolFill::slot_optionButtonStripWhatToFill_buttonToggled(
    KoGroupButton *button,
    bool checked)
{
    if (!checked) {
        return;
    }

    if (button == m_buttonWhatToFillSelection) {
        m_optionWidget->setWidgetVisible("sectionRegionExtent", false);
        m_optionWidget->setWidgetVisible("sectionAdjustments", false);
        m_optionWidget->setWidgetVisible("sectionReference", false);
        m_optionWidget->setWidgetVisible("sectionDragFill", false);
        m_fillMode = FillMode_FillSelection;
        m_configGroup.writeEntry("whatToFill", "fillSelection");
    } else if (button == m_buttonWhatToFillContiguous) {
        m_optionWidget->setWidgetVisible("sectionRegionExtent", true);
        m_optionWidget->setWidgetVisible("sectionAdjustments", true);
        m_optionWidget->setWidgetVisible("sectionReference", true);
        m_optionWidget->setWidgetVisible("sectionDragFill", true);
        m_optionWidget->widgetAs<KisOptionCollectionWidgetWithHeader*>("sectionRegionExtent")
            ->setPrimaryWidgetVisible(true);
        m_optionWidget->widgetAs<KisOptionCollectionWidgetWithHeader*>("sectionRegionExtent")
            ->setWidgetVisible("buttonContiguousFillBoundaryColor", m_contiguousFillMode == ContiguousFillMode_BoundaryFill);
        m_optionWidget->widgetAs<KisOptionCollectionWidgetWithHeader*>("sectionRegionExtent")
            ->setWidgetVisible("sliderCloseGap", true);
        m_optionWidget->widgetAs<KisOptionCollectionWidgetWithHeader*>("sectionRegionExtent")
            ->setWidgetVisible("checkBoxSelectionAsBoundary", true);
        m_fillMode = FillMode_FillContiguousRegion;
        m_configGroup.writeEntry("whatToFill", "fillContiguousRegion");
    } else {
        m_optionWidget->setWidgetVisible("sectionRegionExtent", true);
        m_optionWidget->setWidgetVisible("sectionAdjustments", true);
        m_optionWidget->setWidgetVisible("sectionReference", true);
        m_optionWidget->setWidgetVisible("sectionDragFill", false);
        m_optionWidget->widgetAs<KisOptionCollectionWidgetWithHeader*>("sectionRegionExtent")
            ->setPrimaryWidgetVisible(false);
        m_optionWidget->widgetAs<KisOptionCollectionWidgetWithHeader*>("sectionRegionExtent")
            ->setWidgetVisible("buttonContiguousFillBoundaryColor", false);
        m_optionWidget->widgetAs<KisOptionCollectionWidgetWithHeader*>("sectionRegionExtent")
            ->setWidgetVisible("sliderCloseGap", false);
        m_optionWidget->widgetAs<KisOptionCollectionWidgetWithHeader*>("sectionRegionExtent")
            ->setWidgetVisible("checkBoxSelectionAsBoundary", false);
        m_fillMode = FillMode_FillSimilarRegions;
        m_configGroup.writeEntry("whatToFill", "fillSimilarRegions");
    }
}

void KisToolFill::slot_optionButtonStripFillWith_buttonToggled(
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
    
    m_fillType = button == m_buttonFillWithFG ? FillType_FillWithForegroundColor
                                              : (button == m_buttonFillWithBG
                                                 ? FillType_FillWithBackgroundColor
                                                 : FillType_FillWithPattern);

    m_configGroup.writeEntry(
        "fillWith",
        button == m_buttonFillWithFG
        ? "foregroundColor"
        : (button == m_buttonFillWithBG ? "backgroundColor" : "pattern")
    );
}

void KisToolFill::slot_sliderPatternScale_valueChanged(double value)
{
    if (value == m_patternScale) {
        return;
    }
    m_patternScale = value;
    m_configGroup.writeEntry("patternScale", value);
}

void KisToolFill::slot_angleSelectorPatternRotation_angleChanged(double value)
{
    if (value == m_patternRotation) {
        return;
    }
    m_patternRotation = value;
    m_configGroup.writeEntry("patternRotate", value);
}

void KisToolFill::slot_checkBoxUseCustomBlendingOptions_toggled(bool checked)
{
    KisOptionCollectionWidgetWithHeader *sectionFillWith =
        m_optionWidget->widgetAs<KisOptionCollectionWidgetWithHeader*>("sectionFillWith");
    sectionFillWith->setWidgetVisible("sliderCustomOpacity", checked);
    sectionFillWith->setWidgetVisible("comboBoxCustomCompositeOp", checked);
    m_useCustomBlendingOptions = checked;
    m_configGroup.writeEntry("useCustomBlendingOptions", checked);
}

void KisToolFill::slot_sliderCustomOpacity_valueChanged(int value)
{
    if (value == m_customOpacity) {
        return;
    }
    m_customOpacity = value;
    m_configGroup.writeEntry("customOpacity", value);
}

void KisToolFill::slot_comboBoxCustomCompositeOp_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    const QString compositeOpId = m_comboBoxCustomCompositeOp->selectedCompositeOp().id();
    if (compositeOpId == m_customCompositeOp) {
        return;
    }
    m_customCompositeOp = compositeOpId;
    m_configGroup.writeEntry("customCompositeOp", compositeOpId);
}

void KisToolFill::slot_optionButtonStripContiguousFillMode_buttonToggled(
    KoGroupButton *button,
    bool checked)
{
    if (!checked) {
        return;
    }

    const bool visible = button == m_buttonContiguousFillModeBoundaryFill;
    KisOptionCollectionWidgetWithHeader *sectionRegionExtent =
        m_optionWidget->widgetAs<KisOptionCollectionWidgetWithHeader*>("sectionRegionExtent");
    sectionRegionExtent->setWidgetVisible("buttonContiguousFillBoundaryColor", visible);

    m_contiguousFillMode = button == m_buttonContiguousFillModeFloodFill
                           ? ContiguousFillMode_FloodFill
                           : ContiguousFillMode_BoundaryFill;

    m_configGroup.writeEntry(
        "contiguousFillMode",
        button == m_buttonContiguousFillModeFloodFill
        ? "floodFill"
        : "boundaryFill"
    );
}

void KisToolFill::slot_buttonContiguousFillBoundaryColor_changed(const KoColor &color)
{
    if (color == m_contiguousFillBoundaryColor) {
        return;
    }
    m_contiguousFillBoundaryColor = color;
    m_configGroup.writeEntry("contiguousFillBoundaryColor", color.toXML());
}

void KisToolFill::slot_sliderThreshold_valueChanged(int value)
{
    if (value == m_threshold) {
        return;
    }
    m_threshold = value;
    m_configGroup.writeEntry("thresholdAmount", value);
}

void KisToolFill::slot_sliderSpread_valueChanged(int value)
{
    if (value == m_opacitySpread) {
        return;
    }
    m_opacitySpread = value;
    m_configGroup.writeEntry("opacitySpread", value);
}

void KisToolFill::slot_sliderCloseGap_valueChanged(int value)
{
    if (value == m_closeGap) {
        return;
    }
    m_closeGap = value;
    m_configGroup.writeEntry("closeGapAmount", value);
}

void KisToolFill::slot_checkBoxSelectionAsBoundary_toggled(bool checked)
{
    if (checked == m_useSelectionAsBoundary) {
        return;
    }
    m_useSelectionAsBoundary = checked;
    m_configGroup.writeEntry("useSelectionAsBoundary", checked);
}

void KisToolFill::slot_checkBoxAntiAlias_toggled(bool checked)
{
    if (checked == m_antiAlias) {
        return;
    }
    m_antiAlias = checked;
    m_configGroup.writeEntry("antiAlias", checked);
}

void KisToolFill::slot_sliderGrow_valueChanged(int value)
{
    if (value == m_sizemod) {
        return;
    }
    m_sizemod = value;
    m_configGroup.writeEntry("growSelection", value);
}

void KisToolFill::slot_buttonStopGrowingAtDarkestPixel_toggled(bool enabled)
{
    if (enabled == m_stopGrowingAtDarkestPixel) {
        return;
    }
    m_stopGrowingAtDarkestPixel = enabled;
    m_configGroup.writeEntry("stopGrowingAtDarkestPixel", enabled);
}

void KisToolFill::slot_sliderFeather_valueChanged(int value)
{
    if (value == m_feather) {
        return;
    }
    m_feather = value;
    m_configGroup.writeEntry("featherAmount", value);
}

void KisToolFill::slot_optionButtonStripReference_buttonToggled(
    KoGroupButton *button,
    bool checked)
{
    if (!checked) {
        return;
    }
    KisOptionCollectionWidgetWithHeader *sectionReference =
        m_optionWidget->widgetAs<KisOptionCollectionWidgetWithHeader*>("sectionReference");
    sectionReference->setWidgetVisible("widgetLabels", button == m_buttonReferenceLabeled);
    
    m_reference = button == m_buttonReferenceCurrent ? Reference_CurrentLayer
                                                     : (button == m_buttonReferenceAll
                                                        ? Reference_AllLayers
                                                        : Reference_ColorLabeledLayers);

    m_configGroup.writeEntry(
        "sampleLayersMode",
        button == m_buttonReferenceCurrent
        ? "currentLayer"
        : (button == m_buttonReferenceAll ? "allLayers" : "colorLabeledLayers")
    );
}

void KisToolFill::slot_widgetLabels_selectionChanged()
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

void KisToolFill::slot_optionButtonStripDragFill_buttonToggled(
    KoGroupButton *button,
    bool checked)
{
    if (!checked) {
        return;
    }
    m_continuousFillMode = button == m_buttonDragFillAny
                            ? ContinuousFillMode_FillAnyRegion
                            : button == m_buttonDragFillDoNotUse
                                ? ContinuousFillMode_DoNotUse
                                : ContinuousFillMode_FillSimilarRegions;
    m_configGroup.writeEntry(
        "continuousFillMode",
        button == m_buttonDragFillAny
            ? "fillAnyRegion"
            : button == m_buttonDragFillDoNotUse
                ? "doNotUse"
                : "fillSimilarRegions"
    );
}

void KisToolFill::slot_buttonReset_clicked()
{
    m_buttonWhatToFillContiguous->setChecked(true);
    m_buttonFillWithFG->setChecked(true);
    m_sliderPatternScale->setValue(100.0);
    m_angleSelectorPatternRotation->setAngle(0.0);
    m_checkBoxCustomBlendingOptions->setChecked(false);
    m_sliderCustomOpacity->setValue(100);
    m_comboBoxCustomCompositeOp->selectCompositeOp(KoID(COMPOSITE_OVER));
    m_sliderThreshold->setValue(8);
    m_sliderSpread->setValue(100);
    m_sliderCloseGap->setValue(0);
    m_checkBoxSelectionAsBoundary->setChecked(true);
    m_checkBoxAntiAlias->setChecked(false);
    m_sliderGrow->setValue(0);
    m_buttonStopGrowingAtDarkestPixel->setChecked(false);
    m_sliderFeather->setValue(0);
    m_buttonReferenceCurrent->setChecked(true);
    m_widgetLabels->setSelection({});
    m_buttonDragFillAny->setChecked(true);
}

void KisToolFill::slot_currentNodeChanged(const KisNodeSP node)
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

void KisToolFill::slot_colorSpaceChanged(const KoColorSpace *colorSpace)
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

void KisToolFill::slot_checkBoxUseActiveLayer_toggled(bool checked)
{
    if (checked == m_useActiveLayer) {
        return;
    }
    m_useActiveLayer = checked;
    m_configGroup.writeEntry("useActiveLayer", checked);
}
