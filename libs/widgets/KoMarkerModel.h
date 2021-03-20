/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2011 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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
