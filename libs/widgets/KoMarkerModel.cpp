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


KoMarkerModel::KoMarkerModel(const QList<KoMarker*> markers, KoFlake::MarkerPosition position, QObject *parent)
    : QAbstractListModel(parent)
    , m_markerPosition(position)
    , m_temporaryMarkerPosition(-1)
{
    Q_FOREACH (KoMarker *marker, markers) {
        m_markers.append(QExplicitlySharedDataPointer<KoMarker>(marker));
    }
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
            return QVariant::fromValue<KoMarker*>(m_markers.at(index.row()).data());
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
    for (int i = 0; i < m_markers.size(); i++) {
        if (m_markers[i] == marker) return i;
        if (m_markers[i] && marker && *m_markers[i] == *marker) return i;
    }

    return false;
}

int KoMarkerModel::addTemporaryMarker(KoMarker *marker)
{
    if (m_temporaryMarkerPosition >= 0) {
        removeTemporaryMarker();
    }

    m_temporaryMarkerPosition = m_markers.size() > 0 ? 1 : 0;
    beginInsertRows(QModelIndex(), m_temporaryMarkerPosition, m_temporaryMarkerPosition);
    m_markers.prepend(QExplicitlySharedDataPointer<KoMarker>(marker));

    endInsertRows();

    return m_temporaryMarkerPosition;
}

void KoMarkerModel::removeTemporaryMarker()
{
    if (m_temporaryMarkerPosition >= 0) {
        beginRemoveRows(QModelIndex(), m_temporaryMarkerPosition, m_temporaryMarkerPosition);
        m_markers.removeAt(m_temporaryMarkerPosition);
        m_temporaryMarkerPosition = -1;
        endRemoveRows();
    }
}

int KoMarkerModel::temporaryMarkerPosition() const
{
    return m_temporaryMarkerPosition;
}

QVariant KoMarkerModel::marker(int index, int role) const
{
    if (index < 0){
        return QVariant();
    }

    switch(role) {
        case Qt::DecorationRole:
            if (index< m_markers.size()) {
                return QVariant::fromValue<KoMarker*>(m_markers.at(index).data());
            }
            return QVariant();
        case Qt::SizeHintRole:
            return QSize(80, 30);
        default:
            return QVariant();
    }
}

KoFlake::MarkerPosition KoMarkerModel::position() const
{
    return m_markerPosition;
}
