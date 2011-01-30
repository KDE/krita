/*
 *  Copyright (c) 2011 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 or later of the License.
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

#include "kis_multi_sensors_model_p.h"
#include "kis_dynamic_sensor.h"
#include "sensors/kis_dynamic_sensor_list.h"

KisMultiSensorsModel::KisMultiSensorsModel(QObject* parent) : QAbstractListModel(parent), m_currentSensor(0), m_listSensor(0)
{
}

int KisMultiSensorsModel::rowCount(const QModelIndex &/*parent*/) const
{
    return KisDynamicSensor::sensorsIds().size();
}

QVariant KisMultiSensorsModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()) return QVariant();
    if(role == Qt::DisplayRole)
    {
        return KisDynamicSensor::sensorsIds()[index.row()].name();
    } else if(role == Qt::CheckStateRole)
    {
        if(m_listSensor)
        {
            return QVariant(m_listSensor->hasSensor(KisDynamicSensor::sensorsIds()[index.row()].id()) ? Qt::Checked : Qt::Unchecked );
        } else if(m_currentSensor)
        {
            return QVariant(m_currentSensor->id() == KisDynamicSensor::sensorsIds()[index.row()].id() ? Qt::Checked : Qt::Unchecked );
        } else {
            return QVariant(Qt::Checked);
        }
    }
    return QVariant();
}

bool KisMultiSensorsModel::setData(const QModelIndex &index, const QVariant &value, int role )
{
    if(role == Qt::CheckStateRole)
    {
        bool checked = value.toInt() == Qt::Checked;
        if(!checked && m_listSensor == 0) // It is not accepted to uncheck when there is only one sensor left
        {
          return false;
        } else if(m_listSensor) {
        } else {
            Q_ASSERT(checked);
            m_listSensor = new KisDynamicSensorList;
            m_listSensor->addSensor(m_currentSensor);
            m_currentSensor = m_listSensor;
            m_listSensor->addSensor(KisDynamicSensor::id2Sensor( KisDynamicSensor::sensorsIds()[index.row()] ) );
            return true;
        }
    }
    return false;
}

Qt::ItemFlags KisMultiSensorsModel::flags( const QModelIndex & /*index */) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
}

void KisMultiSensorsModel::setCurrentSensor(KisDynamicSensor* sensor)
{
    m_currentSensor = sensor;
    m_listSensor    = dynamic_cast<KisDynamicSensorList*>(sensor);
    reset();
}
