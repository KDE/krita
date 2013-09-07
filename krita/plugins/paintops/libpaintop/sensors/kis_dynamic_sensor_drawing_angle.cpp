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
#include <QHBoxLayout>
#include <kis_slider_spin_box.h>


KisDynamicSensorDrawingAngle::KisDynamicSensorDrawingAngle()
    : KisDynamicSensor(DrawingAngleId),
      m_fanCornersEnabled(false),
      m_fanCornersStep(30)
{
    setMinimumLabel(i18n("0°"));
    setMaximumLabel(i18n("360°"));
}

qreal KisDynamicSensorDrawingAngle::value(const KisPaintInformation& info)
{
    /* so that we are in 0.0..1.0 */
    return 0.5 + info.drawingAngle() / (2.0 * M_PI);
}

bool KisDynamicSensorDrawingAngle::dependsOnCanvasRotation() const
{
    return false;
}

QWidget* KisDynamicSensorDrawingAngle::createConfigurationWidget(QWidget* parent, QWidget *ss)
{
    QWidget *w = new QWidget(parent);

    QCheckBox *fanCornersEnabled = new QCheckBox(i18n("Fan Corners"), w);

    KisSliderSpinBox *fanCornersStep = new KisSliderSpinBox(w);
    fanCornersStep->setRange(0, 90);
    fanCornersStep->setSingleStep(1);
    fanCornersStep->setSuffix(i18n("°"));

    connect(fanCornersEnabled, SIGNAL(stateChanged(int)), SLOT(setFanCornersEnabled(int)));
    connect(fanCornersStep, SIGNAL(valueChanged(int)), SLOT(setFanCornersStep(int)));
    connect(fanCornersEnabled, SIGNAL(stateChanged(int)), ss, SIGNAL(parametersChanged()));
    connect(fanCornersStep, SIGNAL(valueChanged(int)), ss, SIGNAL(parametersChanged()));

    fanCornersEnabled->setChecked(m_fanCornersEnabled);
    fanCornersStep->setValue(m_fanCornersStep);

    QVBoxLayout* l = new QVBoxLayout(w);
    l->addWidget(fanCornersEnabled);
    l->addWidget(fanCornersStep);

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

void KisDynamicSensorDrawingAngle::setFanCornersEnabled(int state)
{
    m_fanCornersEnabled = state;
}

void KisDynamicSensorDrawingAngle::setFanCornersStep(int angle)
{
    m_fanCornersStep = angle;
}

void KisDynamicSensorDrawingAngle::toXML(QDomDocument &doc, QDomElement &e) const
{
    KisDynamicSensor::toXML(doc, e);
    e.setAttribute("fanCornersEnabled", m_fanCornersEnabled);
    e.setAttribute("fanCornersStep", m_fanCornersStep);
}

void KisDynamicSensorDrawingAngle::fromXML(const QDomElement &e)
{
    KisDynamicSensor::fromXML(e);
    m_fanCornersEnabled = e.attribute("fanCornersEnabled", "0").toInt();
    m_fanCornersStep = e.attribute("fanCornersStep", "30").toInt();
}
