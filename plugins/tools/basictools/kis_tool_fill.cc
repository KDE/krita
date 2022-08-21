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

#include <QLabel>
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
#include <kis_cursor.h>
#include <kis_color_filter_combo.h>
#include <KisAngleSelector.h>
#include <kis_color_label_selector_widget.h>
#include <kis_color_button.h>

#include <processing/fill_processing_visitor.h>
#include <kis_command_utils.h>
#include <kis_layer_utils.h>
#include <krita_utils.h>
#include <kis_stroke_strategy_undo_command_based.h>
#include <commands_new/kis_processing_command.h>
#include <commands_new/kis_update_command.h>

#include <KisPart.h>
#include <KisDocument.h>
#include <kis_dummies_facade.h>
#include <KoShapeControllerBase.h>
#include <kis_shape_controller.h>

#include "kis_icon_utils.h"

KisToolFill::KisToolFill(KoCanvasBase * canvas)
        : KisToolPaint(canvas, KisCursor::load("tool_fill_cursor.png", 6, 6))
        , m_continuousFillMask(nullptr)
        , m_referencePaintDevice(nullptr)
        , m_referenceNodeList(nullptr)
        , m_compressorContinuousFillUpdate(150, KisSignalCompressor::FIRST_ACTIVE)
        , m_fillStrokeId(nullptr)
{
    setObjectName("tool_fill");
    connect(&m_compressorContinuousFillUpdate, SIGNAL(timeout()), SLOT(slotUpdateContinuousFill()));
}

KisToolFill::~KisToolFill()
{
}

void KisToolFill::resetCursorStyle()
{
    KisToolPaint::resetCursorStyle();

    overrideCursorIfNotEditable();
}

void KisToolFill::activate(const QSet<KoShape*> &shapes)
{
    KisToolPaint::activate(shapes);
    m_configGroup = KSharedConfig::openConfig()->group(toolId());
}

void KisToolFill::deactivate()
{
    m_referencePaintDevice = nullptr;
    m_referenceNodeList = nullptr;
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
    
    // Switch the fill mode if alt modifier is pressed
    m_effectiveFillMode =
        event->modifiers() == Qt::AltModifier
        ? (m_fillMode == FillSelection ? FillContiguousRegion : FillSelection)
        : m_fillMode;

    m_seedPoints.append(lastImagePosition);
    beginFilling(lastImagePosition);
    m_isFilling = true;
}

void KisToolFill::continuePrimaryAction(KoPointerEvent *event)
{
    if (!m_isFilling) {
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

    m_compressorContinuousFillUpdate.start();
}

void KisToolFill::endPrimaryAction(KoPointerEvent *)
{
    if (m_isFilling) {
        m_compressorContinuousFillUpdate.stop();
        slotUpdateContinuousFill();
        endFilling();
    }

    m_isFilling = false;
    m_isDragging = false;
    m_seedPoints.clear();
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

    if (m_reference == CurrentLayer) {
        m_referencePaintDevice = currentNode()->paintDevice();
    } else if (m_reference == AllLayers) {
        m_referencePaintDevice = currentImage()->projection();
    } else if (m_reference == ColorLabeledLayers) {
        KisImageSP refImage = KisMergeLabeledLayersCommand::createRefImage(image(), "Fill Tool Reference Image");
        if (!m_referenceNodeList) {
            m_referencePaintDevice = KisMergeLabeledLayersCommand::createRefPaintDevice(image(), "Fill Tool Reference Result Paint Device");
            m_referenceNodeList.reset(new KisMergeLabeledLayersCommand::ReferenceNodeInfoList);
        }
        KisPaintDeviceSP newReferencePaintDevice = KisMergeLabeledLayersCommand::createRefPaintDevice(image(), "Fill Tool Reference Result Paint Device");
        KisMergeLabeledLayersCommand::ReferenceNodeInfoListSP newReferenceNodeList(new KisMergeLabeledLayersCommand::ReferenceNodeInfoList);
        image()->addJob(
            m_fillStrokeId,
            new KisStrokeStrategyUndoCommandBased::Data(
                KUndo2CommandSP(new KisMergeLabeledLayersCommand(refImage,
                                                                 m_referenceNodeList,
                                                                 newReferenceNodeList,
                                                                 m_referencePaintDevice,
                                                                 newReferencePaintDevice,
                                                                 image()->root(),
                                                                 m_selectedColorLabels,
                                                                 KisMergeLabeledLayersCommand::GroupSelectionPolicy_SelectIfColorLabeled)),
                false,
                KisStrokeJobData::SEQUENTIAL,
                KisStrokeJobData::EXCLUSIVE
            )
        );
        m_referencePaintDevice = newReferencePaintDevice;
        m_referenceNodeList = newReferenceNodeList;
    }

    if (m_reference == ColorLabeledLayers) {
        // We need to obtain the reference color from the reference paint
        // device, but it is produced in a stroke, so we must get the color
        // after the device is ready. So we get it in the stroke
        image()->addJob(
            m_fillStrokeId,
            new KisStrokeStrategyUndoCommandBased::Data(
                KUndo2CommandSP(new KisCommandUtils::LambdaCommand(
                    [this, seedPoint]() -> KUndo2Command*
                    {
                        m_continuousFillReferenceColor = m_referencePaintDevice->pixel(seedPoint);
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
        m_continuousFillReferenceColor = m_referencePaintDevice->pixel(seedPoint);
    }

    m_continuousFillMask = new KisSelection;

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

    FillProcessingVisitor *visitor =  new FillProcessingVisitor(m_referencePaintDevice,
                                                                m_resourcesSnapshot->activeSelection(),
                                                                m_resourcesSnapshot);

    const bool useFastMode = !m_resourcesSnapshot->activeSelection() &&
                             m_resourcesSnapshot->opacity() == OPACITY_OPAQUE_U8 &&
                             m_resourcesSnapshot->compositeOpId() == COMPOSITE_OVER &&
                             m_fillType != FillWithPattern &&
                             m_opacitySpread == 100 &&
                             m_useSelectionAsBoundary == false &&
                             !m_antiAlias && m_sizemod == 0 && m_feather == 0 &&
                             m_reference == CurrentLayer;

    visitor->setSeedPoints(seedPoints);
    visitor->setUseFastMode(useFastMode);
    visitor->setSelectionOnly(m_effectiveFillMode == FillSelection);
    visitor->setUseBgColor(m_fillType == FillWithBackgroundColor);
    visitor->setUsePattern(m_fillType == FillWithPattern);
    visitor->setRegionFillingMode(
        m_contiguousFillMode == FloodFill
        ? KisFillPainter::RegionFillingMode_FloodFill
        : KisFillPainter::RegionFillingMode_BoundaryFill
    );
    if (m_contiguousFillMode == BoundaryFill) {
        visitor->setRegionFillingBoundaryColor(m_contiguousFillBoundaryColor);
    }
    visitor->setFillThreshold(m_threshold);
    visitor->setOpacitySpread(m_opacitySpread);
    visitor->setUseSelectionAsBoundary(m_useSelectionAsBoundary);
    visitor->setAntiAlias(m_antiAlias);
    visitor->setSizeMod(m_sizemod);
    visitor->setStopGrowingAtDarkestPixel(m_stopGrowingAtDarkestPixel);
    visitor->setFeather(m_feather);
    if (m_isDragging) {
        visitor->setContinuousFillMode(
            m_continuousFillMode == FillAnyRegion
            ? FillProcessingVisitor::ContinuousFillMode_FillAnyRegion
            : FillProcessingVisitor::ContinuousFillMode_FillSimilarRegions
        );
        visitor->setContinuousFillMask(m_continuousFillMask);
        visitor->setContinuousFillReferenceColor(m_continuousFillReferenceColor);
    }

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

void KisToolFill::addUpdateOperation()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_fillStrokeId);

    image()->addJob(
        m_fillStrokeId,
        new KisStrokeStrategyUndoCommandBased::Data(
            KUndo2CommandSP(new KisUpdateCommand(currentNode(), image()->bounds(), image().data())),
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
}

void KisToolFill::slotUpdateContinuousFill()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_fillStrokeId);

    addFillingOperation(KritaUtils::rasterizePolylineDDA(m_seedPoints));
    addUpdateOperation();
    // clear to not re-add the segments, but retain the last point to mantain continuity
    m_seedPoints = {m_seedPoints.last()};
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
    m_sliderPatternScale->setPrefix(i18nc("The pattern 'scale' spinbox prefix in fill tool options", "Scale: "));
    m_sliderPatternScale->setSuffix(i18n("%"));
    m_angleSelectorPatternRotation = new KisAngleSelector;
    m_angleSelectorPatternRotation->setFlipOptionsMode(KisAngleSelector::FlipOptionsMode_ContextMenu);
    m_angleSelectorPatternRotation->setIncreasingDirection(KisAngleGauge::IncreasingDirection_Clockwise);

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
    m_sliderSpread->setPrefix(i18nc("The 'spread' spinbox prefix in fill tool options", "Spread: "));
    m_sliderSpread->setSuffix(i18n("%"));
    m_sliderSpread->setRange(0, 100);
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
    m_sliderGrow->setRange(-40, 40);
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
    m_sliderFeather->setRange(0, 40);
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

    KisOptionButtonStrip *optionButtonStripMultipleFill =
        new KisOptionButtonStrip;
    m_buttonMultipleFillAny = optionButtonStripMultipleFill->addButton(
        KisIconUtils::loadIcon("different-regions"));
    m_buttonMultipleFillSimilar = optionButtonStripMultipleFill->addButton(
        KisIconUtils::loadIcon("similar-regions"));
    m_buttonMultipleFillAny->setChecked(true);

    QPushButton *buttonReset = new QPushButton(i18nc("The 'reset' button in fill tool options", "Reset"));

    // Set the tooltips
    m_buttonWhatToFillSelection->setToolTip(i18n("Current selection"));
    m_buttonWhatToFillContiguous->setToolTip(i18n("Contiguous region obtained from the layers"));

    m_buttonFillWithFG->setToolTip(i18n("Foreground color"));
    m_buttonFillWithBG->setToolTip(i18n("Background color"));
    m_buttonFillWithPattern->setToolTip(i18n("Pattern"));
    m_sliderPatternScale->setToolTip(i18n("Set the scale of the pattern"));
    m_angleSelectorPatternRotation->setToolTip(i18n("Set the rotation of the pattern"));

    m_buttonContiguousFillModeFloodFill->setToolTip(i18n("Select pixels similar to the one you clicked on"));
    m_buttonContiguousFillModeBoundaryFill->setToolTip(i18n("Select all pixels until a specific boundary color"));
    m_buttonContiguousFillBoundaryColor->setToolTip(i18n("Boundary color"));
    m_sliderThreshold->setToolTip(i18n("Set how far the region should extend from the selected pixel in terms of color similarity"));
    m_sliderSpread->setToolTip(i18n("Set how far the fully opaque portion of the region should extend."
                                    "\n0% will make opaque only the pixels that are exactly equal to the selected pixel."
                                    "\n100% will make opaque all the pixels in the region up to its boundary."));
    m_checkBoxSelectionAsBoundary->setToolTip(i18n("Set if the contour of the current selection should be treated as a boundary when obtaining the region"));

    m_checkBoxAntiAlias->setToolTip(i18n("Smooth the jagged edges"));
    m_sliderGrow->setToolTip(i18n("Grow (positive values) or shrink (negative values) the region by the set amount"));
    m_buttonStopGrowingAtDarkestPixel->setToolTip(i18n("Stop growing at the darkest and/or most opaque pixels"));
    m_sliderFeather->setToolTip(i18n("Blur the region by the set amount"));

    m_buttonReferenceCurrent->setToolTip(i18n("Obtain the region using the active layer"));
    m_buttonReferenceAll->setToolTip(i18n("Obtain the region using a merged copy of all layers"));
    m_buttonReferenceLabeled->setToolTip(i18n("Obtain the region using a merged copy of the selected color-labeled layers"));

    m_buttonMultipleFillAny->setToolTip(i18n("Fill regions of any color"));
    m_buttonMultipleFillSimilar->setToolTip(i18n("Fill only regions similar in color to the initial region"));

    buttonReset->setToolTip(i18n("Reset the options to their default values"));

    // Construct the option widget
    m_optionWidget = new KisOptionCollectionWidget;
    m_optionWidget->setContentsMargins(0, 10, 0, 10);
    m_optionWidget->setSeparatorsVisible(true);

    KisOptionCollectionWidgetWithHeader *sectionWhatToFill =
        new KisOptionCollectionWidgetWithHeader(
            i18nc("The 'what to fill' section label in fill tool options", "What to fill")
        );
    sectionWhatToFill->setPrimaryWidget(optionButtonStripWhatToFill);
    m_optionWidget->appendWidget("sectionWhatToFill", sectionWhatToFill);

    KisOptionCollectionWidgetWithHeader *sectionFillWith =
        new KisOptionCollectionWidgetWithHeader(
            i18nc("The 'fill with' section label in fill tool options", "Fill with")
        );
    sectionFillWith->setPrimaryWidget(optionButtonStripFillWith);
    sectionFillWith->appendWidget("sliderPatternScale", m_sliderPatternScale);
    sectionFillWith->appendWidget("angleSelectorPatternRotation", m_angleSelectorPatternRotation);
    sectionFillWith->setWidgetVisible("sliderPatternScale", false);
    sectionFillWith->setWidgetVisible("angleSelectorPatternRotation", false);
    m_optionWidget->appendWidget("sectionFillWith", sectionFillWith);

    KisOptionCollectionWidgetWithHeader *sectionRegionExtent =
        new KisOptionCollectionWidgetWithHeader(
            i18nc("The 'region extent' section label in fill tool options", "Region extent")
        );
    sectionRegionExtent->setPrimaryWidget(optionButtonStripContiguousFillMode);
    sectionRegionExtent->appendWidget("buttonContiguousFillBoundaryColor", m_buttonContiguousFillBoundaryColor);
    sectionRegionExtent->setWidgetVisible("buttonContiguousFillBoundaryColor", false);
    sectionRegionExtent->appendWidget("sliderThreshold", m_sliderThreshold);
    sectionRegionExtent->appendWidget("sliderSpread", m_sliderSpread);
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
    
    KisOptionCollectionWidgetWithHeader *sectionReference =
        new KisOptionCollectionWidgetWithHeader(
            i18nc("The 'reference' section label in fill tool options", "Reference")
        );
    sectionReference->setPrimaryWidget(optionButtonStripReference);
    sectionReference->appendWidget("widgetLabels", m_widgetLabels);
    sectionReference->setWidgetVisible("widgetLabels", false);
    m_optionWidget->appendWidget("sectionReference", sectionReference);

    KisOptionCollectionWidgetWithHeader *sectionMultipleFill =
        new KisOptionCollectionWidgetWithHeader(
            i18nc("The 'multiple fill' section label in fill tool options", "Multiple fill")
        );
    sectionMultipleFill->setPrimaryWidget(optionButtonStripMultipleFill);
    m_optionWidget->appendWidget("sectionMultipleFill", sectionMultipleFill);

    m_optionWidget->appendWidget("buttonReset", buttonReset);

    // Initialize widgets
    if (m_fillMode == FillSelection) {
        m_buttonWhatToFillSelection->setChecked(true);
        m_optionWidget->setWidgetVisible("sectionRegionExtent", false);
        m_optionWidget->setWidgetVisible("sectionAdjustments", false);
        m_optionWidget->setWidgetVisible("sectionReference", false);
        m_optionWidget->setWidgetVisible("sectionMultipleFill", false);
    }
    if (m_fillType == FillWithBackgroundColor) {
        m_buttonFillWithBG->setChecked(true);
    } else if (m_fillType == FillWithPattern) {
        m_buttonFillWithPattern->setChecked(true);
        sectionFillWith->setWidgetVisible("sliderPatternScale", true);
        sectionFillWith->setWidgetVisible("angleSelectorPatternRotation", true);
    }
    m_sliderPatternScale->setValue(m_patternScale);
    m_angleSelectorPatternRotation->setAngle(m_patternRotation);
    if (m_contiguousFillMode == BoundaryFill) {
        m_buttonContiguousFillModeBoundaryFill->setChecked(true);
        sectionRegionExtent->setWidgetVisible("buttonContiguousFillBoundaryColor", true);
    }
    m_buttonContiguousFillBoundaryColor->setColor(m_contiguousFillBoundaryColor);
    m_sliderThreshold->setValue(m_threshold);
    m_sliderSpread->setValue(m_opacitySpread);
    m_checkBoxSelectionAsBoundary->setChecked(m_useSelectionAsBoundary);
    m_checkBoxAntiAlias->setChecked(m_antiAlias);
    m_sliderGrow->setValue(m_sizemod);
    m_buttonStopGrowingAtDarkestPixel->setChecked(m_stopGrowingAtDarkestPixel);
    m_sliderFeather->setValue(m_feather);
    if (m_reference == AllLayers) {
        m_buttonReferenceAll->setChecked(true);
    } else if (m_reference == ColorLabeledLayers) {
        m_buttonReferenceLabeled->setChecked(true);
        sectionReference->setWidgetVisible("widgetLabels", true);
    }
    if (m_continuousFillMode == FillSimilarRegions) {
        m_buttonMultipleFillSimilar->setChecked(true);
    }
    m_widgetLabels->setSelection(m_selectedColorLabels);

    // Make connections
    connect(optionButtonStripWhatToFill,
            SIGNAL(buttonToggled(KoGroupButton *, bool)),
            SLOT(slot_optionButtonStripWhatToFill_buttonToggled(KoGroupButton *,
                                                                bool)));
    connect(optionButtonStripFillWith,
            SIGNAL(buttonToggled(KoGroupButton *, bool)),
            SLOT(slot_optionButtonStripFillWith_buttonToggled(KoGroupButton *,
                                                              bool)));
    connect(m_sliderPatternScale, SIGNAL(valueChanged(double)), SLOT(slot_sliderPatternScale_valueChanged(double)));
    connect(m_angleSelectorPatternRotation, SIGNAL(angleChanged(double)), SLOT(slot_angleSelectorPatternRotation_angleChanged(double)));
    connect(optionButtonStripContiguousFillMode,
            SIGNAL(buttonToggled(KoGroupButton *, bool)),
            SLOT(slot_optionButtonStripContiguousFillMode_buttonToggled(KoGroupButton *,
                                                                        bool)));
    connect(m_buttonContiguousFillBoundaryColor,
            SIGNAL(changed(const KoColor&)),
            SLOT(slot_buttonContiguousFillBoundaryColor_changed(const KoColor&)));
    connect(m_sliderThreshold, SIGNAL(valueChanged(int)), SLOT(slot_sliderThreshold_valueChanged(int)));
    connect(m_sliderSpread, SIGNAL(valueChanged(int)), SLOT(slot_sliderSpread_valueChanged(int)));
    connect(m_checkBoxSelectionAsBoundary, SIGNAL(toggled(bool)), SLOT(slot_checkBoxSelectionAsBoundary_toggled(bool)));
    connect(m_checkBoxAntiAlias, SIGNAL(toggled(bool)), SLOT(slot_checkBoxAntiAlias_toggled(bool)));
    connect(m_sliderGrow, SIGNAL(valueChanged(int)), SLOT(slot_sliderGrow_valueChanged(int)));
    connect(m_buttonStopGrowingAtDarkestPixel, SIGNAL(toggled(bool)), SLOT(slot_buttonStopGrowingAtDarkestPixel_toogled(bool)));
    connect(m_sliderFeather, SIGNAL(valueChanged(int)), SLOT(slot_sliderFeather_valueChanged(int)));
    connect(optionButtonStripReference,
            SIGNAL(buttonToggled(KoGroupButton *, bool)),
            SLOT(slot_optionButtonStripReference_buttonToggled(KoGroupButton *,
                                                               bool)));
    connect(m_widgetLabels, SIGNAL(selectionChanged()), SLOT(slot_widgetLabels_selectionChanged()));
    connect(
        optionButtonStripMultipleFill,
        SIGNAL(buttonToggled(KoGroupButton *, bool)),
        SLOT(slot_optionButtonStripMultipleFill_buttonToggled(KoGroupButton *,
                                                              bool)));
    connect(buttonReset, SIGNAL(clicked()), SLOT(slot_buttonReset_clicked()));
    
    return m_optionWidget;
}

void KisToolFill::loadConfiguration()
{
    {
        const bool fillSelection = m_configGroup.readEntry<bool>("fillSelection", false);
        m_fillMode = fillSelection ? FillSelection : FillContiguousRegion;
    }
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
    {
        const QString contiguousFillModeStr = m_configGroup.readEntry<QString>("contiguousFillMode", "");
        m_contiguousFillMode = contiguousFillModeStr == "boundaryFill"
                               ? BoundaryFill
                               : FloodFill;
    }
    m_contiguousFillBoundaryColor = loadContiguousFillBoundaryColorFromConfig();
    m_threshold = m_configGroup.readEntry<int>("thresholdAmount", 8);
    m_opacitySpread = m_configGroup.readEntry<int>("opacitySpread", 100);
    m_useSelectionAsBoundary = m_configGroup.readEntry<bool>("useSelectionAsBoundary", true);
    m_antiAlias = m_configGroup.readEntry<bool>("antiAlias", false);
    m_sizemod = m_configGroup.readEntry<int>("growSelection", 0);
    m_stopGrowingAtDarkestPixel = m_configGroup.readEntry<bool>("stopGrowingAtDarkestPixel", false);
    m_feather = m_configGroup.readEntry<int>("featherAmount", 0);
    {
        const QString sampleLayersModeStr = m_configGroup.readEntry<QString>("sampleLayersMode", "currentLayer");
        if (sampleLayersModeStr == "allLayers") {
            m_reference = AllLayers;
        } else if (sampleLayersModeStr == "colorLabeledLayers") {
            m_reference = ColorLabeledLayers;
        } else {
            m_reference = CurrentLayer;
        }
    }
    {
        const QStringList colorLabelsStr = m_configGroup.readEntry<QString>("colorLabels", "").split(',', QString::SkipEmptyParts);
        m_selectedColorLabels.clear();
        for (const QString &colorLabelStr : colorLabelsStr) {
            bool ok;
            const int colorLabel = colorLabelStr.toInt(&ok);
            if (ok) {
                m_selectedColorLabels << colorLabel;
            }
        }
    }
    {
        const QString continuousFillModeStr = m_configGroup.readEntry<QString>("continuousFillMode", "fillAnyRegion");
        if (continuousFillModeStr == "fillSimilarRegions") {
            m_continuousFillMode = FillSimilarRegions;
        } else {
            m_continuousFillMode = FillAnyRegion;
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
    const bool visible = button == m_buttonWhatToFillContiguous;
    m_optionWidget->setWidgetVisible("sectionRegionExtent", visible);
    m_optionWidget->setWidgetVisible("sectionAdjustments", visible);
    m_optionWidget->setWidgetVisible("sectionReference", visible);
    m_optionWidget->setWidgetVisible("sectionMultipleFill", visible);

    m_fillMode = button == m_buttonWhatToFillSelection ? FillSelection : FillContiguousRegion;

    m_configGroup.writeEntry("fillSelection", button == m_buttonWhatToFillSelection);
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
    
    m_fillType = button == m_buttonFillWithFG ? FillWithForegroundColor
                                              : (button == m_buttonFillWithBG ? FillWithBackgroundColor : FillWithPattern);

    m_configGroup.writeEntry(
        "fillWith",
        button == m_buttonFillWithFG ? "foregroundColor" : (button == m_buttonFillWithBG ? "backgroundColor" : "pattern")
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
                           ? FloodFill
                           : BoundaryFill;

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

void KisToolFill::slot_buttonStopGrowingAtDarkestPixel_toogled(bool enabled)
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
    
    m_reference = button == m_buttonReferenceCurrent ? CurrentLayer
                                                     : (button == m_buttonReferenceAll ? AllLayers : ColorLabeledLayers);

    m_configGroup.writeEntry(
        "sampleLayersMode",
        button == m_buttonReferenceCurrent ? "currentLayer" : (button == m_buttonReferenceAll ? "allLayers" : "colorLabeledLayers")
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

void KisToolFill::slot_optionButtonStripMultipleFill_buttonToggled(
    KoGroupButton *button,
    bool checked)
{
    if (!checked) {
        return;
    }
    m_continuousFillMode = button == m_buttonMultipleFillAny ? FillAnyRegion : FillSimilarRegions;
    m_configGroup.writeEntry("continuousFillMode", button == m_buttonMultipleFillAny ? "fillAnyRegion" : "fillSimilarRegions");
}

void KisToolFill::slot_buttonReset_clicked()
{
    m_buttonWhatToFillContiguous->setChecked(true);
    m_buttonFillWithFG->setChecked(true);
    m_sliderPatternScale->setValue(100.0);
    m_angleSelectorPatternRotation->setAngle(0.0);
    m_sliderThreshold->setValue(8);
    m_sliderSpread->setValue(100);
    m_checkBoxSelectionAsBoundary->setChecked(true);
    m_checkBoxAntiAlias->setChecked(false);
    m_sliderGrow->setValue(0);
    m_buttonStopGrowingAtDarkestPixel->setChecked(false);
    m_sliderFeather->setValue(0);
    m_buttonReferenceCurrent->setChecked(true);
    m_widgetLabels->setSelection({});
    m_buttonMultipleFillAny->setChecked(true);
}
