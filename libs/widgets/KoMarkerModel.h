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

#ifndef KOMARKERMODEL_H
#define KOMARKERMODEL_H

#include <KoMarkerData.h>
#include <QAbstractListModel>

class KoMarker;

class KoMarkerModel : public QAbstractListModel
{
public:
    KoMarkerModel(const QList<KoMarker*> markers, KoMarkerData::MarkerPosition position, QObject *parent = 0);
    virtual ~KoMarkerModel();

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    int markerIndex(KoMarker *marker) const;
    QVariant marker(int index, int role = Qt::UserRole) const;
    KoMarkerData::MarkerPosition position() const;

private:
    QList<KoMarker*> m_markers;
    KoMarkerData::MarkerPosition m_markerPosition;
};

#endif /* KOMARKERMODEL_H */
