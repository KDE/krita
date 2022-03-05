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
#include <QLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QVector>
#include <QRect>
#include <QColor>
#include <QSharedPointer>

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


KisToolFill::KisToolFill(KoCanvasBase * canvas)
        : KisToolPaint(canvas, KisCursor::load("tool_fill_cursor.png", 6, 6))
        , m_colorLabelCompressor(500, KisSignalCompressor::FIRST_INACTIVE)
        , m_compressorContinuousFillUpdate(150, KisSignalCompressor::FIRST_ACTIVE)
        , m_fillStrokeId(nullptr)
{
    setObjectName("tool_fill");
    m_feather = 0;
    m_sizemod = 0;
    m_threshold = 8;
    m_opacitySpread = 100;
    m_usePattern = false;
    m_fillOnlySelection = false;
    m_continuousFillMode = FillAnyRegion;
    m_continuousFillMask = nullptr;
    connect(&m_colorLabelCompressor, SIGNAL(timeout()), SLOT(slotUpdateAvailableColorLabels()));
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

void KisToolFill::slotUpdateAvailableColorLabels()
{
    if (m_widgetsInitialized && m_cmbSelectedLabels) {
        m_cmbSelectedLabels->updateAvailableLabels(currentImage()->root());
    }
}

void KisToolFill::activate(const QSet<KoShape*> &shapes)
{
    KisToolPaint::activate(shapes);
    m_configGroup =  KSharedConfig::openConfig()->group(toolId());
    if (m_widgetsInitialized && m_imageConnections.isEmpty()) {
        activateConnectionsToImage();
    }
}

void KisToolFill::deactivate()
{
    KisToolPaint::deactivate();
    m_imageConnections.clear();
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
    
    if (event->modifiers() == Qt::AltModifier) {
        m_fillOnlySelection = true;
    }

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

    m_fillOnlySelection = m_checkFillSelection->isChecked();
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

    if (m_sampleLayersMode == SAMPLE_LAYERS_MODE_ALL) {
        m_referencePaintDevice = currentImage()->projection();
    } else if (m_sampleLayersMode == SAMPLE_LAYERS_MODE_CURRENT) {
        m_referencePaintDevice = currentNode()->paintDevice();
    } else if (m_sampleLayersMode == SAMPLE_LAYERS_MODE_COLOR_LABELED) {
        m_referencePaintDevice = KisMergeLabeledLayersCommand::createRefPaintDevice(image(), "Fill Tool Reference Result Paint Device");
        image()->addJob(
            m_fillStrokeId,
            new KisStrokeStrategyUndoCommandBased::Data(
                KUndo2CommandSP(new KisMergeLabeledLayersCommand(refImage, m_referencePaintDevice,
                                                                 currentRoot, m_selectedColors,
                                                                 KisMergeLabeledLayersCommand::GroupSelectionPolicy_SelectIfColorLabeled)),
                false,
                KisStrokeJobData::SEQUENTIAL,
                KisStrokeJobData::EXCLUSIVE
            )
        );
    }

    KIS_SAFE_ASSERT_RECOVER_RETURN(m_referencePaintDevice);

    if (m_sampleLayersMode == SAMPLE_LAYERS_MODE_COLOR_LABELED) {
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
    m_continuousFillReferenceColor = m_referencePaintDevice->pixel(seedPoint);

    m_transform.reset();
    m_transform.rotate(m_patternRotation);
    m_transform.scale(m_patternScale, m_patternScale);
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
    visitor->setSeedPoints(seedPoints);
    visitor->setUseFastMode(m_useFastMode);
    visitor->setUsePattern(m_usePattern);
    visitor->setSelectionOnly(m_fillOnlySelection);
    visitor->setUseSelectionAsBoundary(m_useSelectionAsBoundary);
    visitor->setAntiAlias(m_antiAlias);
    visitor->setFeather(m_feather);
    visitor->setSizeMod(m_sizemod);
    visitor->setFillThreshold(m_threshold);
    visitor->setOpacitySpread(m_opacitySpread);
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
    QWidget *widget = KisToolPaint::createOptionWidget();
    widget->setObjectName(toolId() + " option widget");

    QLabel *lbl_fastMode = new QLabel(i18n("Fast mode:"), widget);
    m_checkUseFastMode = new QCheckBox(QString(), widget);
    m_checkUseFastMode->setToolTip(
        i18n("Fills area faster, but does not take composition "
             "mode into account. Selections and other extended "
             "features will also be disabled."));


    QLabel *lbl_threshold = new QLabel(i18nc("The Threshold label in Fill tool options", "Threshold:"), widget);
    m_slThreshold = new KisSliderSpinBox(widget);
    m_slThreshold->setObjectName("int_widget");
    m_slThreshold->setRange(1, 100);
    m_slThreshold->setPageStep(3);

    QLabel *lbl_opacitySpread = new QLabel(i18nc("The Opacity Spread label in Fill tool options", "Opacity Spread:"), widget);
    m_slOpacitySpread = new KisSliderSpinBox(widget);
    m_slOpacitySpread->setObjectName("opacitySpread");
    m_slOpacitySpread->setSuffix(i18n("%"));
    m_slOpacitySpread->setRange(0, 100);
    m_slOpacitySpread->setPageStep(3);

    QLabel *lbl_antiAlias = new QLabel(i18n("Anti-aliasing"), widget);
    m_checkAntiAlias = new QCheckBox(QString(), widget);
    m_checkAntiAlias->setToolTip(
        i18n("Smooths the jagged edges."));

    QLabel *lbl_sizemod = new QLabel(i18n("Grow selection:"), widget);
    m_sizemodWidget = new KisSliderSpinBox(widget);
    m_sizemodWidget->setObjectName("sizemod");
    m_sizemodWidget->setRange(-40, 40);
    m_sizemodWidget->setSingleStep(1);
    m_sizemodWidget->setSuffix(i18n(" px"));

    QLabel *lbl_feather = new QLabel(i18n("Feathering radius:"), widget);
    m_featherWidget = new KisSliderSpinBox(widget);
    m_featherWidget->setObjectName("feather");
    m_featherWidget->setRange(0, 40);
    m_featherWidget->setSingleStep(1);
    m_featherWidget->setSuffix(i18n(" px"));

    QLabel *lbl_usePattern = new QLabel(i18n("Use pattern:"), widget);
    m_checkUsePattern = new QCheckBox(QString(), widget);
    m_checkUsePattern->setToolTip(i18n("When checked do not use the foreground color, but the pattern selected to fill with"));

    QLabel *lbl_patternRotation = new QLabel(i18n("Rotate:"), widget);
    m_angleSelectorPatternRotate = new KisAngleSelector(widget);
    m_angleSelectorPatternRotate->setFlipOptionsMode(KisAngleSelector::FlipOptionsMode_MenuButton);
    m_angleSelectorPatternRotate->setIncreasingDirection(KisAngleGauge::IncreasingDirection_Clockwise);
    m_angleSelectorPatternRotate->setObjectName("patternrotate");

    QLabel *lbl_patternScale = new QLabel(i18n("Scale:"), widget);
    m_sldPatternScale = new KisDoubleSliderSpinBox(widget);
    m_sldPatternScale->setObjectName("patternscale");
    m_sldPatternScale->setRange(0, 500, 2);
    m_sldPatternScale->setSingleStep(1.0);
    m_sldPatternScale->setSuffix(QChar(Qt::Key_Percent));

    QLabel *lbl_sampleLayers = new QLabel(i18nc("This is a label before a combobox with different choices regarding which layers "
                                                "to take into considerationg when calculating the area to fill. "
                                                "Options together with the label are: /Sample current layer/ /Sample all layers/ "
                                                "/Sample color labeled layers/. Sample is a verb here and means something akin to 'take into account'.", "Sample:"), widget);
    m_cmbSampleLayersMode = new QComboBox(widget);
    m_cmbSampleLayersMode->addItem(sampleLayerModeToUserString(SAMPLE_LAYERS_MODE_CURRENT), SAMPLE_LAYERS_MODE_CURRENT);
    m_cmbSampleLayersMode->addItem(sampleLayerModeToUserString(SAMPLE_LAYERS_MODE_ALL), SAMPLE_LAYERS_MODE_ALL);
    m_cmbSampleLayersMode->addItem(sampleLayerModeToUserString(SAMPLE_LAYERS_MODE_COLOR_LABELED), SAMPLE_LAYERS_MODE_COLOR_LABELED);
    m_cmbSampleLayersMode->setEditable(false);

    QLabel *lbl_cmbLabel = new QLabel(i18nc("This is a string in tool options for Fill Tool to describe a combobox about "
                                            "a choice of color labels that a layer can be marked with. Those color labels "
                                            "will be used for calculating the area to fill.", "Labels used:"), widget);
    m_cmbSelectedLabels = new KisColorFilterCombo(widget, false, false);
    m_cmbSelectedLabels->updateAvailableLabels(currentImage().isNull() ? KisNodeSP() : currentImage()->root());

    QLabel *lbl_fillSelection = new QLabel(i18n("Fill entire selection:"), widget);
    m_checkFillSelection = new QCheckBox(QString(), widget);
    m_checkFillSelection->setToolTip(i18n("When checked do not look at the current layer colors, but just fill all of the selected area"));

    QLabel *lbl_useSelectionAsBoundary = new QLabel(i18nc("Description for a checkbox in a Fill Tool to use selection borders as boundary when filling", "Use selection as boundary:"), widget);
    m_checkUseSelectionAsBoundary = new QCheckBox(QString(), widget);
    m_checkUseSelectionAsBoundary->setToolTip(i18nc("Tooltip for 'Use selection as boundary' checkbox", "When checked, use selection borders as boundary when filling"));

    QLabel *lbl_continuousFillMode = new QLabel(i18nc("The 'continuous fill' label in Fill tool options", "Continuous Fill:"), widget);
    m_cmbContinuousFillMode = new QComboBox(widget);
    m_cmbContinuousFillMode->addItem(i18nc("Continuous fill mode in fill tool options", "Any region"));
    m_cmbContinuousFillMode->addItem(i18nc("Continuous fill mode in fill tool options", "Similar regions"));
    m_cmbContinuousFillMode->setEditable(false);
    m_cmbContinuousFillMode->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    m_cmbContinuousFillMode->view()->setMinimumWidth(m_cmbContinuousFillMode->view()->sizeHintForColumn(0));

    connect (m_checkUseFastMode  , SIGNAL(toggled(bool))    , this, SLOT(slotSetUseFastMode(bool)));
    connect (m_slThreshold       , SIGNAL(valueChanged(int)), this, SLOT(slotSetThreshold(int)));
    connect (m_slOpacitySpread        , SIGNAL(valueChanged(int)), this, SLOT(slotSetOpacitySpread(int)));
    connect (m_checkAntiAlias    , SIGNAL(toggled(bool)), this, SLOT(slotSetAntiAlias(bool)));
    connect (m_sizemodWidget     , SIGNAL(valueChanged(int)), this, SLOT(slotSetSizemod(int)));
    connect (m_featherWidget     , SIGNAL(valueChanged(int)), this, SLOT(slotSetFeather(int)));
    connect (m_checkUsePattern   , SIGNAL(toggled(bool))    , this, SLOT(slotSetUsePattern(bool)));
    connect (m_checkFillSelection, SIGNAL(toggled(bool))    , this, SLOT(slotSetFillSelection(bool)));
    connect (m_checkUseSelectionAsBoundary, SIGNAL(toggled(bool))    , this, SLOT(slotSetUseSelectionAsBoundary(bool)));
    connect (m_cmbContinuousFillMode,
             QOverload<int>::of(&QComboBox::currentIndexChanged),
             [this](int i){ slotSetContinuousFillMode(static_cast<ContinuousFillMode>(i)); });

    connect (m_cmbSampleLayersMode   , SIGNAL(currentIndexChanged(int)), this, SLOT(slotSetSampleLayers(int)));
    connect (m_cmbSelectedLabels          , SIGNAL(selectedColorsChanged()), this, SLOT(slotSetSelectedColorLabels()));
    connect (m_angleSelectorPatternRotate  , SIGNAL(angleChanged(qreal)), this, SLOT(slotSetPatternRotation(qreal)));
    connect (m_sldPatternScale   , SIGNAL(valueChanged(qreal)), this, SLOT(slotSetPatternScale(qreal)));

    addOptionWidgetOption(m_checkUseFastMode, lbl_fastMode);
    addOptionWidgetOption(m_slThreshold, lbl_threshold);
    addOptionWidgetOption(m_slOpacitySpread, lbl_opacitySpread);
    addOptionWidgetOption(m_checkAntiAlias, lbl_antiAlias);
    addOptionWidgetOption(m_sizemodWidget, lbl_sizemod);
    addOptionWidgetOption(m_featherWidget, lbl_feather);
    addOptionWidgetOption(m_cmbContinuousFillMode, lbl_continuousFillMode);

    addOptionWidgetOption(m_checkFillSelection, lbl_fillSelection);
    addOptionWidgetOption(m_checkUseSelectionAsBoundary, lbl_useSelectionAsBoundary);
    addOptionWidgetOption(m_cmbSampleLayersMode, lbl_sampleLayers);
    addOptionWidgetOption(m_cmbSelectedLabels, lbl_cmbLabel);
    addOptionWidgetOption(m_checkUsePattern, lbl_usePattern);

    addOptionWidgetOption(m_angleSelectorPatternRotate, lbl_patternRotation);
    addOptionWidgetOption(m_sldPatternScale, lbl_patternScale);

    updateGUI();

    widget->setFixedHeight(widget->sizeHint().height());



    // load configuration options
    m_checkUseFastMode->setChecked(m_configGroup.readEntry("useFastMode", false));
    m_slThreshold->setValue(m_configGroup.readEntry("thresholdAmount", 8));
    m_slOpacitySpread->setValue(m_configGroup.readEntry("opacitySpread", 100));
    m_checkAntiAlias->setChecked(m_configGroup.readEntry("antiAlias", false));
    m_sizemodWidget->setValue(m_configGroup.readEntry("growSelection", 0));

    m_cmbContinuousFillMode->setCurrentIndex(m_configGroup.readEntry("continuousFillMode", "fillAnyRegion") == "fillSimilarRegions" ? 1 : 0);

    m_featherWidget->setValue(m_configGroup.readEntry("featherAmount", 0));
    m_checkUsePattern->setChecked(m_configGroup.readEntry("usePattern", false));
    if (m_configGroup.hasKey("sampleLayersMode")) {
        m_sampleLayersMode = m_configGroup.readEntry("sampleLayersMode", SAMPLE_LAYERS_MODE_CURRENT);
        setCmbSampleLayersMode(m_sampleLayersMode);
    } else { // if neither option is present in the configuration, it will fall back to CURRENT
        bool sampleMerged = m_configGroup.readEntry("sampleMerged", false);
        m_sampleLayersMode = sampleMerged ? SAMPLE_LAYERS_MODE_ALL : SAMPLE_LAYERS_MODE_CURRENT;
        setCmbSampleLayersMode(m_sampleLayersMode);
    }
    m_checkFillSelection->setChecked(m_configGroup.readEntry("fillSelection", false));
    m_checkUseSelectionAsBoundary->setChecked(m_configGroup.readEntry("useSelectionAsBoundary", false));

    m_angleSelectorPatternRotate->setAngle(m_configGroup.readEntry("patternRotate", 0.0));
    m_sldPatternScale->setValue(m_configGroup.readEntry("patternScale", 100.0));

    // manually set up all variables in case there were no signals when setting value
    m_antiAlias = m_checkAntiAlias->isChecked();
    m_feather = m_featherWidget->value();
    m_sizemod = m_sizemodWidget->value();
    m_threshold = m_slThreshold->value();
    m_opacitySpread = m_slOpacitySpread->value();
    m_useFastMode = m_checkUseFastMode->isChecked();
    m_fillOnlySelection = m_checkFillSelection->isChecked();
    m_useSelectionAsBoundary = m_checkUseSelectionAsBoundary->isChecked();
    m_patternRotation = m_angleSelectorPatternRotate->angle();
    m_patternScale = m_sldPatternScale->value() * 0.01;
    m_usePattern = m_checkUsePattern->isChecked();
    // m_sampleLayersMode is set manually above
    // selectedColors are also set manually


    activateConnectionsToImage();

    m_widgetsInitialized = true;
    return widget;
}

void KisToolFill::updateGUI()
{
    bool useAdvancedMode = !m_checkUseFastMode->isChecked();
    bool selectionOnly = m_checkFillSelection->isChecked();

    m_checkUseFastMode->setEnabled(!selectionOnly);
    m_slThreshold->setEnabled(!selectionOnly);
    m_slOpacitySpread->setEnabled(!selectionOnly && useAdvancedMode);

    m_checkAntiAlias->setEnabled(!selectionOnly && useAdvancedMode);
    m_sizemodWidget->setEnabled(!selectionOnly && useAdvancedMode);
    m_featherWidget->setEnabled(!selectionOnly && useAdvancedMode);
    m_checkUsePattern->setEnabled(useAdvancedMode);
    m_angleSelectorPatternRotate->setEnabled((m_checkUsePattern->isChecked() && useAdvancedMode));
    m_sldPatternScale->setEnabled((m_checkUsePattern->isChecked() && useAdvancedMode));

    m_cmbContinuousFillMode->setEnabled(!selectionOnly);

    m_cmbSampleLayersMode->setEnabled(!selectionOnly && useAdvancedMode);

    m_checkUseSelectionAsBoundary->setEnabled(!selectionOnly && useAdvancedMode);

    bool sampleLayersModeIsColorLabeledLayers = m_cmbSampleLayersMode->currentData().toString() == SAMPLE_LAYERS_MODE_COLOR_LABELED;
    m_cmbSelectedLabels->setEnabled(!selectionOnly && useAdvancedMode && sampleLayersModeIsColorLabeledLayers);
}

QString KisToolFill::sampleLayerModeToUserString(QString sampleLayersModeId)
{
    QString currentLayer = i18nc("Option in fill tool: take only the current layer into account when calculating the area to fill", "Current Layer");
    if (sampleLayersModeId == SAMPLE_LAYERS_MODE_CURRENT) {
        return currentLayer;
    } else if (sampleLayersModeId == SAMPLE_LAYERS_MODE_ALL) {
        return i18nc("Option in fill tool: take all layers (merged) into account when calculating the area to fill", "All Layers");
    } else if (sampleLayersModeId == SAMPLE_LAYERS_MODE_COLOR_LABELED) {
        return i18nc("Option in fill tool: take all layers that were labeled with a color label (more precisely: all those layers merged)"
                     " into account when calculating the area to fill", "Color Labeled Layers");
    }

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(false, currentLayer);
    return currentLayer;
}

void KisToolFill::setCmbSampleLayersMode(QString sampleLayersModeId)
{
    for (int i = 0; i < m_cmbSampleLayersMode->count(); i++) {
        if (m_cmbSampleLayersMode->itemData(i).toString() == sampleLayersModeId)
        {
            m_cmbSampleLayersMode->setCurrentIndex(i);
            break;
        }
    }
    m_sampleLayersMode = sampleLayersModeId;
    updateGUI();
}

void KisToolFill::activateConnectionsToImage()
{
    auto *kisCanvas = dynamic_cast<KisCanvas2 *>(canvas());
    KIS_SAFE_ASSERT_RECOVER_RETURN(kisCanvas);
    KisDocument *doc = kisCanvas->imageView()->document();

    KisShapeController *kritaShapeController =
            dynamic_cast<KisShapeController*>(doc->shapeController());
    m_dummiesFacade = static_cast<KisDummiesFacadeBase*>(kritaShapeController);
    if (m_dummiesFacade) {
        m_imageConnections.addConnection(m_dummiesFacade, SIGNAL(sigEndInsertDummy(KisNodeDummy*)),
                                                     &m_colorLabelCompressor, SLOT(start()));
        m_imageConnections.addConnection(m_dummiesFacade, SIGNAL(sigEndRemoveDummy()),
                                                     &m_colorLabelCompressor, SLOT(start()));
        m_imageConnections.addConnection(m_dummiesFacade, SIGNAL(sigDummyChanged(KisNodeDummy*)),
                                                     &m_colorLabelCompressor, SLOT(start()));
    }
}

void KisToolFill::deactivateConnectionsToImage()
{
    m_imageConnections.clear();
}

void KisToolFill::slotSetUseFastMode(bool value)
{
    m_useFastMode = value;
    updateGUI();
    m_configGroup.writeEntry("useFastMode", value);
}

void KisToolFill::slotSetThreshold(int threshold)
{
    m_threshold = threshold;
    m_configGroup.writeEntry("thresholdAmount", threshold);
}

void KisToolFill::slotSetOpacitySpread(int opacitySpread)
{
    m_opacitySpread = opacitySpread;
    m_configGroup.writeEntry("opacitySpread", opacitySpread);
}

void KisToolFill::slotSetUsePattern(bool state)
{
    m_usePattern = state;
    m_sldPatternScale->setEnabled(state);
    m_angleSelectorPatternRotate->setEnabled(state);
    m_configGroup.writeEntry("usePattern", state);
}

void KisToolFill::slotSetSampleLayers(int index)
{
    Q_UNUSED(index);
    m_sampleLayersMode = m_cmbSampleLayersMode->currentData(Qt::UserRole).toString();
    updateGUI();
    m_configGroup.writeEntry("sampleLayersMode", m_sampleLayersMode);
}

void KisToolFill::slotSetSelectedColorLabels()
{
    m_selectedColors = m_cmbSelectedLabels->selectedColors();
}

void KisToolFill::slotSetPatternScale(qreal scale)
{
    m_patternScale = scale*0.01;
    m_configGroup.writeEntry("patternScale", scale);
}

void KisToolFill::slotSetPatternRotation(qreal rotate)
{
    m_patternRotation = rotate;
    m_configGroup.writeEntry("patternRotate", rotate);
}

void KisToolFill::slotSetFillSelection(bool state)
{
    m_fillOnlySelection = state;
    m_configGroup.writeEntry("fillSelection", state);
    updateGUI();
}

void KisToolFill::slotSetUseSelectionAsBoundary(bool state)
{
    m_useSelectionAsBoundary = state;
    m_configGroup.writeEntry("useSelectionAsBoundary", state);
    updateGUI();
}

void KisToolFill::slotSetAntiAlias(bool antiAlias)
{
    m_antiAlias = antiAlias;
    m_configGroup.writeEntry("antiAlias", antiAlias);
}

void KisToolFill::slotSetSizemod(int sizemod)
{
    m_sizemod = sizemod;
    m_configGroup.writeEntry("growSelection", sizemod);
}

void KisToolFill::slotSetFeather(int feather)
{
    m_feather = feather;
    m_configGroup.writeEntry("featherAmount", feather);
}

void KisToolFill::slotSetContinuousFillMode(ContinuousFillMode continuousFillMode)
{
    m_continuousFillMode = continuousFillMode;
    m_configGroup.writeEntry("continuousFillMode", continuousFillMode == FillAnyRegion ? "fillAnyRegion" : "fillSimilarRegions");
}
