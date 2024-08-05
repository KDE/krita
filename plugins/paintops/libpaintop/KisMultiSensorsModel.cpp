/*
 *  SPDX-FileCopyrightText: 2011 Cyrille Berger <cberger@cberger.net>
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "KisMultiSensorsModel.h"
#include <KoID.h>

struct KisMultiSensorsModel::Private
{
    Private(lager::cursor<MultiSensorData> _sensorsData)
        : sensorsData(_sensorsData)
    {}

    lager::cursor<MultiSensorData> sensorsData;
};

KisMultiSensorsModel::KisMultiSensorsModel(lager::cursor<MultiSensorData> sensorsData,
                                             QObject* parent)
    : QAbstractListModel(parent),
      m_d(new Private(sensorsData))
{
    m_d->sensorsData.bind(std::bind(&KisMultiSensorsModel::slotSensorModelChanged, this));
}

KisMultiSensorsModel::~KisMultiSensorsModel()
{
}

int KisMultiSensorsModel::rowCount(const QModelIndex &/*parent*/) const
{
    return m_d->sensorsData->size();
}

QVariant KisMultiSensorsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();

    if (role == Qt::DisplayRole) {
        return m_d->sensorsData->at(index.row()).first.name();
    }
    else if (role == Qt::CheckStateRole) {
        return m_d->sensorsData->at(index.row()).second ? Qt::Checked : Qt::Unchecked;
    }
    return QVariant();
}

bool KisMultiSensorsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    bool result = false;

    if (role == Qt::CheckStateRole) {
        const bool checked = (value.toInt() == Qt::Checked);

        m_d->sensorsData.update([index, checked] (MultiSensorData sensors) {
            const int numActiveSensors =
                    std::count_if(sensors.begin(), sensors.end(),
                                  std::mem_fn(&SensorData::second));

            // we don't allow unchecking of the last sensor (but why?)
            if (checked || numActiveSensors > 1 ) {
                sensors[index.row()].second = checked;
            }
            return sensors;
        });
        result = true;
    }

    return result;
}

Qt::ItemFlags KisMultiSensorsModel::flags(const QModelIndex & /*index */) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
}

QString KisMultiSensorsModel::getSensorId(const QModelIndex& index)
{
    if (!index.isValid()) return 0;
    return m_d->sensorsData->at(index.row()).first.id();
}

QModelIndex KisMultiSensorsModel::sensorIndex(const QString &id)
{
    const size_t foundIndex =
            std::distance(m_d->sensorsData->begin(),
                 std::find_if(m_d->sensorsData->begin(), m_d->sensorsData->end(),
                     [id] (const SensorData &sensor) {
                         return sensor.first.id() == id;
                     }));

    return foundIndex < m_d->sensorsData->size() ?
                index(foundIndex) : QModelIndex();
}

void KisMultiSensorsModel::slotSensorModelChanged()
{
    Q_EMIT dataChanged(index(0), index(rowCount() - 1));
}
