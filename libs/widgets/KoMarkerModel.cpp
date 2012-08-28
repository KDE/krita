/* This file is part of the KDE project
 * Copyright (C) 2011 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoMarkerModel.h"

// Calligra
#include <KoMarker.h>
// Qt
#include <QSize>


KoMarkerModel::KoMarkerModel(const QList<KoMarker*> markers, KoMarkerData::MarkerPosition position, QObject *parent)
: QAbstractListModel(parent)
, m_markers(markers)
, m_markerPosition(position)
{
}

KoMarkerModel::~KoMarkerModel()
{
}

int KoMarkerModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_markers.count();
}

QVariant KoMarkerModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
         return QVariant();
    }

    switch(role) {
    case Qt::DecorationRole:
        if (index.row() < m_markers.size()) {
            return QVariant::fromValue<KoMarker*>(m_markers.at(index.row()));
        }
        return QVariant();
    case Qt::SizeHintRole:
        return QSize(80,30);
    default:
        return QVariant();
    }
}

int KoMarkerModel::markerIndex(KoMarker *marker) const
{
    return m_markers.indexOf(marker);
}

QVariant KoMarkerModel::marker(int index, int role) const
{
    if (index < 0){
        return QVariant();
    }

    switch(role) {
        case Qt::DecorationRole:
            if (index< m_markers.size()) {
                return QVariant::fromValue<KoMarker*>(m_markers.at(index));
            }
            return QVariant();
        case Qt::SizeHintRole:
            return QSize(80, 30);
        default:
            return QVariant();
    }
}

KoMarkerData::MarkerPosition KoMarkerModel::position() const
{
    return m_markerPosition;
}
