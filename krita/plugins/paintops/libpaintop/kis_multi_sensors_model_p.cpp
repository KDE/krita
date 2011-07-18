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

KisMultiSensorsModel::~KisMultiSensorsModel()
{
    qDeleteAll(m_sensorCache);
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
        emit(parametersChanged());
        bool checked = value.toInt() == Qt::Checked;
        if(!checked && m_listSensor == 0) // It is not accepted to uncheck when there is only one sensor left
        {
          return false;
        } else if(m_listSensor) {
            if(checked)
            {
                m_listSensor->addSensor(takeOrCreateSensorFromCache( KisDynamicSensor::sensorsIds()[index.row()].id() ) );
                return true;
            } else {
                KisDynamicSensor* sensor = m_listSensor->takeSensor(KisDynamicSensor::sensorsIds()[index.row()].id());
                pushSensorToCache(sensor);;
                
                // If there is only one sensor left, remove it from the list sensor, and delete the list sensor
                QList<QString> ids = m_listSensor->sensorIds();
                Q_ASSERT(!ids.empty());
                if(ids.size() == 1)
                {
                    m_currentSensor = m_listSensor->takeSensor(ids.first());
                    m_listSensor = 0; // Don't delete the list sensor, it will be deleted as an effect of the call to sensorChanged
                    emit(sensorChanged(m_currentSensor));
                }
                return true;
            }
        } else {
            Q_ASSERT(checked);
            m_listSensor = new KisDynamicSensorList;
            m_listSensor->addSensor(m_currentSensor->clone()); // Use a clone to make sure that it is owned by the KisDynamicSensorList
            m_currentSensor = m_listSensor;
            m_listSensor->addSensor(takeOrCreateSensorFromCache( KisDynamicSensor::sensorsIds()[index.row()].id() ) );
            emit(sensorChanged(m_currentSensor));
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

KisDynamicSensor* KisMultiSensorsModel::getSensor(const QModelIndex& index)
{
    if(!index.isValid()) return 0;
    QString id = KisDynamicSensor::sensorsIds()[index.row()].id();
    if(m_currentSensor && m_currentSensor->id() == id)
    {
        return m_currentSensor;
    } else if (m_listSensor) {
        KisDynamicSensor* sensor = m_listSensor->getSensor(id);
        if(sensor)
        {
            return sensor;
        }
    }
    return getOrCreateSensorFromCache(id);
}

KisDynamicSensor* KisMultiSensorsModel::getOrCreateSensorFromCache(const QString& id)
{
    if(m_sensorCache.contains(id)) return m_sensorCache.value(id);
    KisDynamicSensor* sensor = KisDynamicSensor::id2Sensor(id);
    if(!m_listSensor || !m_listSensor->hasCustomCurve())
    {
        sensor->setCurve(m_currentSensor->curve());
    }
    m_sensorCache[id] = sensor;
    return sensor;
}

KisDynamicSensor* KisMultiSensorsModel::takeOrCreateSensorFromCache(const QString& id)
{
    if(m_sensorCache.contains(id)) return m_sensorCache.take(id);
    KisDynamicSensor* sensor = KisDynamicSensor::id2Sensor(id);
    if(!m_listSensor || !m_listSensor->hasCustomCurve())
    {
        sensor->setCurve(m_currentSensor->curve());
    }
    return sensor;
}

void KisMultiSensorsModel::pushSensorToCache(KisDynamicSensor* sensor)
{
    Q_ASSERT(!m_sensorCache.contains(sensor->id()));
    m_sensorCache[sensor->id()] = sensor;
}

void KisMultiSensorsModel::setCurrentCurve(const QModelIndex& currentIndex, const KisCubicCurve& curve, bool useSameCurve)
{
    if(useSameCurve)
    {
        foreach(KisDynamicSensor* sensor, m_sensorCache)
        {
            sensor->setCurve(curve);
            sensor->removeCurve();
        }
        m_currentSensor->setCurve(curve);
        if(m_listSensor)
        {
            foreach(const QString& id, m_listSensor->sensorIds())
            {
                KisDynamicSensor* sensor =  m_listSensor->getSensor(id);
                if (sensor) {
                    sensor->setCurve(curve);
                    sensor->removeCurve();
                }
            }
        }
    } else {
        KisDynamicSensor* sensor = getSensor(currentIndex);
        if (sensor) {
            sensor->setCurve(curve);
            if (m_listSensor)
            {
                m_listSensor->removeCurve();
            }
        }
    }
}

QModelIndex KisMultiSensorsModel::sensorIndex(KisDynamicSensor* arg1)
{
    return index(KisDynamicSensor::sensorsIds().indexOf(KoID(arg1->id())));
}

#include "kis_multi_sensors_model_p.moc"
