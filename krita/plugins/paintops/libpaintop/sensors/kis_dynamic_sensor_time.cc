/*
 *  Copyright (c) 2007,2010 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
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

#include "kis_dynamic_sensor_time.h"

#include <kis_debug.h>

#include <QDomElement>
#include "ui_SensorTimeConfiguration.h"

#include "kis_paint_information.h"

KisDynamicSensorTime::KisDynamicSensorTime()
    : KisDynamicSensor(TIME)
    , m_time(0)
    , m_periodic(true)
    , m_lastTime(0)
{
    setLength(3);
}

qreal KisDynamicSensorTime::value(const KisPaintInformation&  pi)
{
    qreal curtime = pi.currentTime();

    if (curtime >= m_lastTime) {
        m_time += curtime - m_lastTime;
    } else {
        // safely handle the situation when currentTime() < m_lastTime
        m_time = 0;
    }

    m_lastTime = curtime;

    if (m_time > m_length) {
        if (m_periodic) {
            m_time = m_time % m_length;
        }
        else {
            m_time = m_length;
        }
    }
    return 1.0 - m_time / qreal(m_length);
}

void KisDynamicSensorTime::reset()
{
    m_lastTime = 0;
    m_time = 0;
}

void KisDynamicSensorTime::setPeriodic(bool periodic)
{
    m_periodic = periodic;
}

void KisDynamicSensorTime::setLength(int length)
{
    m_length = length * 1000;
}

QWidget* KisDynamicSensorTime::createConfigurationWidget(QWidget* parent, QWidget* ss)
{
    QWidget* wdg = new QWidget(parent);
    Ui_SensorTimeConfiguration stc;
    stc.setupUi(wdg);
    stc.checkBoxRepeat->setChecked(m_periodic);
    connect(stc.checkBoxRepeat, SIGNAL(toggled(bool)), SLOT(setPeriodic(bool)));
    connect(stc.checkBoxRepeat, SIGNAL(toggled(bool)), ss, SIGNAL(parametersChanged()));
    stc.spinBoxDuration->setValue(m_length / 1000);
    connect(stc.spinBoxDuration, SIGNAL(valueChanged(int)), SLOT(setLength(int)));
    connect(stc.spinBoxDuration, SIGNAL(valueChanged(int)), ss, SIGNAL(parametersChanged()));
    return wdg;
}

void KisDynamicSensorTime::toXML(QDomDocument& doc, QDomElement& e) const
{
    KisDynamicSensor::toXML(doc, e);
    e.setAttribute("periodic", m_periodic);
    e.setAttribute("duration", m_length);
}

void KisDynamicSensorTime::fromXML(const QDomElement& e)
{
    KisDynamicSensor::fromXML(e);
    m_periodic = e.attribute("periodic", "0").toInt();
    m_length = e.attribute("duration", "30").toInt();
}

