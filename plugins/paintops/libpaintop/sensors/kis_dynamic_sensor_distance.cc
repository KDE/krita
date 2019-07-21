/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_dynamic_sensor_distance.h"

#include <kis_debug.h>

#include <QDomElement>
#include "ui_SensorDistanceConfiguration.h"

#include <brushengine/kis_paint_information.h>

KisDynamicSensorDistance::KisDynamicSensorDistance()
    : KisDynamicSensor(DISTANCE)
    , m_periodic(true)
{
    setLength(30);
}

qreal KisDynamicSensorDistance::value(const KisPaintInformation&  pi)
{
    if (pi.isHoveringMode()) return 1.0;


    const qreal distance =
        m_periodic ?
        fmod(pi.totalStrokeLength(), m_length) :
        qMin(pi.totalStrokeLength(), (qreal)m_length);

    return distance / m_length;
}

void KisDynamicSensorDistance::setPeriodic(bool periodic)
{
    m_periodic = periodic;
}

void KisDynamicSensorDistance::setLength(int length)
{
    m_length = length;
}

QWidget* KisDynamicSensorDistance::createConfigurationWidget(QWidget* parent, QWidget* ss)
{
    QWidget* wdg = new QWidget(parent);
    Ui_SensorDistanceConfiguration stc;
    stc.setupUi(wdg);
    stc.checkBoxRepeat->setChecked(m_periodic);
    connect(stc.checkBoxRepeat, SIGNAL(toggled(bool)), SLOT(setPeriodic(bool)));
    connect(stc.checkBoxRepeat, SIGNAL(toggled(bool)), ss, SIGNAL(parametersChanged()));
    stc.spinBoxLength->setValue(m_length);
    connect(stc.spinBoxLength, SIGNAL(valueChanged(int)), SLOT(setLength(int)));
    connect(stc.spinBoxLength, SIGNAL(valueChanged(int)), ss, SIGNAL(parametersChanged()));
    return wdg;
}

void KisDynamicSensorDistance::toXML(QDomDocument& doc, QDomElement& e) const
{
    KisDynamicSensor::toXML(doc, e);
    e.setAttribute("periodic", m_periodic);
    e.setAttribute("length", m_length);
}

void KisDynamicSensorDistance::fromXML(const QDomElement& e)
{
    KisDynamicSensor::fromXML(e);
    m_periodic = e.attribute("periodic", "0").toInt();
    m_length = e.attribute("length", "30").toInt();
}

