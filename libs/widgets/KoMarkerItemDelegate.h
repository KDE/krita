/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2011 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOMARKERITEMDELEGATE_H
#define KOMARKERITEMDELEGATE_H

// Calligra
#include <KoFlake.h>
// Qt
#include <QAbstractItemDelegate>

class KoMarker;

class KoMarkerItemDelegate : public QAbstractItemDelegate
{
public:
    explicit KoMarkerItemDelegate(KoFlake::MarkerPosition position, QObject *parent = 0);
    ~KoMarkerItemDelegate() override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex &index) const override;

    static void drawMarkerPreview(QPainter *painter, const QRect &rect, const QPen &pen, KoMarker *marker, KoFlake::MarkerPosition position);
private:
    KoFlake::MarkerPosition m_position;
};

#endif /* KOMARKERITEMDELEGATE_H */
