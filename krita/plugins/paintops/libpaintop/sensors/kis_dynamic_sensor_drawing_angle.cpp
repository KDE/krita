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
#include <kis_paint_information.h>

#include <QCheckBox>
#include <QLabel>
#include <QHBoxLayout>
#include <kis_slider_spin_box.h>


KisDynamicSensorDrawingAngle::KisDynamicSensorDrawingAngle()
    : KisDynamicSensor(ANGLE),
      m_fanCornersEnabled(false),
      m_fanCornersStep(30),
      m_angleOffset(0)
{
}

qreal KisDynamicSensorDrawingAngle::value(const KisPaintInformation& info)
{
    /* so that we are in 0.0..1.0 */
    qreal ret = 0.5 + info.drawingAngle() / (2.0 * M_PI) + m_angleOffset/360.0;

    // check if m_angleOffset pushed us out of bounds
    if (ret > 1.0)
        ret -= 1.0;

    return ret;
}

bool KisDynamicSensorDrawingAngle::dependsOnCanvasRotation() const
{
    return false;
}

QWidget* KisDynamicSensorDrawingAngle::createConfigurationWidget(QWidget* parent, QWidget *ss)
{
    QWidget *w = new QWidget(parent);

    QCheckBox *fanCornersEnabled = new QCheckBox(i18n("Fan Corners"), w);

    connect(fanCornersEnabled, SIGNAL(stateChanged(int)), SLOT(setFanCornersEnabled(int)));
    connect(fanCornersEnabled, SIGNAL(stateChanged(int)), ss, SIGNAL(parametersChanged()));

    fanCornersEnabled->setChecked(m_fanCornersEnabled);

    KisSliderSpinBox *fanCornersStep = new KisSliderSpinBox(w);
    fanCornersStep->setRange(5, 90);
    fanCornersStep->setSingleStep(1);
    fanCornersStep->setSuffix(i18n("°"));

    connect(fanCornersStep, SIGNAL(valueChanged(int)), SLOT(setFanCornersStep(int)));
    connect(fanCornersStep, SIGNAL(valueChanged(int)), ss, SIGNAL(parametersChanged()));

    fanCornersStep->setValue(m_fanCornersStep);

    KisSliderSpinBox *angleOffset = new KisSliderSpinBox(w);
    angleOffset->setRange(0, 359);
    angleOffset->setSingleStep(1);
    angleOffset->setSuffix(i18n("°"));

    connect(angleOffset, SIGNAL(valueChanged(int)), SLOT(setAngleOffset(int)));
    connect(angleOffset, SIGNAL(valueChanged(int)), ss, SIGNAL(parametersChanged()));

    angleOffset->setValue(m_angleOffset);

    QVBoxLayout* l = new QVBoxLayout(w);
    l->addWidget(fanCornersEnabled);
    l->addWidget(fanCornersStep);
    l->addWidget(new QLabel(i18n("Angle Offset")));
    l->addWidget(angleOffset);

    w->setLayout(l);
    return w;
}

bool KisDynamicSensorDrawingAngle::fanCornersEnabled() const
{
    return m_fanCornersEnabled;
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

void KisDynamicSensorDrawingAngle::setAngleOffset(int angle)
{
    Q_ASSERT(angle >= 0 && angle < 360);//dont include 360
    m_angleOffset = angle;
}

void KisDynamicSensorDrawingAngle::toXML(QDomDocument &doc, QDomElement &e) const
{
    KisDynamicSensor::toXML(doc, e);
    e.setAttribute("fanCornersEnabled", m_fanCornersEnabled);
    e.setAttribute("fanCornersStep", m_fanCornersStep);
    e.setAttribute("angleOffset", m_angleOffset);
}

void KisDynamicSensorDrawingAngle::fromXML(const QDomElement &e)
{
    KisDynamicSensor::fromXML(e);
    m_fanCornersEnabled = e.attribute("fanCornersEnabled", "0").toInt();
    m_fanCornersStep = e.attribute("fanCornersStep", "30").toInt();
    m_angleOffset = e.attribute("angleOffset", "0").toInt();
}
