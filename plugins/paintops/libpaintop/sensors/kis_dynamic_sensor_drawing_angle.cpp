/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_dynamic_sensor_drawing_angle.h"

#include <QDomElement>
#include <brushengine/kis_paint_information.h>

#include <QCheckBox>
#include <QLabel>
#include <QHBoxLayout>
#include <kis_slider_spin_box.h>


KisDynamicSensorDrawingAngle::KisDynamicSensorDrawingAngle()
    : KisDynamicSensor(ANGLE),
      m_fanCornersEnabled(false),
      m_fanCornersStep(30),
      m_angleOffset(0),
      m_lockedAngle(0),
      m_lockedAngleMode(false)
{
}

void KisDynamicSensorDrawingAngle::reset()
{
}

qreal KisDynamicSensorDrawingAngle::value(const KisPaintInformation& info)
{
    /* so that we are in 0.0..1.0 */
    qreal ret = 0.5 + info.drawingAngle(m_lockedAngleMode) / (2.0 * M_PI) + m_angleOffset/360.0;

    // check if m_angleOffset pushed us out of bounds
    if (ret > 1.0)
        ret -= 1.0;

    return ret;
}

bool KisDynamicSensorDrawingAngle::dependsOnCanvasRotation() const
{
    return false;
}

bool KisDynamicSensorDrawingAngle::isAbsoluteRotation() const
{
    return true;
}

void KisDynamicSensorDrawingAngle::updateGUI()
{
    const bool fanEnabled = !m_chkLockedMode->isChecked();

    m_chkFanCorners->setEnabled(fanEnabled);
    m_intFanCornersStep->setEnabled(fanEnabled);
}

QWidget* KisDynamicSensorDrawingAngle::createConfigurationWidget(QWidget* parent, QWidget *ss)
{
    QWidget *mainWidget = new QWidget(parent);
    m_chkLockedMode = new QCheckBox(i18n("Locked Angle"), mainWidget);
    m_chkLockedMode->setChecked(m_lockedAngleMode);

    connect(m_chkLockedMode, SIGNAL(stateChanged(int)), SLOT(setLockedAngleMode(int)));
    connect(m_chkLockedMode, SIGNAL(stateChanged(int)), SLOT(updateGUI()));

    m_chkFanCorners = new QCheckBox(i18n("Fan Corners"), mainWidget);

    connect(m_chkFanCorners, SIGNAL(stateChanged(int)), SLOT(setFanCornersEnabled(int)));

    m_chkFanCorners->setChecked(m_fanCornersEnabled);

    m_intFanCornersStep = new KisSliderSpinBox(mainWidget);
     m_intFanCornersStep->setPrefix(i18n("Fan Corners Angle:"));
    m_intFanCornersStep->setRange(5, 90);
    m_intFanCornersStep->setSingleStep(1);
    m_intFanCornersStep->setSuffix(i18n("°"));

    connect(m_intFanCornersStep, SIGNAL(valueChanged(int)), SLOT(setFanCornersStep(int)));

    m_intFanCornersStep->setValue(m_fanCornersStep);

    KisSliderSpinBox *angleOffset = new KisSliderSpinBox(mainWidget);
    angleOffset->setRange(0, 359);
    angleOffset->setSingleStep(1);
    angleOffset->setPrefix(i18n("Angle Offset:"));
    angleOffset->setSuffix(i18n("°"));

    connect(angleOffset, SIGNAL(valueChanged(int)), SLOT(setAngleOffset(int)));

    angleOffset->setValue(m_angleOffset);

    QVBoxLayout* optionsLayout = new QVBoxLayout(mainWidget);
    optionsLayout->setContentsMargins(0,0,0,0);

    QHBoxLayout* angleModeLayout = new QHBoxLayout(mainWidget);
    angleModeLayout->addWidget(m_chkLockedMode);
    angleModeLayout->addWidget(m_chkFanCorners);

    optionsLayout->addLayout(angleModeLayout);
    optionsLayout->addWidget(m_intFanCornersStep);
    optionsLayout->addWidget(angleOffset);

    updateGUI();

    connect(angleOffset, SIGNAL(valueChanged(int)), ss, SIGNAL(parametersChanged()));
    connect(m_chkLockedMode, SIGNAL(stateChanged(int)), ss, SIGNAL(parametersChanged()));
    connect(m_chkFanCorners, SIGNAL(stateChanged(int)), ss, SIGNAL(parametersChanged()));
    connect(m_intFanCornersStep, SIGNAL(valueChanged(int)), ss, SIGNAL(parametersChanged()));

    mainWidget->setLayout(optionsLayout);
    return mainWidget;
}

bool KisDynamicSensorDrawingAngle::fanCornersEnabled() const
{
    return m_fanCornersEnabled && !m_lockedAngleMode;
}

int KisDynamicSensorDrawingAngle::fanCornersStep() const
{
    return m_fanCornersStep;
}

int KisDynamicSensorDrawingAngle::angleOffset() const
{
    return m_angleOffset;
}

void KisDynamicSensorDrawingAngle::setFanCornersEnabled(int state)
{
    m_fanCornersEnabled = state;
}

void KisDynamicSensorDrawingAngle::setFanCornersStep(int angle)
{
    m_fanCornersStep = angle;
}

void KisDynamicSensorDrawingAngle::setLockedAngleMode(int value)
{
    m_lockedAngleMode = value;
}

void KisDynamicSensorDrawingAngle::setAngleOffset(int angle)
{
    Q_ASSERT(angle >= 0 && angle < 360);//don't include 360
    m_angleOffset = angle;
}

void KisDynamicSensorDrawingAngle::toXML(QDomDocument &doc, QDomElement &e) const
{
    KisDynamicSensor::toXML(doc, e);
    e.setAttribute("fanCornersEnabled", m_fanCornersEnabled);
    e.setAttribute("fanCornersStep", m_fanCornersStep);
    e.setAttribute("angleOffset", m_angleOffset);
    e.setAttribute("lockedAngleMode", m_lockedAngleMode);
}

void KisDynamicSensorDrawingAngle::fromXML(const QDomElement &e)
{
    KisDynamicSensor::fromXML(e);
    m_fanCornersEnabled = e.attribute("fanCornersEnabled", "0").toInt();
    m_fanCornersStep = e.attribute("fanCornersStep", "30").toInt();
    m_angleOffset = e.attribute("angleOffset", "0").toInt();
    m_lockedAngleMode = e.attribute("lockedAngleMode", "0").toInt();
}
