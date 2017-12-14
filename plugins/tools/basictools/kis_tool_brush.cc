/*
 *  kis_tool_brush.cc - part of Krita
 *
 *  Copyright (c) 2003-2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2015 Moritz Molch <kde@moritzmolch.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
#include "ui_wdgfreehandbrushoptions.h"

#define MAXIMUM_SMOOTHNESS_DISTANCE 1000.0 // 0..1000.0 == weight in gui
#define MAXIMUM_MAGNETISM 1000

void KisToolBrush::addSmoothingAction(int enumId, const QString &id, const QString &name, const QIcon &icon, KActionCollection *globalCollection)
{
    /**
     * KisToolBrush is the base of several tools, but the actions
     * should be unique, so let's be careful with them
     */
    if (!globalCollection->action(id)) {
        QAction *action = new QAction(name, globalCollection);
        action->setIcon(icon);
        globalCollection->addAction(id, action);
    }

    QAction *action = dynamic_cast<QAction*>(globalCollection->action(id));
    addAction(id, action);

    connect(action, SIGNAL(triggered()), &m_signalMapper, SLOT(map()));
    m_signalMapper.setMapping(action, enumId);
}

KisToolBrush::KisToolBrush(KoCanvasBase * canvas)
    : KisToolFreehand(canvas,
                      KisCursor::load("tool_freehand_cursor.png", 5, 5),
                      kundo2_i18n("Freehand Brush Stroke"))
{
    setObjectName("tool_brush");
    connect(this, SIGNAL(smoothingTypeChanged()), this, SLOT(resetCursorStyle()));

    KActionCollection *collection = this->canvas()->canvasController()->actionCollection();

    addSmoothingAction(KisSmoothingOptions::NO_SMOOTHING, "set_no_brush_smoothing", i18nc("@action", "Brush Smoothing: Disabled"), KisIconUtils::loadIcon("smoothing-no"), collection);
    addSmoothingAction(KisSmoothingOptions::SIMPLE_SMOOTHING, "set_simple_brush_smoothing", i18nc("@action", "Brush Smoothing: Basic"), KisIconUtils::loadIcon("smoothing-basic"), collection);
    addSmoothingAction(KisSmoothingOptions::WEIGHTED_SMOOTHING, "set_weighted_brush_smoothing", i18nc("@action", "Brush Smoothing: Weighted"), KisIconUtils::loadIcon("smoothing-weighted"), collection);
    addSmoothingAction(KisSmoothingOptions::STABILIZER, "set_stabilizer_brush_smoothing", i18nc("@action", "Brush Smoothing: Stabilizer"), KisIconUtils::loadIcon("smoothing-stabilizer"), collection);


    // add stabilizer preset options from 0 - 5
    BrushStabilizerSetting setting;

    // setting of 0 -- weakest stabilizer settings
    setting.smoothingType = 1; // basic
    stabilizerSettings.append(setting);

    // setting of 1
    setting.smoothingType = 2; // weighted smoothing
    setting.distance = 1.0;
    stabilizerSettings.append(setting);

    // setting of 2
    setting.smoothingType = 2;
    setting.distance = 3.0;
    stabilizerSettings.append(setting);

    // setting of 3
    setting.smoothingType = 3; // stabilizer
    setting.distance = 8.0;
    setting.hasDelay = true;
    setting.hasStabilizerSensors = false;
    stabilizerSettings.append(setting);


    // setting of 4 - strongest stabilizer settings
    setting.smoothingType = 3;
    setting.distance = 15.0;
    setting.hasDelay = true;
    setting.hasStabilizerSensors = true;
    stabilizerSettings.append(setting);
}

KisToolBrush::~KisToolBrush()
{
}

void KisToolBrush::activate(ToolActivation activation, const QSet<KoShape*> &shapes)
{
    KisToolFreehand::activate(activation, shapes);
    connect(&m_signalMapper, SIGNAL(mapped(int)), SLOT(slotSetSmoothingType(int)), Qt::UniqueConnection);

    m_configGroup =  KSharedConfig::openConfig()->group(toolId());
}

void KisToolBrush::deactivate()
{
    disconnect(&m_signalMapper, 0, this, 0);
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
    if (m_toolBrushOptions->smoothingTypeCombobox->currentIndex() != index) {
        m_toolBrushOptions->smoothingTypeCombobox->setCurrentIndex(index);
    }

    switch (index) {
        case 0:
            smoothingOptions()->setSmoothingType(KisSmoothingOptions::NO_SMOOTHING);
            break;
        case 1:
            smoothingOptions()->setSmoothingType(KisSmoothingOptions::SIMPLE_SMOOTHING);
            break;
        case 2:
            smoothingOptions()->setSmoothingType(KisSmoothingOptions::WEIGHTED_SMOOTHING);
            break;
        case 3:
        default:
            smoothingOptions()->setSmoothingType(KisSmoothingOptions::STABILIZER);
    }

    updateStabilizerSettingsVisibility(index);


    emit smoothingTypeChanged();
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

void KisToolBrush::slotStabilizerPresetChanged(int value)
{
    blockSignals(true);
    m_toolBrushOptions->smoothingTypeCombobox->setCurrentIndex(stabilizerSettings[value].smoothingType);
    m_toolBrushOptions->distanceInputbox->setValue(stabilizerSettings[value].distance);
    m_toolBrushOptions->strokeEndingInputbox->setValue(stabilizerSettings[value].strokeEnding);
    m_toolBrushOptions->enableStabilizerDelay->setChecked(stabilizerSettings[value].hasDelay);
    m_toolBrushOptions->stabilizerDelayInput->setValue(stabilizerSettings[value].delayDistance);
    m_toolBrushOptions->smoothPressureCheckbox->setChecked(stabilizerSettings[value].hasSmoothPressure);
    m_toolBrushOptions->scalableDistanceCheckbox->setChecked(stabilizerSettings[value].hasScalableDistance);
    m_toolBrushOptions->finishLineCheckbox->setChecked(stabilizerSettings[value].hasFinishLine);
    m_toolBrushOptions->stabilizeSensorsCheckbox->setChecked(stabilizerSettings[value].hasStabilizerSensors);
    blockSignals(false);

    // changing the UI fields causes their visibility to change
    // force hide them all since we are done changing values
    hideAllStabilizerUIFields();
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

void KisToolBrush::resetCursorStyle()
{
    KisConfig cfg;
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
    m_toolBrushOptions->stabilizerDelayInput->setEnabled(value);
    m_toolBrushOptions->finishLineCheckbox->setEnabled(!value);

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
    m_toolBrushOptions->smoothingTypeCombobox->setCurrentIndex(smoothingOptions()->smoothingType());

    m_toolBrushOptions->distanceInputbox->setValue(smoothingOptions()->smoothnessDistance());
    m_toolBrushOptions->enableStabilizerDelay->setChecked(smoothingOptions()->useDelayDistance());
    m_toolBrushOptions->distanceInputbox->setValue(smoothingOptions()->delayDistance());
    m_toolBrushOptions->strokeEndingInputbox->setValue(smoothingOptions()->tailAggressiveness());
    m_toolBrushOptions->smoothPressureCheckbox->setChecked(smoothingOptions()->smoothPressure());
    m_toolBrushOptions->scalableDistanceCheckbox->setChecked(smoothingOptions()->useScalableDistance());
    m_toolBrushOptions->smoothingTypeCombobox->setCurrentIndex((int)smoothingOptions()->smoothingType());
    m_toolBrushOptions->stabilizeSensorsCheckbox->setChecked(smoothingOptions()->stabilizeSensors());

    emit smoothnessQualityChanged();
    emit smoothnessFactorChanged();
    emit smoothPressureChanged();
    emit smoothingTypeChanged();
    emit useScalableDistanceChanged();
    emit useDelayDistanceChanged();
    emit delayDistanceChanged();
    emit finishStabilizedCurveChanged();
    emit stabilizeSensorsChanged();

    KisTool::updateSettingsViews();
}

QWidget * KisToolBrush::createOptionWidget()
{
    QWidget *optionsWidget = KisToolFreehand::createOptionWidget();

    m_toolBrushOptions = new KisToolBrushToolOptionsWidget();
    optionsWidget->layout()->addWidget(m_toolBrushOptions);

    connect(m_toolBrushOptions->customStabilizerSettingsCheckbox, SIGNAL(toggled(bool)), this, SLOT(slotCustomSettingsChecked(bool)));

    m_toolBrushOptions->stabilizerStrengthSlider->setRange(0, 4);
    //m_toolBrushOptions->stabilizerStrengthSlider->setTracking(false); // don't send signals while scrubbing, only on release
    connect(m_toolBrushOptions->stabilizerStrengthSlider, SIGNAL(valueChanged(int)), this, SLOT(slotStabilizerPresetChanged(int)));


    optionsWidget->setObjectName(toolId() + "option widget");

    // See https://bugs.kde.org/show_bug.cgi?id=316896
    QWidget *specialSpacer = new QWidget(optionsWidget);
    specialSpacer->setObjectName("SpecialSpacer");
    specialSpacer->setFixedSize(0, 0);
    optionsWidget->layout()->addWidget(specialSpacer);


    m_toolBrushOptions->smoothingTypeCombobox->addItems(QStringList()
                                                        << i18n("None")
                                                        << i18n("Basic")
                                                        << i18n("Weighted")
                                                        << i18n("Stabilizer"));
    connect(m_toolBrushOptions->smoothingTypeCombobox, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSetSmoothingType(int)));


    m_toolBrushOptions->distanceInputbox->setRange(3.0, MAXIMUM_SMOOTHNESS_DISTANCE);
    m_toolBrushOptions->distanceInputbox->setEnabled(true);
    connect(m_toolBrushOptions->distanceInputbox, SIGNAL(valueChanged(qreal)), SLOT(slotSetSmoothnessDistance(qreal)));
    m_toolBrushOptions->distanceInputbox->setValue(smoothingOptions()->smoothnessDistance());


    // Finish stabilizer curve
    connect(m_toolBrushOptions->finishLineCheckbox, SIGNAL(toggled(bool)), this, SLOT(setFinishStabilizedCurve(bool)));
    m_toolBrushOptions->finishLineCheckbox->setChecked(smoothingOptions()->finishStabilizedCurve());

    // Delay Distance for Stabilizer
    m_toolBrushOptions->enableStabilizerDelay->setToolTip(i18n("Delay the brush stroke to make the line smoother"));
    connect(m_toolBrushOptions->enableStabilizerDelay, SIGNAL(toggled(bool)), this, SLOT(setUseDelayDistance(bool)));


    m_toolBrushOptions->distanceInputbox->setToolTip(i18n("Radius where the brush is blocked"));
    m_toolBrushOptions->distanceInputbox->setRange(0, 500);
    m_toolBrushOptions->distanceInputbox->setSuffix(i18n(" px"));
    connect(m_toolBrushOptions->distanceInputbox, SIGNAL(valueChanged(qreal)), SLOT(setDelayDistance(qreal)));

    m_toolBrushOptions->distanceInputbox->setValue(smoothingOptions()->delayDistance());
    m_toolBrushOptions->enableStabilizerDelay->setChecked(smoothingOptions()->useDelayDistance());

    // if the state is not flipped, then the previous line doesn't generate any signals
    setUseDelayDistance(m_toolBrushOptions->enableStabilizerDelay->isChecked());

    // Stabilize sensors
    connect(m_toolBrushOptions->stabilizeSensorsCheckbox, SIGNAL(toggled(bool)), this, SLOT(setStabilizeSensors(bool)));
    m_toolBrushOptions->stabilizeSensorsCheckbox->setChecked(smoothingOptions()->stabilizeSensors());

    m_toolBrushOptions->strokeEndingInputbox->setRange(0.0, 1.0);
    m_toolBrushOptions->strokeEndingInputbox->setEnabled(true);
    connect(m_toolBrushOptions->strokeEndingInputbox, SIGNAL(valueChanged(qreal)), SLOT(slotSetTailAgressiveness(qreal)));
    m_toolBrushOptions->strokeEndingInputbox->setValue(smoothingOptions()->tailAggressiveness());

    m_toolBrushOptions->smoothPressureCheckbox->setChecked(smoothingOptions()->smoothPressure());

    connect(m_toolBrushOptions->smoothPressureCheckbox, SIGNAL(toggled(bool)), this, SLOT(setSmoothPressure(bool)));

    m_toolBrushOptions->scalableDistanceCheckbox->setChecked(smoothingOptions()->useScalableDistance());

    m_toolBrushOptions->scalableDistanceCheckbox->setToolTip(i18nc("@info:tooltip",
                                               "Scalable distance takes zoom level "
                                               "into account and makes the distance "
                                               "be visually constant whatever zoom "
                                               "level is chosen"));
    connect(m_toolBrushOptions->scalableDistanceCheckbox, SIGNAL(toggled(bool)), this, SLOT(setUseScalableDistance(bool)));




    // Drawing assistant configuration

    // allow  snapping to assistants be assigned to a shortcut
    QAction *toggleaction =  KisActionRegistry::instance()->makeQAction("toggle_assistant", this);
    addAction("toggle_assistant", toggleaction);
    toggleaction->setShortcut(QKeySequence(Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_L));
    connect(toggleaction, SIGNAL(triggered(bool)), m_toolBrushOptions->useAssistantsCheckbox, SLOT(toggle()));

    m_toolBrushOptions->magnetismInputbox->setToolTip(i18n("Assistant Magnetism"));
    m_toolBrushOptions->magnetismInputbox->setRange(0, MAXIMUM_MAGNETISM);

     m_toolBrushOptions->magnetismInputbox->setValue(m_magnetism * MAXIMUM_MAGNETISM);
    connect( m_toolBrushOptions->magnetismInputbox, SIGNAL(valueChanged(int)), SLOT(slotSetMagnetism(int)));

    m_toolBrushOptions->snapSingleCheckbox->setToolTip(i18nc("@info:tooltip","Make it only snap to a single assistant, prevents snapping mess while using the infinite assistants."));
    m_toolBrushOptions->snapSingleCheckbox->setCheckState(Qt::Checked);//turn on by default.
    connect(m_toolBrushOptions->snapSingleCheckbox, SIGNAL(toggled(bool)), this, SLOT(setOnlyOneAssistantSnap(bool)));


    connect(m_toolBrushOptions->useAssistantsCheckbox, SIGNAL(toggled(bool)), this, SLOT(slotUseAssistants(bool)));

    KisConfig cfg;
    slotSetSmoothingType(cfg.lineSmoothingType());


    // add a line spacer so we know that the next set of options are for different settings
    QFrame* line = new QFrame(optionsWidget);
    line->setObjectName(QString::fromUtf8("line"));
    line->setFrameShape(QFrame::HLine);
    addOptionWidgetOption(line);


    // hide all the custom stabilizer settings by default
    slotCustomSettingsChecked(false);
    slotUseAssistants(false); // initialize to not snap to assistants

    return optionsWidget;
}

void KisToolBrush::slotCustomSettingsChecked(bool checked)
{
    // toggle on/off the smoothing type combo box part and disable/enable the slider
    m_toolBrushOptions->smoothTypeLabel->setVisible(checked);
    m_toolBrushOptions->smoothingTypeCombobox->setVisible(checked);
    m_toolBrushOptions->stabilizerStrengthSlider->setEnabled(!checked);

    // hide all settings if we are not using custom settings
    if (!checked) {
        hideAllStabilizerUIFields();
    } else {
        updateStabilizerSettingsVisibility(m_toolBrushOptions->smoothingTypeCombobox->currentIndex());
    }

}

void KisToolBrush::hideAllStabilizerUIFields()
{
    m_toolBrushOptions->distanceLabel->setVisible(false);
    m_toolBrushOptions->distanceInputbox->setVisible(false);
    m_toolBrushOptions->strokeEndingLabel->setVisible(false);
    m_toolBrushOptions->strokeEndingInputbox->setVisible(false);
    m_toolBrushOptions->enableStabilizerDelay->setVisible(false);
    m_toolBrushOptions->stabilizerDelayInput->setVisible(false);

    m_toolBrushOptions->smoothPressureCheckbox->setVisible(false);
    m_toolBrushOptions->scalableDistanceCheckbox->setVisible(false);
    m_toolBrushOptions->finishLineCheckbox->setVisible(false);
    m_toolBrushOptions->stabilizeSensorsCheckbox->setVisible(false);
}

void KisToolBrush::updateStabilizerSettingsVisibility(int smoothingTypeIndex) {

    if (!m_toolBrushOptions) {
        return;
    }

    switch (smoothingTypeIndex) {
    case 0:
        m_toolBrushOptions->distanceLabel->setVisible(false);
        m_toolBrushOptions->distanceInputbox->setVisible(false);
        m_toolBrushOptions->strokeEndingLabel->setVisible(false);
        m_toolBrushOptions->strokeEndingInputbox->setVisible(false);
        m_toolBrushOptions->enableStabilizerDelay->setVisible(false);
        m_toolBrushOptions->stabilizerDelayInput->setVisible(false);

        m_toolBrushOptions->smoothPressureCheckbox->setVisible(false);
        m_toolBrushOptions->scalableDistanceCheckbox->setVisible(false);
        m_toolBrushOptions->finishLineCheckbox->setVisible(false);
        m_toolBrushOptions->stabilizeSensorsCheckbox->setVisible(false);

        break;
    case 1:
        m_toolBrushOptions->distanceLabel->setVisible(false);
        m_toolBrushOptions->distanceInputbox->setVisible(false);
        m_toolBrushOptions->strokeEndingLabel->setVisible(false);
        m_toolBrushOptions->strokeEndingInputbox->setVisible(false);
        m_toolBrushOptions->enableStabilizerDelay->setVisible(false);
        m_toolBrushOptions->stabilizerDelayInput->setVisible(false);

        m_toolBrushOptions->smoothPressureCheckbox->setVisible(false);
        m_toolBrushOptions->scalableDistanceCheckbox->setVisible(false);
        m_toolBrushOptions->finishLineCheckbox->setVisible(false);
        m_toolBrushOptions->stabilizeSensorsCheckbox->setVisible(false);

        break;
    case 2:
        m_toolBrushOptions->distanceLabel->setVisible(true);
        m_toolBrushOptions->distanceInputbox->setVisible(true);
        m_toolBrushOptions->strokeEndingLabel->setVisible(true);
        m_toolBrushOptions->strokeEndingInputbox->setVisible(true);
        m_toolBrushOptions->enableStabilizerDelay->setVisible(false);
        m_toolBrushOptions->stabilizerDelayInput->setVisible(false);

        m_toolBrushOptions->smoothPressureCheckbox->setVisible(true);
        m_toolBrushOptions->scalableDistanceCheckbox->setVisible(true);
        m_toolBrushOptions->finishLineCheckbox->setVisible(false);
        m_toolBrushOptions->stabilizeSensorsCheckbox->setVisible(false);

        break;
    case 3:
    default:
        m_toolBrushOptions->distanceLabel->setVisible(true);
        m_toolBrushOptions->distanceInputbox->setVisible(true);
        m_toolBrushOptions->strokeEndingLabel->setVisible(false);
        m_toolBrushOptions->strokeEndingInputbox->setVisible(false);
        m_toolBrushOptions->enableStabilizerDelay->setVisible(true);
        m_toolBrushOptions->stabilizerDelayInput->setVisible(true);

        m_toolBrushOptions->smoothPressureCheckbox->setVisible(true);
        m_toolBrushOptions->scalableDistanceCheckbox->setVisible(false);
        m_toolBrushOptions->finishLineCheckbox->setVisible(true);
        m_toolBrushOptions->stabilizeSensorsCheckbox->setVisible(true);
    }
}

void KisToolBrush::slotUseAssistants(bool isUsing)
{
    // hide UI elements if we are not snapping to assistants
    m_toolBrushOptions->magnetismLabel->setVisible(isUsing);
    m_toolBrushOptions->magnetismInputbox->setVisible(isUsing);
    m_toolBrushOptions->snapSingleCheckbox->setVisible(isUsing);

    setAssistant(isUsing);

}
