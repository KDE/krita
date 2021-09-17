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
#include <kis_painter.h>
#include <resources/KoPattern.h>
#include <kis_fill_painter.h>
#include <kis_selection.h>

#include <KisViewManager.h>
#include <canvas/kis_canvas2.h>
#include <widgets/kis_cmb_composite.h>
#include <kis_slider_spin_box.h>
#include <kis_cursor.h>
#include "kis_resources_snapshot.h"
#include "commands_new/KisMergeLabeledLayersCommand.h"
#include <kis_color_filter_combo.h>
#include <KisAngleSelector.h>

#include <processing/fill_processing_visitor.h>
#include <kis_processing_applicator.h>
#include <kis_command_utils.h>
#include <functional>
#include <kis_group_layer.h>
#include <kis_layer_utils.h>

#include <KisPart.h>
#include <KisDocument.h>
#include <kis_dummies_facade.h>
#include <KoShapeControllerBase.h>
#include <kis_shape_controller.h>

#include "KoCompositeOpRegistry.h"


KisToolFill::KisToolFill(KoCanvasBase * canvas)
        : KisToolPaint(canvas, KisCursor::load("tool_fill_cursor.png", 6, 6))
        , m_colorLabelCompressor(500, KisSignalCompressor::FIRST_INACTIVE)
{
    setObjectName("tool_fill");
    m_feather = 0;
    m_sizemod = 0;
    m_threshold = 80;
    m_usePattern = false;
    m_fillOnlySelection = false;
    connect(&m_colorLabelCompressor, SIGNAL(timeout()), SLOT(slotUpdateAvailableColorLabels()));
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
    if ( currentNode().isNull() || currentNode()->inherits("KisShapeLayer") || nodePaintAbility()!=NodePaintAbility::PAINT ) {
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

    setMode(KisTool::PAINT_MODE);

    m_startPos = convertToImagePixelCoordFloored(event);
    keysAtStart = event->modifiers();
}

void KisToolFill::endPrimaryAction(KoPointerEvent *event)
{
    Q_UNUSED(event);
    CHECK_MODE_SANITY_OR_RETURN(KisTool::PAINT_MODE);

    setMode(KisTool::HOVER_MODE);

    if (!currentNode() ||
        (!image()->wrapAroundModePermitted() &&
         !image()->bounds().contains(m_startPos))) {

        return;
    }



    Qt::KeyboardModifiers fillOnlySelectionModifier = Qt::AltModifier; // Not sure where to keep it
    if (keysAtStart == fillOnlySelectionModifier) {
      m_fillOnlySelection = true;
    }
    keysAtStart = Qt::NoModifier; // libs/ui/tool/kis_tool_select_base.h cleans it up in endPrimaryAction so i do it too

    KisProcessingApplicator applicator(currentImage(), currentNode(),
                                       KisProcessingApplicator::SUPPORTS_WRAPAROUND_MODE,
                                       KisImageSignalVector(),
                                       kundo2_i18n("Flood Fill"));

    KisResourcesSnapshotSP resources =
        new KisResourcesSnapshot(image(), currentNode(), this->canvas()->resourceManager());

    KisPaintDeviceSP refPaintDevice = 0;

    KisImageWSP currentImageWSP = currentImage();
    KisNodeSP currentRoot = currentImageWSP->root();

    KisImageSP refImage = KisMergeLabeledLayersCommand::createRefImage(image(), "Fill Tool Reference Image");

    if (m_sampleLayersMode == SAMPLE_LAYERS_MODE_ALL) {
        refPaintDevice = currentImage()->projection();
    } else if (m_sampleLayersMode == SAMPLE_LAYERS_MODE_CURRENT) {
        refPaintDevice = currentNode()->paintDevice();
    } else if (m_sampleLayersMode == SAMPLE_LAYERS_MODE_COLOR_LABELED) {

        refPaintDevice = KisMergeLabeledLayersCommand::createRefPaintDevice(image(), "Fill Tool Reference Result Paint Device");

        applicator.applyCommand(new KisMergeLabeledLayersCommand(refImage, refPaintDevice, currentRoot, m_selectedColors),
                                KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);
    }

    KIS_ASSERT(refPaintDevice);

    QTransform transform;

    transform.rotate(m_patternRotation);
    transform.scale(m_patternScale, m_patternScale);
    resources->setFillTransform(transform);

    KisProcessingVisitorSP visitor =
        new FillProcessingVisitor(refPaintDevice,
                                  m_startPos,
                                  resources->activeSelection(),
                                  resources,
                                  m_useFastMode,
                                  m_usePattern,
                                  m_fillOnlySelection,
                                  m_useSelectionAsBoundary,
                                  m_feather,
                                  m_sizemod,
                                  m_threshold,
                                  false, /* use the current device (unmerged) */
                                  false);

    applicator.applyVisitor(visitor,
                            KisStrokeJobData::SEQUENTIAL,
                            KisStrokeJobData::EXCLUSIVE);

    applicator.end();
    m_fillOnlySelection = m_checkFillSelection->isChecked();
}

QWidget* KisToolFill::createOptionWidget()
{
    QWidget *widget = KisToolPaint::createOptionWidget();
    widget->setObjectName(toolId() + " option widget");

    QLabel *lbl_fastMode = new QLabel(i18n("Fast mode: "), widget);
    m_checkUseFastMode = new QCheckBox(QString(), widget);
    m_checkUseFastMode->setToolTip(
        i18n("Fills area faster, but does not take composition "
             "mode into account. Selections and other extended "
             "features will also be disabled."));


    QLabel *lbl_threshold = new QLabel(i18nc("The Threshold label in Fill tool options", "Threshold: "), widget);
    m_slThreshold = new KisSliderSpinBox(widget);
    m_slThreshold->setObjectName("int_widget");
    m_slThreshold->setRange(1, 100);
    m_slThreshold->setPageStep(3);


    QLabel *lbl_sizemod = new QLabel(i18n("Grow selection: "), widget);
    m_sizemodWidget = new KisSliderSpinBox(widget);
    m_sizemodWidget->setObjectName("sizemod");
    m_sizemodWidget->setRange(-40, 40);
    m_sizemodWidget->setSingleStep(1);
    m_sizemodWidget->setSuffix(i18n(" px"));

    QLabel *lbl_feather = new QLabel(i18n("Feathering radius: "), widget);
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



    connect (m_checkUseFastMode       , SIGNAL(toggled(bool))    , this, SLOT(slotSetUseFastMode(bool)));
    connect (m_slThreshold       , SIGNAL(valueChanged(int)), this, SLOT(slotSetThreshold(int)));
    connect (m_sizemodWidget     , SIGNAL(valueChanged(int)), this, SLOT(slotSetSizemod(int)));
    connect (m_featherWidget     , SIGNAL(valueChanged(int)), this, SLOT(slotSetFeather(int)));
    connect (m_checkUsePattern   , SIGNAL(toggled(bool))    , this, SLOT(slotSetUsePattern(bool)));
    connect (m_checkFillSelection, SIGNAL(toggled(bool))    , this, SLOT(slotSetFillSelection(bool)));
    connect (m_checkUseSelectionAsBoundary, SIGNAL(toggled(bool))    , this, SLOT(slotSetUseSelectionAsBoundary(bool)));

    connect (m_cmbSampleLayersMode   , SIGNAL(currentIndexChanged(int)), this, SLOT(slotSetSampleLayers(int)));
    connect (m_cmbSelectedLabels          , SIGNAL(selectedColorsChanged()), this, SLOT(slotSetSelectedColorLabels()));
    connect (m_angleSelectorPatternRotate  , SIGNAL(angleChanged(qreal)), this, SLOT(slotSetPatternRotation(qreal)));
    connect (m_sldPatternScale   , SIGNAL(valueChanged(qreal)), this, SLOT(slotSetPatternScale(qreal)));

    addOptionWidgetOption(m_checkUseFastMode, lbl_fastMode);
    addOptionWidgetOption(m_slThreshold, lbl_threshold);
    addOptionWidgetOption(m_sizemodWidget      , lbl_sizemod);
    addOptionWidgetOption(m_featherWidget      , lbl_feather);

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
    m_sizemodWidget->setValue(m_configGroup.readEntry("growSelection", 0));

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
    m_feather = m_featherWidget->value();
    m_sizemod = m_sizemodWidget->value();
    m_threshold = m_slThreshold->value();
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

    m_sizemodWidget->setEnabled(!selectionOnly && useAdvancedMode);
    m_featherWidget->setEnabled(!selectionOnly && useAdvancedMode);
    m_checkUsePattern->setEnabled(useAdvancedMode);
    m_angleSelectorPatternRotate->setEnabled((m_checkUsePattern->isChecked() && useAdvancedMode));
    m_sldPatternScale->setEnabled((m_checkUsePattern->isChecked() && useAdvancedMode));

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
