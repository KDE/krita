/*
 *  Copyright (c) 2007,2010 Cyrille Berger <cberger@cberger.net>
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

#include "kis_dynamic_sensor_time.h"

#include <kis_debug.h>

#include <QDomElement>
#include "ui_SensorTimeConfiguration.h"

#include <brushengine/kis_paint_information.h>

KisDynamicSensorTime::KisDynamicSensorTime()
    : KisDynamicSensor(TIME)
    , m_periodic(true)
{
    setLength(3);
}

qreal KisDynamicSensorTime::value(const KisPaintInformation&  pi)
{
    if (pi.isHoveringMode()) return 1.0;

    const qreal currentTime =
        m_periodic ?
        std::fmod(pi.currentTime(), m_length) :
        qMin(pi.currentTime(), qreal(m_length));

    return currentTime / qreal(m_length);
}

void KisDynamicSensorTime::reset()
{
}

void KisDynamicSensorTime::setPeriodic(bool periodic)
{
    m_periodic = periodic;
}

void KisDynamicSensorTime::setLength(qreal length)
{
    m_length = (int)(length * 1000); // convert to milliseconds
}

QWidget* KisDynamicSensorTime::createConfigurationWidget(QWidget* parent, QWidget* ss)
{
    QWidget* wdg = new QWidget(parent);
    Ui_SensorTimeConfiguration stc;
    stc.setupUi(wdg);
    stc.checkBoxRepeat->setChecked(m_periodic);
    connect(stc.checkBoxRepeat, SIGNAL(toggled(bool)), SLOT(setPeriodic(bool)));
    connect(stc.checkBoxRepeat, SIGNAL(toggled(bool)), ss, SIGNAL(parametersChanged()));

    stc.spinBoxDuration->setRange(0.02, 10.0, 2);
    stc.spinBoxDuration->setSuffix(i18n(" s"));

    stc.spinBoxDuration->setValue(m_length / 1000);
    connect(stc.spinBoxDuration, SIGNAL(valueChanged(qreal)), SLOT(setLength(qreal)));
    connect(stc.spinBoxDuration, SIGNAL(valueChanged(qreal)), ss, SIGNAL(parametersChanged()));



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

