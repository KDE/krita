/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
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
#include "kis_sensor_selector.h"

KisDynamicSensorTime::KisDynamicSensorTime() : KisDynamicSensor(TimeId), m_time(0.0), m_length(30), m_periodic(true), m_previousPos(KisVector2D::Zero())
{

}

double KisDynamicSensorTime::parameter(const KisPaintInformation&  pi)
{
    m_time += pi.movement().norm();
    if (m_time > m_length) {
        if (m_periodic) {
            do {
                m_time -= m_length; // TODO: replace by a modulo when I am a little bit more awake
            } while (m_time > m_length);
        } else {
            m_time = m_length;
        }
    }
    return 1.0 - m_time / m_length;
}

void KisDynamicSensorTime::reset()
{
    m_time = 0;
}

void KisDynamicSensorTime::setPeriodic(bool periodic)
{
    m_periodic = periodic;
}

void KisDynamicSensorTime::setLength(int length)
{
    m_length = length;
}

QWidget* KisDynamicSensorTime::createConfigurationWidget(QWidget* parent, KisSensorSelector* ss)
{
    QWidget* wdg = new QWidget(parent);
    Ui_SensorTimeConfiguration stc;
    stc.setupUi(wdg);
    stc.checkBoxRepeat->setChecked(m_periodic);
    connect(stc.checkBoxRepeat, SIGNAL(toggled(bool)), SLOT(setPeriodic(bool)));
    connect(stc.checkBoxRepeat, SIGNAL(toggled(bool)), ss, SIGNAL(parametersChanged()));
    stc.spinBoxLength->setValue(m_length);
    connect(stc.spinBoxLength, SIGNAL(valueChanged(int)), SLOT(setLength(int)));
    connect(stc.spinBoxLength, SIGNAL(valueChanged(int)), ss, SIGNAL(parametersChanged()));
    return wdg;
}

void KisDynamicSensorTime::toXML(QDomDocument& doc, QDomElement& e) const
{
    KisDynamicSensor::toXML(doc, e);
    e.setAttribute("periodic", m_periodic);
    e.setAttribute("length", m_length);
}

void KisDynamicSensorTime::fromXML(const QDomElement& e)
{
    m_periodic = e.attribute("periodic", "0").toInt();
    m_length = e.attribute("length", "30").toInt();
}

#include "kis_dynamic_sensor_time.moc"
