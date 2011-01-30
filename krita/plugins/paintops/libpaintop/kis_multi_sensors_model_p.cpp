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

KisMultiSensorsModel::KisMultiSensorsModel(QObject* parent) : QAbstractListModel(parent)
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
        return QVariant(false);
    }
    return QVariant();
}

Qt::ItemFlags KisMultiSensorsModel::flags( const QModelIndex & /*index */) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled;
}
