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
#include <QToolButton>
#include <QPushButton>
#include <QButtonGroup>

#include <KisOptionCollectionWidget.h>

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

#include <processing/fill_processing_visitor.h>
#include <kis_command_utils.h>
#include <kis_layer_utils.h>
#include <krita_utils.h>
#include <kis_stroke_strategy_undo_command_based.h>
#include <commands_new/KisMergeLabeledLayersCommand.h>
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

    m_referencePaintDevice = nullptr;

    KisImageWSP currentImageWSP = image();
    KisNodeSP currentRoot = currentImageWSP->root();

    KisImageSP refImage = KisMergeLabeledLayersCommand::createRefImage(image(), "Fill Tool Reference Image");

    if (m_reference == AllLayers) {
        m_referencePaintDevice = currentImage()->projection();
    } else if (m_reference == CurrentLayer) {
        m_referencePaintDevice = currentNode()->paintDevice();
    } else if (m_reference == ColorLabeledLayers) {
        m_referencePaintDevice = KisMergeLabeledLayersCommand::createRefPaintDevice(image(), "Fill Tool Reference Result Paint Device");
        image()->addJob(
            m_fillStrokeId,
            new KisStrokeStrategyUndoCommandBased::Data(
                KUndo2CommandSP(new KisMergeLabeledLayersCommand(refImage, m_referencePaintDevice,
                                                                 currentRoot, m_selectedColorLabels,
                                                                 KisMergeLabeledLayersCommand::GroupSelectionPolicy_SelectIfColorLabeled)),
                false,
                KisStrokeJobData::SEQUENTIAL,
                KisStrokeJobData::EXCLUSIVE
            )
        );
    }

    KIS_SAFE_ASSERT_RECOVER_RETURN(m_referencePaintDevice);

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
    visitor->setFillThreshold(m_threshold);
    visitor->setOpacitySpread(m_opacitySpread);
    visitor->setUseSelectionAsBoundary(m_useSelectionAsBoundary);
    visitor->setAntiAlias(m_antiAlias);
    visitor->setSizeMod(m_sizemod);
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

QToolButton* makeToolButton(const QString &iconFile, bool checked = false)
{
    QToolButton *button = new QToolButton;
    button->setCheckable(true);
    button->setChecked(checked);
    button->setAutoRaise(true);
    button->setAutoExclusive(true);
    button->setIcon(KisIconUtils::loadIcon(iconFile));
    return button;
}

QWidget* makeToolButtonContainer(const QVector<QToolButton*> &buttons)
{
    QWidget *buttonContainer = new QWidget;
    QButtonGroup *buttonGroup = new QButtonGroup(buttonContainer);
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setAlignment(Qt::AlignLeft);
    int id = 0;
    for (QToolButton *button : buttons) {
        button->setParent(buttonContainer);
        buttonGroup->addButton(button, id++);
        layout->addWidget(button);
    }
    buttonContainer->setLayout(layout);
    return buttonContainer;
}

QWidget* KisToolFill::createOptionWidget()
{
    loadConfiguration();

    // Create widgets
    m_buttonWhatToFillSelection = makeToolButton("tool_outline_selection");
    m_buttonWhatToFillContiguous = makeToolButton("contiguous-selection", true);
    QWidget *containerWhatToFillButtons = makeToolButtonContainer({m_buttonWhatToFillSelection, m_buttonWhatToFillContiguous});

    m_buttonFillWithFG = makeToolButton("object-order-lower-calligra", true);
    m_buttonFillWithBG = makeToolButton("object-order-raise-calligra");
    m_buttonFillWithPattern = makeToolButton("pattern");
    QWidget *containerFillWithButtons = makeToolButtonContainer({m_buttonFillWithFG, m_buttonFillWithBG, m_buttonFillWithPattern});
    m_sliderPatternScale = new KisDoubleSliderSpinBox;
    m_sliderPatternScale->setRange(0, 500, 2);
    m_sliderPatternScale->setPrefix(i18nc("The pattern 'scale' spinbox prefix in fill tool options", "Scale: "));
    m_sliderPatternScale->setSuffix(i18n("%"));
    m_angleSelectorPatternRotation = new KisAngleSelector;
    m_angleSelectorPatternRotation->setFlipOptionsMode(KisAngleSelector::FlipOptionsMode_ContextMenu);
    m_angleSelectorPatternRotation->setIncreasingDirection(KisAngleGauge::IncreasingDirection_Clockwise);

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
    m_sliderGrow = new KisSliderSpinBox;
    m_sliderGrow->setPrefix(i18nc("The 'grow/shrink' spinbox prefix in fill tool options", "Grow: "));
    m_sliderGrow->setRange(-40, 40);
    m_sliderGrow->setSuffix(i18n(" px"));
    m_sliderFeather = new KisSliderSpinBox;
    m_sliderFeather->setPrefix(i18nc("The 'feather' spinbox prefix in fill tool options", "Feather: "));
    m_sliderFeather->setRange(0, 40);
    m_sliderFeather->setSuffix(i18n(" px"));

    m_buttonReferenceCurrent = makeToolButton("current-layer", true);
    m_buttonReferenceAll = makeToolButton("all-layers");
    m_buttonReferenceLabeled = makeToolButton("tag");
    QWidget *containerReferenceButtons = makeToolButtonContainer({m_buttonReferenceCurrent, m_buttonReferenceAll, m_buttonReferenceLabeled});
    m_widgetLabels = new KisColorLabelSelectorWidget;
    m_widgetLabels->setExclusive(false);
    m_widgetLabels->setButtonSize(20);
    m_widgetLabels->setButtonWrapEnabled(true);
    m_widgetLabels->setMouseDragEnabled(true);

    m_buttonMultipleFillAny = makeToolButton("different-regions", true);
    m_buttonMultipleFillSimilar = makeToolButton("similar-regions");
    QWidget *containerMultipleFillButtons = makeToolButtonContainer({m_buttonMultipleFillAny, m_buttonMultipleFillSimilar});

    QPushButton *buttonReset = new QPushButton(i18nc("The 'reset' button in fill tool options", "Reset"));

    // Set the tooltips
    m_buttonWhatToFillSelection->setToolTip(i18n("Current selection"));
    m_buttonWhatToFillContiguous->setToolTip(i18n("Contiguous region obtained from the layers"));

    m_buttonFillWithFG->setToolTip(i18n("Foreground color"));
    m_buttonFillWithBG->setToolTip(i18n("Background color"));
    m_buttonFillWithPattern->setToolTip(i18n("Pattern"));
    m_sliderPatternScale->setToolTip(i18n("Set the scale of the pattern"));
    m_angleSelectorPatternRotation->setToolTip(i18n("Set the rotation of the pattern"));

    m_sliderThreshold->setToolTip(i18n("Set how far the region should extend from the selected pixel in terms of color similarity"));
    m_sliderSpread->setToolTip(i18n("Set how far the fully opaque portion of the region should extend."
                                    "\n0% will make opaque only the pixels that are exactly equal to the selected pixel."
                                    "\n100% will make opaque all the pixels in the region up to its boundary."));
    m_checkBoxSelectionAsBoundary->setToolTip(i18n("Set if the contour of the current selection should be treated as a boundary when obtaining the region"));

    m_checkBoxAntiAlias->setToolTip(i18n("Smooth the jagged edges"));
    m_sliderGrow->setToolTip(i18n("Grow (positive values) or shrink (negative values) the region by the set amount"));
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
    sectionWhatToFill->setPrimaryWidget(containerWhatToFillButtons);
    m_optionWidget->appendWidget("sectionWhatToFill", sectionWhatToFill);

    KisOptionCollectionWidgetWithHeader *sectionFillWith =
        new KisOptionCollectionWidgetWithHeader(
            i18nc("The 'fill with' section label in fill tool options", "Fill with")
        );
    sectionFillWith->setPrimaryWidget(containerFillWithButtons);
    sectionFillWith->appendWidget("sliderPatternScale", m_sliderPatternScale);
    sectionFillWith->appendWidget("angleSelectorPatternRotation", m_angleSelectorPatternRotation);
    sectionFillWith->setWidgetVisible("sliderPatternScale", false);
    sectionFillWith->setWidgetVisible("angleSelectorPatternRotation", false);
    m_optionWidget->appendWidget("sectionFillWith", sectionFillWith);

    KisOptionCollectionWidgetWithHeader *sectionRegionExtent =
        new KisOptionCollectionWidgetWithHeader(
            i18nc("The 'region extent' section label in fill tool options", "Region extent")
        );
    sectionRegionExtent->appendWidget("sliderThreshold", m_sliderThreshold);
    sectionRegionExtent->appendWidget("sliderSpread", m_sliderSpread);
    sectionRegionExtent->appendWidget("checkBoxSelectionAsBoundary", m_checkBoxSelectionAsBoundary);
    m_optionWidget->appendWidget("sectionRegionExtent", sectionRegionExtent);

    KisOptionCollectionWidgetWithHeader *sectionAdjustments =
        new KisOptionCollectionWidgetWithHeader(
            i18nc("The 'adjustments' section label in fill tool options", "Adjustments")
        );
    sectionAdjustments->appendWidget("checkBoxAntiAlias", m_checkBoxAntiAlias);
    sectionAdjustments->appendWidget("sliderGrow", m_sliderGrow);
    sectionAdjustments->appendWidget("sliderFeather", m_sliderFeather);
    m_optionWidget->appendWidget("sectionAdjustments", sectionAdjustments);
    
    KisOptionCollectionWidgetWithHeader *sectionReference =
        new KisOptionCollectionWidgetWithHeader(
            i18nc("The 'reference' section label in fill tool options", "Reference")
        );
    sectionReference->setPrimaryWidget(containerReferenceButtons);
    sectionReference->appendWidget("widgetLabels", m_widgetLabels);
    sectionReference->setWidgetVisible("widgetLabels", false);
    m_optionWidget->appendWidget("sectionReference", sectionReference);

    KisOptionCollectionWidgetWithHeader *sectionMultipleFill =
        new KisOptionCollectionWidgetWithHeader(
            i18nc("The 'multiple fill' section label in fill tool options", "Multiple fill")
        );
    sectionMultipleFill->setPrimaryWidget(containerMultipleFillButtons);
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
    m_sliderThreshold->setValue(m_threshold);
    m_sliderSpread->setValue(m_opacitySpread);
    m_checkBoxSelectionAsBoundary->setChecked(m_useSelectionAsBoundary);
    m_checkBoxAntiAlias->setChecked(m_antiAlias);
    m_sliderGrow->setValue(m_sizemod);
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
    connect(m_buttonWhatToFillSelection->group(), SIGNAL(buttonToggled(QAbstractButton*, bool)), SLOT(slot_buttonGroupWhatToFill_buttonToggled(QAbstractButton*, bool)));
    connect(m_buttonFillWithFG->group(), SIGNAL(buttonToggled(QAbstractButton*, bool)), SLOT(slot_buttonGroupFillWith_buttonToggled(QAbstractButton*, bool)));
    connect(m_sliderPatternScale, SIGNAL(valueChanged(double)), SLOT(slot_sliderPatternScale_valueChanged(double)));
    connect(m_angleSelectorPatternRotation, SIGNAL(angleChanged(double)), SLOT(slot_angleSelectorPatternRotation_angleChanged(double)));
    connect(m_sliderThreshold, SIGNAL(valueChanged(int)), SLOT(slot_sliderThreshold_valueChanged(int)));
    connect(m_sliderSpread, SIGNAL(valueChanged(int)), SLOT(slot_sliderSpread_valueChanged(int)));
    connect(m_checkBoxSelectionAsBoundary, SIGNAL(toggled(bool)), SLOT(slot_checkBoxSelectionAsBoundary_toggled(bool)));
    connect(m_checkBoxAntiAlias, SIGNAL(toggled(bool)), SLOT(slot_checkBoxAntiAlias_toggled(bool)));
    connect(m_sliderGrow, SIGNAL(valueChanged(int)), SLOT(slot_sliderGrow_valueChanged(int)));
    connect(m_sliderFeather, SIGNAL(valueChanged(int)), SLOT(slot_sliderFeather_valueChanged(int)));
    connect(m_buttonReferenceCurrent->group(), SIGNAL(buttonToggled(QAbstractButton*, bool)), SLOT(slot_buttonGroupReference_buttonToggled(QAbstractButton*, bool)));
    connect(m_widgetLabels, SIGNAL(selectionChanged()), SLOT(slot_widgetLabels_selectionChanged()));
    connect(m_buttonMultipleFillAny->group(), SIGNAL(buttonToggled(QAbstractButton*, bool)), SLOT(slot_buttonGroupMultipleFill_buttonToggled(QAbstractButton*, bool)));
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
    m_threshold = m_configGroup.readEntry<int>("thresholdAmount", 8);
    m_opacitySpread = m_configGroup.readEntry<int>("opacitySpread", 100);
    m_useSelectionAsBoundary = m_configGroup.readEntry<bool>("useSelectionAsBoundary", true);
    m_antiAlias = m_configGroup.readEntry<bool>("antiAlias", false);
    m_sizemod = m_configGroup.readEntry<int>("growSelection", 0);
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

void KisToolFill::slot_buttonGroupWhatToFill_buttonToggled(QAbstractButton *button, bool checked)
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

void KisToolFill::slot_buttonGroupFillWith_buttonToggled(QAbstractButton *button, bool checked)
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

void KisToolFill::slot_sliderFeather_valueChanged(int value)
{
    if (value == m_feather) {
        return;
    }
    m_feather = value;
    m_configGroup.writeEntry("featherAmount", value);
}

void KisToolFill::slot_buttonGroupReference_buttonToggled(QAbstractButton *button, bool checked)
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

void KisToolFill::slot_buttonGroupMultipleFill_buttonToggled(QAbstractButton *button, bool checked)
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
    m_sliderFeather->setValue(0);
    m_buttonReferenceCurrent->setChecked(true);
    m_widgetLabels->setSelection({});
    m_buttonMultipleFillAny->setChecked(true);
}
