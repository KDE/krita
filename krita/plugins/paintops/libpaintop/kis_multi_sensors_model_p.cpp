/*
 *  Copyright (c) 2011 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2.1 of the License, or
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

#include "kis_multi_sensors_model_p.h"
#include "kis_dynamic_sensor.h"
#include "kis_curve_option.h"

KisMultiSensorsModel::KisMultiSensorsModel(QObject* parent)
    : QAbstractListModel(parent)
    , m_curveOption(0)
{
}

KisMultiSensorsModel::~KisMultiSensorsModel()
{
}

void KisMultiSensorsModel::setCurveOption(KisCurveOption *curveOption)
{
    beginResetModel();
    m_curveOption = curveOption;
    endResetModel();
}

int KisMultiSensorsModel::rowCount(const QModelIndex &/*parent*/) const
{
    return m_curveOption->sensors().size();
}

QVariant KisMultiSensorsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();

    if (role == Qt::DisplayRole) {
        return KisDynamicSensor::sensorsIds()[index.row()].name();
    }
    else if (role == Qt::CheckStateRole) {
        QString selectedSensorId = KisDynamicSensor::sensorsIds()[index.row()].id();
        KisDynamicSensorSP sensor = m_curveOption->sensor(selectedSensorId, false);
        if (sensor) {
            //qDebug() << sensor->id() << sensor->isActive();
            return QVariant(sensor->isActive() ? Qt::Checked : Qt::Unchecked);
        }
        else {
            return QVariant(Qt::Unchecked);
        }
    }
    return QVariant();
}

bool KisMultiSensorsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    bool result = false;

    if (role == Qt::CheckStateRole) {
        bool checked = (value.toInt() == Qt::Checked);

        if (checked || m_curveOption->activeSensors().size() != 1) { // Don't uncheck the last sensor (but why not?)
            KisDynamicSensorSP sensor = m_curveOption->sensor(KisDynamicSensor::sensorsIds()[index.row()].id(), false);

            if (!sensor) {
                sensor = KisDynamicSensor::id2Sensor(KisDynamicSensor::sensorsIds()[index.row()].id());
                m_curveOption->replaceSensor(sensor);
            }

            sensor->setActive(checked);
            emit(parametersChanged());
            result = true;
        }
    }
    return result;
}

Qt::ItemFlags KisMultiSensorsModel::flags(const QModelIndex & /*index */) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
}

KisDynamicSensorSP KisMultiSensorsModel::getSensor(const QModelIndex& index)
{
    if (!index.isValid()) return 0;
    QString id = KisDynamicSensor::sensorsIds()[index.row()].id();
    return m_curveOption->sensor(id, false);
}

void KisMultiSensorsModel::setCurrentCurve(const QModelIndex& currentIndex, const KisCubicCurve& curve, bool useSameCurve)
{
    if (!currentIndex.isValid()) return;

    QString selectedSensorId =  KisDynamicSensor::sensorsIds()[currentIndex.row()].id();
    m_curveOption->setCurve(selectedSensorId, useSameCurve, curve);
}

QModelIndex KisMultiSensorsModel::sensorIndex(KisDynamicSensorSP arg1)
{
    return index(KisDynamicSensor::sensorsIds().indexOf(KoID(arg1->id())));
}

void KisMultiSensorsModel::resetCurveOption()
{
    beginResetModel();
    reset();
    endResetModel();
}

#include "kis_multi_sensors_model_p.moc"
