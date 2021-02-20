/*
 *  SPDX-FileCopyrightText: 2011 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
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
    if (m_curveOption) {
        return m_curveOption->sensors().size();
    }
    else {
        return 0;
    }
}

QVariant KisMultiSensorsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();

    if (role == Qt::DisplayRole) {
        return m_curveOption->sensorsIds()[index.row()].name();
    }
    else if (role == Qt::CheckStateRole) {
        QString selectedSensorId = m_curveOption->sensorsIds()[index.row()].id();
        KisDynamicSensorSP sensor = m_curveOption->sensor(m_curveOption->id2Type(KoID(selectedSensorId)), false);
        if (sensor) {
            //dbgKrita << sensor->id() << sensor->isActive();
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
            KisDynamicSensorSP sensor = m_curveOption->sensor(m_curveOption->id2Type(m_curveOption->sensorsIds()[index.row()]), false);

            if (!sensor) {
                sensor = m_curveOption->id2Sensor(m_curveOption->sensorsIds()[index.row()], "NOT_VALID_NAME");
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
    QString id = m_curveOption->sensorsIds()[index.row()].id();
    return m_curveOption->sensor(m_curveOption->id2Type(KoID(id)), false);
}

void KisMultiSensorsModel::setCurrentCurve(const QModelIndex& currentIndex, const KisCubicCurve& curve, bool useSameCurve)
{
    if (!currentIndex.isValid()) return;

    QString selectedSensorId =  m_curveOption->sensorsIds()[currentIndex.row()].id();
    m_curveOption->setCurve(m_curveOption->id2Type(KoID(selectedSensorId)), useSameCurve, curve);
}

QModelIndex KisMultiSensorsModel::sensorIndex(KisDynamicSensorSP arg1)
{
    return index(m_curveOption->sensorsIds().indexOf(KoID(arg1->identifier())));
}

void KisMultiSensorsModel::resetCurveOption()
{
    beginResetModel();
    endResetModel();
}

