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
        showControl(m_chkUseScalableDistance, true);
        showControl(m_sliderDelayDistance, true);
        showControl(m_chkFinishStabilizedCurve, true);
        showControl(m_chkStabilizeSensors, true);
    }

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
    m_sliderDelayDistance->setEnabled(value);
    enableControl(m_chkFinishStabilizedCurve, !value);

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
            << i18n("None")
            << i18n("Basic")
            << i18n("Weighted")
            << i18n("Stabilizer"));
    connect(m_cmbSmoothingType, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSetSmoothingType(int)));
    addOptionWidgetOption(m_cmbSmoothingType, new QLabel(i18n("Brush Smoothing:")));

    m_sliderSmoothnessDistance = new KisDoubleSliderSpinBox(optionsWidget);
    m_sliderSmoothnessDistance->setRange(3.0, MAXIMUM_SMOOTHNESS_DISTANCE, 1);
    m_sliderSmoothnessDistance->setExponentRatio(3.0); // help pick smaller values


    m_sliderSmoothnessDistance->setEnabled(true);
    connect(m_sliderSmoothnessDistance, SIGNAL(valueChanged(qreal)), SLOT(slotSetSmoothnessDistance(qreal)));
    m_sliderSmoothnessDistance->setValue(smoothingOptions()->smoothnessDistance());
    addOptionWidgetOption(m_sliderSmoothnessDistance, new QLabel(i18n("Distance:")));

    // Finish stabilizer curve
    m_chkFinishStabilizedCurve = new QCheckBox(optionsWidget);
    m_chkFinishStabilizedCurve->setMinimumHeight(qMax(m_sliderSmoothnessDistance->sizeHint().height()-3,
                                                      m_chkFinishStabilizedCurve->sizeHint().height()));
    connect(m_chkFinishStabilizedCurve, SIGNAL(toggled(bool)), this, SLOT(setFinishStabilizedCurve(bool)));
    m_chkFinishStabilizedCurve->setChecked(smoothingOptions()->finishStabilizedCurve());

    // Delay Distance for Stabilizer
    QWidget* delayWidget = new QWidget(optionsWidget);
    QHBoxLayout* delayLayout = new QHBoxLayout(delayWidget);
    delayLayout->setContentsMargins(0,0,0,0);
    delayLayout->setSpacing(1);
    QLabel* delayLabel = new QLabel(i18n("Delay:"), optionsWidget);
    delayLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    delayLayout->addWidget(delayLabel);
    m_chkDelayDistance = new QCheckBox(optionsWidget);
    m_chkDelayDistance->setLayoutDirection(Qt::RightToLeft);
    delayWidget->setToolTip(i18n("Delay the brush stroke to make the line smoother"));
    connect(m_chkDelayDistance, SIGNAL(toggled(bool)), this, SLOT(setUseDelayDistance(bool)));
    delayLayout->addWidget(m_chkDelayDistance);
    m_sliderDelayDistance = new KisDoubleSliderSpinBox(optionsWidget);
    m_sliderDelayDistance->setToolTip(i18n("Radius where the brush is blocked"));
    m_sliderDelayDistance->setRange(0, 500);
    m_sliderDelayDistance->setExponentRatio(3.0); // help pick smaller values
    m_sliderDelayDistance->setSuffix(i18n(" px"));
    connect(m_sliderDelayDistance, SIGNAL(valueChanged(qreal)), SLOT(setDelayDistance(qreal)));

    addOptionWidgetOption(m_sliderDelayDistance, delayWidget);
    addOptionWidgetOption(m_chkFinishStabilizedCurve, new QLabel(i18n("Finish line:")));

    m_sliderDelayDistance->setValue(smoothingOptions()->delayDistance());
    m_chkDelayDistance->setChecked(smoothingOptions()->useDelayDistance());
    // if the state is not flipped, then the previous line doesn't generate any signals
    setUseDelayDistance(m_chkDelayDistance->isChecked());

    // Stabilize sensors
    m_chkStabilizeSensors = new QCheckBox(optionsWidget);
    m_chkStabilizeSensors->setMinimumHeight(qMax(m_sliderSmoothnessDistance->sizeHint().height()-3,
                                                 m_chkStabilizeSensors->sizeHint().height()));
    connect(m_chkStabilizeSensors, SIGNAL(toggled(bool)), this, SLOT(setStabilizeSensors(bool)));
    m_chkStabilizeSensors->setChecked(smoothingOptions()->stabilizeSensors());
    addOptionWidgetOption(m_chkStabilizeSensors, new QLabel(i18n("Stabilize Sensors:")));


    m_sliderTailAggressiveness = new KisDoubleSliderSpinBox(optionsWidget);
    m_sliderTailAggressiveness->setRange(0.0, 1.0, 2);
    m_sliderTailAggressiveness->setEnabled(true);
    connect(m_sliderTailAggressiveness, SIGNAL(valueChanged(qreal)), SLOT(slotSetTailAgressiveness(qreal)));
    m_sliderTailAggressiveness->setValue(smoothingOptions()->tailAggressiveness());
    addOptionWidgetOption(m_sliderTailAggressiveness, new QLabel(i18n("Stroke Ending:")));

    m_chkSmoothPressure = new QCheckBox(optionsWidget);
    m_chkSmoothPressure->setMinimumHeight(qMax(m_sliderSmoothnessDistance->sizeHint().height()-3,
                                               m_chkSmoothPressure->sizeHint().height()));
    m_chkSmoothPressure->setChecked(smoothingOptions()->smoothPressure());
    connect(m_chkSmoothPressure, SIGNAL(toggled(bool)), this, SLOT(setSmoothPressure(bool)));
    addOptionWidgetOption(m_chkSmoothPressure, new QLabel(QString("%1:").arg(i18n("Smooth Pressure"))));

    m_chkUseScalableDistance = new QCheckBox(optionsWidget);
    m_chkUseScalableDistance->setChecked(smoothingOptions()->useScalableDistance());
    m_chkUseScalableDistance->setMinimumHeight(qMax(m_sliderSmoothnessDistance->sizeHint().height()-3,
                                                    m_chkUseScalableDistance->sizeHint().height()));
    m_chkUseScalableDistance->setToolTip(i18nc("@info:tooltip",
                                               "Scalable distance takes zoom level "
                                               "into account and makes the distance "
                                               "be visually constant whatever zoom "
                                               "level is chosen"));
    connect(m_chkUseScalableDistance, SIGNAL(toggled(bool)), this, SLOT(setUseScalableDistance(bool)));
    addOptionWidgetOption(m_chkUseScalableDistance, new QLabel(QString("%1:").arg(i18n("Scalable Distance"))));


    // add a line spacer so we know that the next set of options are for different settings
    QFrame* line = new QFrame(optionsWidget);
    line->setObjectName(QString::fromUtf8("line"));
    line->setFrameShape(QFrame::HLine);
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

    QAction *toggleaction =  KisActionRegistry::instance()->makeQAction("toggle_assistant", this);
    addAction("toggle_assistant", toggleaction);
    toggleaction->setShortcut(QKeySequence(Qt::ControlModifier + Qt::ShiftModifier + Qt::Key_L));
    connect(toggleaction, SIGNAL(triggered(bool)), m_chkAssistant, SLOT(toggle()));

    QLabel* magnetismLabel = new QLabel(i18n("Magnetism:"));
    addOptionWidgetOption(m_sliderMagnetism, magnetismLabel);

    QLabel* snapSingleLabel = new QLabel(i18n("Snap Single:"));

    m_chkOnlyOneAssistant = new QCheckBox(optionsWidget);
    m_chkOnlyOneAssistant->setToolTip(i18nc("@info:tooltip","Make it only snap to a single assistant, prevents snapping mess while using the infinite assistants."));
    m_chkOnlyOneAssistant->setCheckState(Qt::Checked);//turn on by default.
    connect(m_chkOnlyOneAssistant, SIGNAL(toggled(bool)), this, SLOT(setOnlyOneAssistantSnap(bool)));
    addOptionWidgetOption(m_chkOnlyOneAssistant, snapSingleLabel);


    // set the assistant snapping options to hidden by default and toggle their visibility based based off snapping checkbox
    m_sliderMagnetism->setVisible(false);
    m_chkOnlyOneAssistant->setVisible(false);
    snapSingleLabel->setVisible(false);
    magnetismLabel->setVisible(false);

   connect(m_chkAssistant, SIGNAL(toggled(bool)), m_sliderMagnetism, SLOT(setVisible(bool)));
   connect(m_chkAssistant, SIGNAL(toggled(bool)), m_chkOnlyOneAssistant, SLOT(setVisible(bool)));
   connect(m_chkAssistant, SIGNAL(toggled(bool)), snapSingleLabel, SLOT(setVisible(bool)));
    connect(m_chkAssistant, SIGNAL(toggled(bool)), magnetismLabel, SLOT(setVisible(bool)));


    KisConfig cfg;
    slotSetSmoothingType(cfg.lineSmoothingType());

    return optionsWidget;
}


