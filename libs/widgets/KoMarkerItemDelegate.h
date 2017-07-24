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
