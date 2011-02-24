/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_dynamic_sensor_fade.h"

#include <kis_debug.h>

#include <QDomElement>
#include "ui_SensorFadeConfiguration.h"

#include "kis_paint_information.h"
#include "kis_sensor_selector.h"

static const int DEFAULT_LENGTH = 1000;

KisDynamicSensorFade::KisDynamicSensorFade() : KisDynamicSensor(FadeId), m_counter(0), m_length(DEFAULT_LENGTH), m_periodic(false)
{
    setMinimumLabel(i18n("0"));
    setLength(DEFAULT_LENGTH);
}

qreal KisDynamicSensorFade::value(const KisPaintInformation&  pi)
{
    Q_UNUSED(pi);
    if (m_counter > m_length){
        if (m_periodic){
            reset();
        }else{
            m_counter = m_length;
        }
    }
    
    qreal result =  1.0 - (m_counter / qreal(m_length));
    m_counter++;
    
    return result;
}

void KisDynamicSensorFade::reset()
{
    m_counter = 0;
}

void KisDynamicSensorFade::setPeriodic(bool periodic)
{
    m_periodic = periodic;
}

void KisDynamicSensorFade::setLength(int length)
{
    m_length = length;
    setMaximumLabel(i18n("%1", length));
}

QWidget* KisDynamicSensorFade::createConfigurationWidget(QWidget* parent, QWidget* ss)
{
    QWidget* wdg = new QWidget(parent);
    Ui_SensorFadeConfiguration stc;
    stc.setupUi(wdg);
    stc.checkBoxRepeat->setChecked(m_periodic);
    connect(stc.checkBoxRepeat, SIGNAL(toggled(bool)), SLOT(setPeriodic(bool)));
    connect(stc.checkBoxRepeat, SIGNAL(toggled(bool)), ss, SIGNAL(parametersChanged()));
    stc.spinBoxLength->setValue(m_length);
    connect(stc.spinBoxLength, SIGNAL(valueChanged(int)), SLOT(setLength(int)));
    connect(stc.spinBoxLength, SIGNAL(valueChanged(int)), ss, SIGNAL(parametersChanged()));
    return wdg;
}

void KisDynamicSensorFade::toXML(QDomDocument& doc, QDomElement& e) const
{
    KisDynamicSensor::toXML(doc, e);
    e.setAttribute("periodic", m_periodic);
    e.setAttribute("length", m_length);
}

void KisDynamicSensorFade::fromXML(const QDomElement& e)
{
    KisDynamicSensor::fromXML(e);
    m_periodic = e.attribute("periodic", "0").toInt();
    m_length = e.attribute("length", QString::number(DEFAULT_LENGTH)).toInt();
}

#include "kis_dynamic_sensor_fade.moc"
