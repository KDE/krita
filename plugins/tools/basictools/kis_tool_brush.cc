/*
 *  kis_tool_brush.cc - part of Krita
 *
 *  SPDX-FileCopyrightText: 2003-2004 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2015 Moritz Molch <kde@moritzmolch.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_tool_brush.h"
#include <kis_icon.h>

#include <QCheckBox>
#include <QComboBox>

#include <klocalizedstring.h>
#include <QAction>
#include <QLabel>
#include <kactioncollection.h>

#include <KoCanvasBase.h>
#include <KoCanvasController.h>

#include <kis_action_registry.h>
#include "kis_cursor.h"
#include "kis_config.h"
#include "kis_slider_spin_box.h"
#include "kundo2magicstring.h"

#include <KisUsageLogger.h>
#include "kis_types.h"
#include "kis_tool.h"
#include "kis_paintop_preset.h"
#include "kis_paintop_settings.h"
#include "kis_floating_message.h"
#include "canvas/kis_canvas2.h"
#include "KisViewManager.h"


#define MAXIMUM_SMOOTHNESS_DISTANCE 1000.0 // 0..1000.0 == weight in gui
#define MAXIMUM_MAGNETISM 1000


void KisToolBrush::addSmoothingAction(int enumId, const QString &id)
{
    /**
     * KisToolBrush is the base of several tools, but the actions
     * should be unique, so let's be careful with them
     */
    QAction *a = action(id);
    connect(a, SIGNAL(triggered()), &m_signalMapper, SLOT(map()));
    m_signalMapper.setMapping(a, enumId);
}

KisToolBrush::KisToolBrush(KoCanvasBase * canvas)
    : KisToolFreehand(canvas,
                      KisCursor::load("tool_freehand_cursor.xpm", 2, 2),
                      kundo2_i18n("Freehand Brush Stroke"))
{
    setObjectName("tool_brush");

    createOptionWidget();

    connect(this, SIGNAL(smoothingTypeChanged()), this, SLOT(resetCursorStyle()));

    addSmoothingAction(KisSmoothingOptions::NO_SMOOTHING, "set_no_brush_smoothing");
    addSmoothingAction(KisSmoothingOptions::SIMPLE_SMOOTHING, "set_simple_brush_smoothing");
    addSmoothingAction(KisSmoothingOptions::WEIGHTED_SMOOTHING, "set_weighted_brush_smoothing");
    addSmoothingAction(KisSmoothingOptions::STABILIZER, "set_stabilizer_brush_smoothing");

}

KisToolBrush::~KisToolBrush()
{
}

void KisToolBrush::activate(const QSet<KoShape*> &shapes)
{
    KisToolFreehand::activate(shapes);
    connect(&m_signalMapper, SIGNAL(mapped(int)), SLOT(slotSetSmoothingType(int)), Qt::UniqueConnection);

    QAction *toggleaction = action("toggle_assistant");
    connect(toggleaction, SIGNAL(triggered(bool)), m_chkAssistant, SLOT(toggle()), Qt::UniqueConnection);

    m_configGroup =  KSharedConfig::openConfig()->group(toolId());
}

void KisToolBrush::deactivate()
{
    disconnect(&m_signalMapper, 0, this, 0);
    QAction *toggleaction = action("toggle_assistant");
    disconnect(toggleaction, 0, m_chkAssistant, 0);

    KisToolFreehand::deactivate();
}

int KisToolBrush::smoothingType() const
{
    return smoothingOptions()->smoothingType();
}

bool KisToolBrush::smoothPressure() const
{
    return smoothingOptions()->smoothPressure();
}

int KisToolBrush::smoothnessQuality() const
{
    return smoothingOptions()->smoothnessDistance();
}

qreal KisToolBrush::smoothnessFactor() const
{
    return smoothingOptions()->tailAggressiveness();
}

void KisToolBrush::slotSetSmoothingType(int index)
{
    /**
     * The slot can also be called from smoothing-type-switching
     * action that would mean the combo box will not be synchronized
     */
    if (m_normalSmoothingCont.cmbSmoothingType->currentIndex() != index) {
        m_normalSmoothingCont.cmbSmoothingType->setCurrentIndex(index);
    }

    slotSetSmoothingType_impl(index, normalSmoothingOptions(), m_normalSmoothingCont);

    updateSmoothnessDistanceLabel();

    emit smoothingTypeChanged();
}

void KisToolBrush::updateSmoothnessDistanceLabel()
{
    const qreal oldValue = m_normalSmoothingCont.sliderSmoothnessDistance->value();

    if (smoothingType() == KisSmoothingOptions::STABILIZER) {
        m_normalSmoothingCont.lblSmoothnessDistance->setText(i18n("Sample count:"));
        m_normalSmoothingCont.sliderSmoothnessDistance->setRange(3.0, MAXIMUM_SMOOTHNESS_DISTANCE, 0);
        m_normalSmoothingCont.sliderSmoothnessDistance->setSingleStep(1);
        m_normalSmoothingCont.sliderSmoothnessDistance->setExponentRatio(3.0); // help pick smaller values

        if (!qFuzzyCompare(m_normalSmoothingCont.sliderSmoothnessDistance->value(), oldValue)) {
            // the slider will emit the change signal automatically
            m_normalSmoothingCont.sliderSmoothnessDistance->setValue(qRound(oldValue));
        }

    } else {
        m_normalSmoothingCont.lblSmoothnessDistance->setText(i18nc("Label of Distance value in Freehand brush tool's Weighted Smoothing mode", "Distance:"));
        m_normalSmoothingCont.sliderSmoothnessDistance->setRange(3.0, MAXIMUM_SMOOTHNESS_DISTANCE, 1);
        m_normalSmoothingCont.sliderSmoothnessDistance->setSingleStep(0.1);
        m_normalSmoothingCont.sliderSmoothnessDistance->setExponentRatio(3.0); // help pick smaller values

        if (!qFuzzyCompare(m_normalSmoothingCont.sliderSmoothnessDistance->value(), oldValue)) {
            // the slider will emit the change signal automatically
            m_normalSmoothingCont.sliderSmoothnessDistance->setValue(oldValue);
        }
    }
}

void KisToolBrush::slotSetSmoothnessDistance(qreal distance)
{
    smoothingOptions()->setSmoothnessDistance(distance);
    emit smoothnessQualityChanged();
}

void KisToolBrush::slotSetTailAgressiveness(qreal argh_rhhrr)
{
    smoothingOptions()->setTailAggressiveness(argh_rhhrr);
    emit smoothnessFactorChanged();
}

// used with weighted smoothing
void KisToolBrush::setSmoothPressure(bool value)
{
    smoothingOptions()->setSmoothPressure(value);
}

void KisToolBrush::slotSetMagnetism(int magnetism)
{
    m_magnetism = expf(magnetism / (double)MAXIMUM_MAGNETISM) / expf(1.0);
}

bool KisToolBrush::useScalableDistance() const
{
    return smoothingOptions()->useScalableDistance();
}

// used with weighted smoothing
void KisToolBrush::setUseScalableDistance(bool value)
{
    smoothingOptions()->setUseScalableDistance(value);

    emit useScalableDistanceChanged();
}
bool KisToolBrush::eraserSmoothing() const
{
    return false;   
}
void KisToolBrush::setEraserSmoothing(bool value)
{
    // emit eraserSmoothingChanged();

    // eraserSmoothingOptions()->valid = value;
    eraserSmoothingOptions()->setActive(value);

    showControl(m_eraserSmoothingCont.cmbSmoothingType, value);
    slotSetEraserSmoothingType(value ? m_eraserSmoothingCont.cmbSmoothingType->currentIndex() : KisSmoothingOptions::NO_SMOOTHING);
}

void KisToolBrush::slotSetEraserSmoothingType(int index)
{
    if (m_eraserSmoothingCont.cmbSmoothingType->currentIndex() != index) {
        m_eraserSmoothingCont.cmbSmoothingType->setCurrentIndex(index);
    }

    slotSetSmoothingType_impl(index, eraserSmoothingOptions(), m_eraserSmoothingCont);

    updateSmoothnessDistanceLabel();

    emit smoothingTypeChanged();
}

void KisToolBrush::resetCursorStyle()
{
    KisConfig cfg(true);
    CursorStyle cursorStyle = cfg.newCursorStyle();

    // When the stabilizer is in use, we avoid using the brush outline cursor,
    // because it would hide the real position of the cursor to the user,
    // yielding unexpected results.
    if (smoothingOptions()->smoothingType() == KisSmoothingOptions::STABILIZER &&
            smoothingOptions()->useDelayDistance() &&
            cursorStyle == CURSOR_STYLE_NO_CURSOR) {

        useCursor(KisCursor::roundCursor());
    } else {
        KisToolFreehand::resetCursorStyle();
    }

    overrideCursorIfNotEditable();
}

// stabilizer brush settings
bool KisToolBrush::useDelayDistance() const
{
    return smoothingOptions()->useDelayDistance();
}

qreal KisToolBrush::delayDistance() const
{
    return smoothingOptions()->delayDistance();
}

void KisToolBrush::setUseDelayDistance(bool value)
{
    smoothingOptions()->setUseDelayDistance(value);
    m_normalSmoothingCont.sliderDelayDistance->setEnabled(value);
    enableControl(m_normalSmoothingCont.chkFinishStabilizedCurve, !value);

    emit useDelayDistanceChanged();
}

void KisToolBrush::setDelayDistance(qreal value)
{
    smoothingOptions()->setDelayDistance(value);
    emit delayDistanceChanged();
}

void KisToolBrush::setFinishStabilizedCurve(bool value)
{
    smoothingOptions()->setFinishStabilizedCurve(value);

    emit finishStabilizedCurveChanged();
}

bool KisToolBrush::finishStabilizedCurve() const
{
    return smoothingOptions()->finishStabilizedCurve();
}

void KisToolBrush::setStabilizeSensors(bool value)
{
    smoothingOptions()->setStabilizeSensors(value);
    emit stabilizeSensorsChanged();
}

bool KisToolBrush::stabilizeSensors() const
{
    return smoothingOptions()->stabilizeSensors();
}

void KisToolBrush::updateSettingsViews()
{
    m_normalSmoothingCont.cmbSmoothingType->setCurrentIndex(smoothingOptions()->smoothingType());

    m_normalSmoothingCont.sliderSmoothnessDistance->setValue(smoothingOptions()->smoothnessDistance());
    m_normalSmoothingCont.chkDelayDistance->setChecked(smoothingOptions()->useDelayDistance());
    m_normalSmoothingCont.sliderDelayDistance->setValue(smoothingOptions()->delayDistance());
    m_normalSmoothingCont.sliderTailAggressiveness->setValue(smoothingOptions()->tailAggressiveness());
    m_normalSmoothingCont.chkSmoothPressure->setChecked(smoothingOptions()->smoothPressure());
    m_normalSmoothingCont.chkUseScalableDistance->setChecked(smoothingOptions()->useScalableDistance());
    m_normalSmoothingCont.cmbSmoothingType->setCurrentIndex((int)smoothingOptions()->smoothingType());
    m_normalSmoothingCont.chkStabilizeSensors->setChecked(smoothingOptions()->stabilizeSensors());

    emit smoothnessQualityChanged();
    emit smoothnessFactorChanged();
    emit smoothPressureChanged();
    emit smoothingTypeChanged();
    emit useScalableDistanceChanged();
    emit useDelayDistanceChanged();
    emit delayDistanceChanged();
    emit finishStabilizedCurveChanged();
    emit stabilizeSensorsChanged();

    // emit eraserSmoothingChanged();

    KisTool::updateSettingsViews();
}

QWidget * KisToolBrush::createOptionWidget()
{
    QWidget *optionsWidget = KisToolFreehand::createOptionWidget();
    optionsWidget->setObjectName(toolId() + "option widget");

    // See https://bugs.kde.org/show_bug.cgi?id=316896
    QWidget *specialSpacer = new QWidget(optionsWidget);
    specialSpacer->setObjectName("SpecialSpacer");
    specialSpacer->setFixedSize(0, 0);
    optionsWidget->layout()->addWidget(specialSpacer);

    createSmoothingOptionsWidget(optionsWidget, m_normalSmoothingCont);
    connect(m_normalSmoothingCont.cmbSmoothingType, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSetSmoothingType(int)));
    connect(m_normalSmoothingCont.sliderSmoothnessDistance, SIGNAL(valueChanged(qreal)), SLOT(slotSetSmoothnessDistance(qreal)));
    connect(m_normalSmoothingCont.chkFinishStabilizedCurve, SIGNAL(toggled(bool)), this, SLOT(setFinishStabilizedCurve(bool)));
    connect(m_normalSmoothingCont.chkDelayDistance, SIGNAL(toggled(bool)), this, SLOT(setUseDelayDistance(bool)));
    connect(m_normalSmoothingCont.sliderDelayDistance, SIGNAL(valueChanged(qreal)), SLOT(setDelayDistance(qreal)));
    connect(m_normalSmoothingCont.chkStabilizeSensors, SIGNAL(toggled(bool)), this, SLOT(setStabilizeSensors(bool)));
    connect(m_normalSmoothingCont.sliderTailAggressiveness, SIGNAL(valueChanged(qreal)), SLOT(slotSetTailAgressiveness(qreal)));
    connect(m_normalSmoothingCont.chkSmoothPressure, SIGNAL(toggled(bool)), this, SLOT(setSmoothPressure(bool)));
    connect(m_normalSmoothingCont.chkUseScalableDistance, SIGNAL(toggled(bool)), this, SLOT(setUseScalableDistance(bool)));


    m_chkEraserSmoothing = new QCheckBox(optionsWidget);
    addOptionWidgetOption(m_chkEraserSmoothing, new QLabel(QString("%1:").arg(i18n("Eraser smoothing"))));
    connect(m_chkEraserSmoothing, SIGNAL(toggled(bool)), this, SLOT(setEraserSmoothing(bool)));


    createSmoothingOptionsWidget(optionsWidget, m_eraserSmoothingCont);
    connect(m_eraserSmoothingCont.cmbSmoothingType, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSetEraserSmoothingType(int)));
    setEraserSmoothing(m_chkEraserSmoothing->isChecked());
    


    // add a line spacer so we know that the next set of options are for different settings
    QFrame* line = new QFrame(optionsWidget);
    line->setObjectName(QString::fromUtf8("line"));
    line->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    addOptionWidgetOption(line);

    // Drawing assistant configuration
    QWidget* assistantWidget = new QWidget(optionsWidget);
    QGridLayout* assistantLayout = new QGridLayout(assistantWidget);
    assistantLayout->setContentsMargins(10,0,0,0);
    assistantLayout->setSpacing(5);

    m_chkAssistant = new QCheckBox(optionsWidget);
    m_chkAssistant->setText(i18n("Snap to Assistants"));

    assistantWidget->setToolTip(i18n("You need to add Assistants before this tool will work."));
    connect(m_chkAssistant, SIGNAL(toggled(bool)), this, SLOT(setAssistant(bool)));
    addOptionWidgetOption(assistantWidget, m_chkAssistant);

    m_sliderMagnetism = new KisSliderSpinBox(optionsWidget);
    m_sliderMagnetism->setToolTip(i18n("Assistant Magnetism"));
    m_sliderMagnetism->setRange(0, MAXIMUM_MAGNETISM);

    m_sliderMagnetism->setValue(m_magnetism * MAXIMUM_MAGNETISM);
    connect(m_sliderMagnetism, SIGNAL(valueChanged(int)), SLOT(slotSetMagnetism(int)));

    QLabel* magnetismLabel = new QLabel(i18n("Magnetism:"));
    addOptionWidgetOption(m_sliderMagnetism, magnetismLabel);

    QLabel* snapSingleLabel = new QLabel(i18n("Snap to Single Line"));

    m_chkOnlyOneAssistant = new QCheckBox(optionsWidget);
    m_chkOnlyOneAssistant->setToolTip(i18nc("@info:tooltip","Make it only snap to a single assistant line, prevents snapping mess while using the infinite assistants."));
    m_chkOnlyOneAssistant->setCheckState(Qt::Checked); // turn on by default.
    connect(m_chkOnlyOneAssistant, SIGNAL(toggled(bool)), this, SLOT(setOnlyOneAssistantSnap(bool)));
    addOptionWidgetOption(m_chkOnlyOneAssistant, snapSingleLabel);

    QLabel* snapEraserLabel = new QLabel(i18n("Snap Eraser"));

    m_chkSnapEraser = new QCheckBox(optionsWidget);
    m_chkSnapEraser->setToolTip(i18nc("@info:tooltip","Enable snapping when using erasers"));
    m_chkSnapEraser->setCheckState(Qt::Unchecked); // turn off by default.
    connect(m_chkSnapEraser, SIGNAL(toggled(bool)), this, SLOT(setSnapEraser(bool)));
    addOptionWidgetOption(m_chkSnapEraser, snapEraserLabel);

    // set the assistant snapping options to hidden by default and toggle their visibility based based off snapping checkbox
    m_sliderMagnetism->setVisible(false);
    m_chkOnlyOneAssistant->setVisible(false);
    m_chkSnapEraser->setVisible(false);
    magnetismLabel->setVisible(false);
    snapSingleLabel->setVisible(false);
    snapEraserLabel->setVisible(false);

    connect(m_chkAssistant, SIGNAL(toggled(bool)), m_sliderMagnetism, SLOT(setVisible(bool)));
    connect(m_chkAssistant, SIGNAL(toggled(bool)), m_chkOnlyOneAssistant, SLOT(setVisible(bool)));
    connect(m_chkAssistant, SIGNAL(toggled(bool)), m_chkSnapEraser, SLOT(setVisible(bool)));
    connect(m_chkAssistant, SIGNAL(toggled(bool)), magnetismLabel, SLOT(setVisible(bool)));
    connect(m_chkAssistant, SIGNAL(toggled(bool)), snapSingleLabel, SLOT(setVisible(bool)));
    connect(m_chkAssistant, SIGNAL(toggled(bool)), snapEraserLabel, SLOT(setVisible(bool)));

    KisConfig cfg(true);

    slotSetSmoothingType(cfg.lineSmoothingType());

    return optionsWidget;
}
void KisToolBrush::createSmoothingOptionsWidget(QWidget *optionsWidget, smoothingOptionsControls& controls)
{
    // Line smoothing configuration
    controls.cmbSmoothingType = new QComboBox(optionsWidget);
    controls.cmbSmoothingType->addItems(QStringList()
                                 << i18nc("@item:inlistbox Brush Smoothing", "None")
                                 << i18nc("@item:inlistbox Brush Smoothing", "Basic")
                                 << i18nc("@item:inlistbox Brush Smoothing", "Weighted")
                                 << i18nc("@item:inlistbox Brush Smoothing", "Stabilizer"));
    addOptionWidgetOption(controls.cmbSmoothingType, new QLabel(i18n("Brush Smoothing:")));

    controls.sliderSmoothnessDistance = new KisDoubleSliderSpinBox(optionsWidget);
    controls.sliderSmoothnessDistance->setEnabled(true);
    controls.sliderSmoothnessDistance->setValue(smoothingOptions()->smoothnessDistance());
    controls.lblSmoothnessDistance = new QLabel();
    updateSmoothnessDistanceLabel();

    addOptionWidgetOption(controls.sliderSmoothnessDistance, controls.lblSmoothnessDistance);

    // Finish stabilizer curve
    controls.chkFinishStabilizedCurve = new QCheckBox(optionsWidget);
    controls.chkFinishStabilizedCurve->setMinimumHeight(qMax(controls.sliderSmoothnessDistance->sizeHint().height()-3,
                                                      controls.chkFinishStabilizedCurve->sizeHint().height()));
    controls.chkFinishStabilizedCurve->setChecked(smoothingOptions()->finishStabilizedCurve());

    // Delay Distance for Stabilizer
    QWidget* delayWidget = new QWidget(optionsWidget);
    QHBoxLayout* delayLayout = new QHBoxLayout(delayWidget);
    delayLayout->setContentsMargins(0,0,0,0);
    delayLayout->setSpacing(1);
    QLabel* delayLabel = new QLabel(i18n("Delay:"), optionsWidget);
    delayLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    delayLayout->addWidget(delayLabel);
    controls.chkDelayDistance = new QCheckBox(optionsWidget);
    controls.chkDelayDistance->setLayoutDirection(Qt::RightToLeft);
    delayWidget->setToolTip(i18n("Delay the brush stroke to make the line smoother"));
    delayLayout->addWidget(controls.chkDelayDistance);
    controls.sliderDelayDistance = new KisDoubleSliderSpinBox(optionsWidget);
    controls.sliderDelayDistance->setToolTip(i18n("Radius where the brush is blocked"));
    controls.sliderDelayDistance->setRange(0, 500);
    controls.sliderDelayDistance->setExponentRatio(3.0); // help pick smaller values
    controls.sliderDelayDistance->setSuffix(i18n(" px"));

    addOptionWidgetOption(controls.sliderDelayDistance, delayWidget);
    addOptionWidgetOption(controls.chkFinishStabilizedCurve, new QLabel(i18n("Finish line:")));

    controls.sliderDelayDistance->setValue(smoothingOptions()->delayDistance());
    controls.chkDelayDistance->setChecked(smoothingOptions()->useDelayDistance());
    // if the state is not flipped, then the previous line doesn't generate any signals
    setUseDelayDistance(controls.chkDelayDistance->isChecked());

    // Stabilize sensors
    controls.chkStabilizeSensors = new QCheckBox(optionsWidget);
    controls.chkStabilizeSensors->setMinimumHeight(qMax(controls.sliderSmoothnessDistance->sizeHint().height()-3,
                                                 controls.chkStabilizeSensors->sizeHint().height()));
    controls.chkStabilizeSensors->setChecked(smoothingOptions()->stabilizeSensors());
    addOptionWidgetOption(controls.chkStabilizeSensors, new QLabel(i18n("Stabilize Sensors:")));


    controls.sliderTailAggressiveness = new KisDoubleSliderSpinBox(optionsWidget);
    controls.sliderTailAggressiveness->setRange(0.0, 1.0, 2);
    controls.sliderTailAggressiveness->setSingleStep(0.01);
    controls.sliderTailAggressiveness->setEnabled(true);
    controls.sliderTailAggressiveness->setValue(smoothingOptions()->tailAggressiveness());
    addOptionWidgetOption(controls.sliderTailAggressiveness, new QLabel(i18n("Stroke Ending:")));

    controls.chkSmoothPressure = new QCheckBox(optionsWidget);
    controls.chkSmoothPressure->setMinimumHeight(qMax(controls.sliderSmoothnessDistance->sizeHint().height()-3,
                                               controls.chkSmoothPressure->sizeHint().height()));
    controls.chkSmoothPressure->setChecked(smoothingOptions()->smoothPressure());
    addOptionWidgetOption(controls.chkSmoothPressure, new QLabel(QString("%1:").arg(i18n("Smooth Pressure"))));

    controls.chkUseScalableDistance = new QCheckBox(optionsWidget);
    controls.chkUseScalableDistance->setChecked(smoothingOptions()->useScalableDistance());
    controls.chkUseScalableDistance->setMinimumHeight(qMax(controls.sliderSmoothnessDistance->sizeHint().height()-3,
                                                    controls.chkUseScalableDistance->sizeHint().height()));
    controls.chkUseScalableDistance->setToolTip(i18nc("@info:tooltip",
                                               "Scalable distance takes zoom level "
                                               "into account and makes the distance "
                                               "be visually constant whatever zoom "
                                               "level is chosen"));
    addOptionWidgetOption(controls.chkUseScalableDistance, new QLabel(QString("%1:").arg(i18n("Scalable Distance"))));

}
void KisToolBrush::slotSetSmoothingType_impl(int index, KisSmoothingOptionsSP options, smoothingOptionsControls& controls)
{
    switch (index) {
    case 0:
        options->setSmoothingType(KisSmoothingOptions::NO_SMOOTHING);
        showControl(controls.sliderSmoothnessDistance, false);
        showControl(controls.sliderTailAggressiveness, false);
        showControl(controls.chkSmoothPressure, false);
        showControl(controls.chkUseScalableDistance, false);
        showControl(controls.sliderDelayDistance, false);
        showControl(controls.chkFinishStabilizedCurve, false);
        showControl(controls.chkStabilizeSensors, false);
        break;
    case 1:
        options->setSmoothingType(KisSmoothingOptions::SIMPLE_SMOOTHING);
        showControl(controls.sliderSmoothnessDistance, false);
        showControl(controls.sliderTailAggressiveness, false);
        showControl(controls.chkSmoothPressure, false);
        showControl(controls.chkUseScalableDistance, false);
        showControl(controls.sliderDelayDistance, false);
        showControl(controls.chkFinishStabilizedCurve, false);
        showControl(controls.chkStabilizeSensors, false);
        break;
    case 2:
        options->setSmoothingType(KisSmoothingOptions::WEIGHTED_SMOOTHING);
        showControl(controls.sliderSmoothnessDistance, true);
        showControl(controls.sliderTailAggressiveness, true);
        showControl(controls.chkSmoothPressure, true);
        showControl(controls.chkUseScalableDistance, true);
        showControl(controls.sliderDelayDistance, false);
        showControl(controls.chkFinishStabilizedCurve, false);
        showControl(controls.chkStabilizeSensors, false);
        break;
    case 3:
    default:
        options->setSmoothingType(KisSmoothingOptions::STABILIZER);
        showControl(controls.sliderSmoothnessDistance, true);
        showControl(controls.sliderTailAggressiveness, false);
        showControl(controls.chkSmoothPressure, false);
        showControl(controls.sliderDelayDistance, true);
        showControl(controls.chkFinishStabilizedCurve, true);
        showControl(controls.chkStabilizeSensors, true);

        // scalable distance option is disabled due to bug 421314
        showControl(controls.chkUseScalableDistance, false);
    }
}

QList<QAction *> KisToolBrushFactory::createActionsImpl()
{
    KisActionRegistry *actionRegistry = KisActionRegistry::instance();

    QList<QAction *> actions = KisToolPaintFactoryBase::createActionsImpl();

    actions << actionRegistry->makeQAction("set_no_brush_smoothing");
    actions << actionRegistry->makeQAction("set_simple_brush_smoothing");
    actions << actionRegistry->makeQAction("set_weighted_brush_smoothing");
    actions << actionRegistry->makeQAction("set_stabilizer_brush_smoothing");
    actions << actionRegistry->makeQAction("toggle_assistant");

    return actions;

}
