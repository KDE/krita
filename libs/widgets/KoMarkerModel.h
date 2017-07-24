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

#include <KoFlake.h>
#include <QAbstractListModel>
#include <QExplicitlySharedDataPointer>


class KoMarker;

class KoMarkerModel : public QAbstractListModel
{
public:
    KoMarkerModel(const QList<KoMarker*> markers, KoFlake::MarkerPosition position, QObject *parent = 0);
    ~KoMarkerModel() override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    int markerIndex(KoMarker *marker) const;

    // returns index of the newly added temporary marker
    int addTemporaryMarker(KoMarker *marker);
    // removes a temporary marker added by \ref addTemporaryMarker
    void removeTemporaryMarker();

    int temporaryMarkerPosition() const;

    QVariant marker(int index, int role = Qt::UserRole) const;
    KoFlake::MarkerPosition position() const;

private:
    QList<QExplicitlySharedDataPointer<KoMarker>> m_markers;
    KoFlake::MarkerPosition m_markerPosition;
    int m_temporaryMarkerPosition;
};

#endif /* KOMARKERMODEL_H */
