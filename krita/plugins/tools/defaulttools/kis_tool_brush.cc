/*
 *  kis_tool_brush.cc - part of Krita
 *
 *  Copyright (c) 2003-2004 Boudewijn Rempt <boud@valdyas.org>
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

#include <QCheckBox>
#include <QComboBox>

#include <klocale.h>
#include <kaction.h>
#include <kactioncollection.h>

#include <KoCanvasBase.h>
#include <KoCanvasController.h>

#include "kis_cursor.h"
#include "kis_config.h"
#include "kis_slider_spin_box.h"

#define MAXIMUM_SMOOTHNESS_DISTANCE 1000.0 // 0..1000.0 == weight in gui
#define MAXIMUM_MAGNETISM 1000


void KisToolBrush::addSmoothingAction(int enumId, const QString &id, const QString &name, KActionCollection *globalCollection)
{
    /**
     * KisToolBrush is the base of several tools, but the actions
     * should be unique, so let's be careful with them
     */
    if (!globalCollection->action(id)) {
        KAction *action = new KAction(name, globalCollection);
        globalCollection->addAction(id, action);
    }

    KAction *action = dynamic_cast<KAction*>(globalCollection->action(id));
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

    addSmoothingAction(KisSmoothingOptions::NO_SMOOTHING, "set_no_brush_smoothing", i18nc("@action", "Brush Smoothing: Disabled"), collection);
    addSmoothingAction(KisSmoothingOptions::SIMPLE_SMOOTHING, "set_simple_brush_smoothing", i18nc("@action", "Brush Smoothing: Basic"), collection);
    addSmoothingAction(KisSmoothingOptions::WEIGHTED_SMOOTHING, "set_weighted_brush_smoothing", i18nc("@action", "Brush Smoothing: Weighted"), collection);
    addSmoothingAction(KisSmoothingOptions::STABILIZER, "set_stabilizer_brush_smoothing", i18nc("@action", "Brush Smoothing: Stabilizer"), collection);

}

KisToolBrush::~KisToolBrush()
{
}

void KisToolBrush::activate(ToolActivation activation, const QSet<KoShape*> &shapes)
{
    KisToolFreehand::activate(activation, shapes);
    connect(&m_signalMapper, SIGNAL(mapped(int)), SLOT(slotSetSmoothingType(int)), Qt::UniqueConnection);

    m_configGroup = KGlobal::config()->group(toolId());


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
    if (m_cmbSmoothingType->currentIndex() != index) {
        m_cmbSmoothingType->setCurrentIndex(index);
    }

    switch (index) {
    case 0:
        smoothingOptions()->setSmoothingType(KisSmoothingOptions::NO_SMOOTHING);
        showControl(m_sliderSmoothnessDistance, false);
        showControl(m_sliderTailAggressiveness, false);
        showControl(m_chkSmoothPressure, false);
        showControl(m_chkUseScalableDistance, false);
        showControl(m_sliderDelayDistance, false);
        showControl(m_chkFinishStabilizedCurve, false);
        showControl(m_chkStabilizeSensors, false);
        break;
    case 1:
        smoothingOptions()->setSmoothingType(KisSmoothingOptions::SIMPLE_SMOOTHING);
        showControl(m_sliderSmoothnessDistance, false);
        showControl(m_sliderTailAggressiveness, false);
        showControl(m_chkSmoothPressure, false);
        showControl(m_chkUseScalableDistance, false);
        showControl(m_sliderDelayDistance, false);
        showControl(m_chkFinishStabilizedCurve, false);
        showControl(m_chkStabilizeSensors, false);
        break;
    case 2:
        smoothingOptions()->setSmoothingType(KisSmoothingOptions::WEIGHTED_SMOOTHING);
        showControl(m_sliderSmoothnessDistance, true);
        showControl(m_sliderTailAggressiveness, true);
        showControl(m_chkSmoothPressure, true);
        showControl(m_chkUseScalableDistance, true);
        showControl(m_sliderDelayDistance, false);
        showControl(m_chkFinishStabilizedCurve, false);
        showControl(m_chkStabilizeSensors, false);
        break;
    case 3:
    default:
        smoothingOptions()->setSmoothingType(KisSmoothingOptions::STABILIZER);
        showControl(m_sliderSmoothnessDistance, true);
        showControl(m_sliderTailAggressiveness, false);
        showControl(m_chkSmoothPressure, false);
        showControl(m_chkUseScalableDistance, false);
        showControl(m_sliderDelayDistance, true);
        showControl(m_chkFinishStabilizedCurve, true);
        showControl(m_chkStabilizeSensors, true);
    }

    m_configGroup.writeEntry("smoothingType", index );


    emit smoothingTypeChanged();
}

void KisToolBrush::slotSetSmoothnessDistance(qreal distance)
{
    smoothingOptions()->setSmoothnessDistance(distance);
     m_configGroup.writeEntry("smoothnessDistance", distance);
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
    m_configGroup.writeEntry("weightedSmoothPressure", value);
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
    m_configGroup.writeEntry("weightedUseScalableDistance", value);

    emit useScalableDistanceChanged();
}

void KisToolBrush::resetCursorStyle()
{
    KisConfig cfg;
    enumCursorStyle cursorStyle = cfg.cursorStyle();

    // When the stabilizer is in use, we avoid using the brush outline cursor,
    // because it would hide the real position of the cursor to the user,
    // yielding unexpected results.
    if (smoothingOptions()->smoothingType() == KisSmoothingOptions::STABILIZER
        && cursorStyle == CURSOR_STYLE_OUTLINE) {
        useCursor(KisCursor::roundCursor());
    } else {
        KisToolFreehand::resetCursorStyle();
    }
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
    m_sliderDelayDistance->setEnabled(value);
    enableControl(m_chkFinishStabilizedCurve, !value);
    m_configGroup.writeEntry("stabilizerUseDelay", value);

    emit useDelayDistanceChanged();
}

void KisToolBrush::setDelayDistance(qreal value)
{
    smoothingOptions()->setDelayDistance(value);
    m_configGroup.writeEntry("stabilizerDelayDistance", value);
    emit delayDistanceChanged();
}


void KisToolBrush::setFinishStabilizedCurve(bool value)
{
    smoothingOptions()->setFinishStabilizedCurve(value);
    m_configGroup.writeEntry("stabilizerSetFinish", value);

    emit finishStabilizedCurveChanged();
}

bool KisToolBrush::finishStabilizedCurve() const
{
    return smoothingOptions()->finishStabilizedCurve();
}

void KisToolBrush::setStabilizeSensors(bool value)
{
    smoothingOptions()->setStabilizeSensors(value);
    m_configGroup.writeEntry("stabilizerSetSensors", value);

    emit stabilizeSensorsChanged();
}

bool KisToolBrush::stabilizeSensors() const
{
    return smoothingOptions()->stabilizeSensors();
}

void KisToolBrush::updateSettingsViews()
{
    m_cmbSmoothingType->setCurrentIndex(smoothingOptions()->smoothingType());
    m_sliderSmoothnessDistance->setValue(smoothingOptions()->smoothnessDistance());
    m_chkDelayDistance->setChecked(smoothingOptions()->useDelayDistance());
    m_sliderDelayDistance->setValue(smoothingOptions()->delayDistance());
    m_sliderTailAggressiveness->setValue(smoothingOptions()->tailAggressiveness());
    m_chkSmoothPressure->setChecked(smoothingOptions()->smoothPressure());
    m_chkUseScalableDistance->setChecked(smoothingOptions()->useScalableDistance());
    m_cmbSmoothingType->setCurrentIndex((int)smoothingOptions()->smoothingType());
    m_chkStabilizeSensors->setChecked(smoothingOptions()->stabilizeSensors());

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
    optionsWidget->setObjectName(toolId() + "option widget");

    // See https://bugs.kde.org/show_bug.cgi?id=316896
    QWidget *specialSpacer = new QWidget(optionsWidget);
    specialSpacer->setObjectName("SpecialSpacer");
    specialSpacer->setFixedSize(0, 0);
    optionsWidget->layout()->addWidget(specialSpacer);

    // Line smoothing configuration
    m_cmbSmoothingType = new QComboBox(optionsWidget);
    m_cmbSmoothingType->addItems(QStringList()
            << i18n("No Smoothing")
            << i18n("Basic Smoothing")
            << i18n("Weighted Smoothing")
            << i18n("Stabilizer"));
    connect(m_cmbSmoothingType, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSetSmoothingType(int)));
    addOptionWidgetOption(m_cmbSmoothingType);

    m_sliderSmoothnessDistance = new KisDoubleSliderSpinBox(optionsWidget);
    m_sliderSmoothnessDistance->setRange(3.0, MAXIMUM_SMOOTHNESS_DISTANCE, 1);
    m_sliderSmoothnessDistance->setEnabled(true);
    connect(m_sliderSmoothnessDistance, SIGNAL(valueChanged(qreal)), SLOT(slotSetSmoothnessDistance(qreal)));
    m_sliderSmoothnessDistance->setValue(smoothingOptions()->smoothnessDistance());
    addOptionWidgetOption(m_sliderSmoothnessDistance, new QLabel(i18n("Distance:")));

    // Finish stabilizer curve
    m_chkFinishStabilizedCurve = new QCheckBox("", optionsWidget);
    connect(m_chkFinishStabilizedCurve, SIGNAL(toggled(bool)), this, SLOT(setFinishStabilizedCurve(bool)));
    m_chkFinishStabilizedCurve->setChecked(smoothingOptions()->finishStabilizedCurve());

    // Delay Distance for Stabilizer
    m_chkDelayDistance = new QCheckBox(i18n("Delay:"), optionsWidget);
    m_chkDelayDistance->setToolTip(i18n("Delay the brush stroke to make the line smoother"));
    m_chkDelayDistance->setLayoutDirection(Qt::RightToLeft);
    connect(m_chkDelayDistance, SIGNAL(toggled(bool)), this, SLOT(setUseDelayDistance(bool)));
    m_sliderDelayDistance = new KisDoubleSliderSpinBox(optionsWidget);
    m_sliderDelayDistance->setToolTip(i18n("Radius where the brush is blocked"));
    m_sliderDelayDistance->setRange(0, 500);
    m_sliderDelayDistance->setSuffix(i18n(" px"));
    connect(m_sliderDelayDistance, SIGNAL(valueChanged(qreal)), SLOT(setDelayDistance(qreal)));

    addOptionWidgetOption(m_sliderDelayDistance, m_chkDelayDistance);
    addOptionWidgetOption(m_chkFinishStabilizedCurve, new QLabel(i18n("Finish line:")));

    m_sliderDelayDistance->setValue(smoothingOptions()->delayDistance());
    m_chkDelayDistance->setChecked(smoothingOptions()->useDelayDistance());
    // if the state is not flipped, then the previous line doesn't generate any signals
    setUseDelayDistance(m_chkDelayDistance->isChecked());

    // Stabilize sensors
    m_chkStabilizeSensors = new QCheckBox("", optionsWidget);
    connect(m_chkStabilizeSensors, SIGNAL(toggled(bool)), this, SLOT(setStabilizeSensors(bool)));
    m_chkStabilizeSensors->setChecked(smoothingOptions()->stabilizeSensors());
    addOptionWidgetOption(m_chkStabilizeSensors, new QLabel(i18n("Stabilize Sensors:")));


    m_sliderTailAggressiveness = new KisDoubleSliderSpinBox(optionsWidget);
    m_sliderTailAggressiveness->setRange(0.0, 1.0, 2);
    m_sliderTailAggressiveness->setEnabled(true);
    connect(m_sliderTailAggressiveness, SIGNAL(valueChanged(qreal)), SLOT(slotSetTailAgressiveness(qreal)));
    m_sliderTailAggressiveness->setValue(smoothingOptions()->tailAggressiveness());
    addOptionWidgetOption(m_sliderTailAggressiveness, new QLabel(i18n("Stroke Ending:")));

    m_chkSmoothPressure = new QCheckBox("", optionsWidget);
    m_chkSmoothPressure->setChecked(smoothingOptions()->smoothPressure());
    connect(m_chkSmoothPressure, SIGNAL(toggled(bool)), this, SLOT(setSmoothPressure(bool)));
    addOptionWidgetOption(m_chkSmoothPressure, new QLabel(i18n("Smooth Pressure")));

    m_chkUseScalableDistance = new QCheckBox("", optionsWidget);
    m_chkUseScalableDistance->setChecked(smoothingOptions()->useScalableDistance());
    connect(m_chkUseScalableDistance, SIGNAL(toggled(bool)), this, SLOT(setUseScalableDistance(bool)));
    addOptionWidgetOption(m_chkUseScalableDistance, new QLabel(i18n("Scalable Distance")));

    // Drawing assistant configuration
    m_chkAssistant = new QCheckBox(i18n("Assistant:"), optionsWidget);
    m_chkAssistant->setToolTip(i18n("You need to add Ruler Assistants before this tool will work."));
    connect(m_chkAssistant, SIGNAL(toggled(bool)), this, SLOT(setAssistant(bool)));
    m_sliderMagnetism = new KisSliderSpinBox(optionsWidget);
    m_sliderMagnetism->setToolTip(i18n("Assistant Magnetism"));
    m_sliderMagnetism->setRange(0, MAXIMUM_MAGNETISM);
    m_sliderMagnetism->setEnabled(false);
    connect(m_chkAssistant, SIGNAL(toggled(bool)), m_sliderMagnetism, SLOT(setEnabled(bool)));
    m_sliderMagnetism->setValue(m_magnetism * MAXIMUM_MAGNETISM);
    connect(m_sliderMagnetism, SIGNAL(valueChanged(int)), SLOT(slotSetMagnetism(int)));

    addOptionWidgetOption(m_sliderMagnetism, m_chkAssistant);


    //load settings from configuration kritarc file
    slotSetSmoothingType((int)m_configGroup.readEntry("smoothingType", 0));
    m_cmbSmoothingType->setCurrentIndex((int)m_configGroup.readEntry("smoothingType", 0));

        // weighted smoothing options
    setSmoothPressure((bool)m_configGroup.readEntry("weightedSmoothPressure", 0));
    setUseScalableDistance((bool)m_configGroup.readEntry("weightedUseScalableDistance", 5));

        // stabilizer smoothing options
    setFinishStabilizedCurve((bool)m_configGroup.readEntry("stabilizerSetFinish", false));
    setStabilizeSensors((bool)m_configGroup.readEntry("stabilizerSetSensors", false));
    setUseDelayDistance((bool)m_configGroup.readEntry("stabilizerUseDelay", false));
    setDelayDistance(m_configGroup.readEntry("stabilizerDelayDistance", 3));
    slotSetSmoothnessDistance(m_configGroup.readEntry("smoothnessDistance", 170.0));


    return optionsWidget;
}

#include "kis_tool_brush.moc"

