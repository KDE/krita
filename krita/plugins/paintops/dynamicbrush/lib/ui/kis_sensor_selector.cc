/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
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

#include <kdebug.h>

#include "kis_sensor_selector.h"

#include <QHBoxLayout>

#include "kis_dynamic_sensor.h"

#include "ui_SensorSelector.h"

KisSensorSelector::KisSensorSelector(QWidget* parent) : QWidget(parent), m_currentConfigWidget(0), m_currentSensor(0)
{
    sensorSelectorUI = new Ui_SensorSelector;
    sensorSelectorUI->setupUi(this);
    sensorSelectorUI->comboBoxSensor->setIDList( KisDynamicSensor::sensorsIds() );
    m_layout = new QHBoxLayout(sensorSelectorUI->widgetConfiguration);
    connect(sensorSelectorUI->comboBoxSensor, SIGNAL(activated(const KoID &)), this, SLOT(setSensorId(const KoID& )));
}

void KisSensorSelector::setCurrent(KisDynamicSensor* sensor)
{
    kDebug(41006) <<"setCurrent" << sensor <<"" << sensor->id();
    m_currentSensor = sensor;
    sensorSelectorUI->comboBoxSensor->setCurrent( sensor->id() );
    delete m_currentConfigWidget;
    m_currentConfigWidget = sensor->createConfigurationWidget( sensorSelectorUI->widgetConfiguration );
    if(m_currentConfigWidget)
    {
        m_layout->addWidget( m_currentConfigWidget );
    }
}

void KisSensorSelector::setSensorId(const KoID& id)
{
    if(not m_currentSensor or id.id() != m_currentSensor->id() )
    {
        setCurrent( KisDynamicSensor::id2Sensor(id) );
        emit sensorChanged(m_currentSensor);
    }
}

#include "kis_sensor_selector.moc"
