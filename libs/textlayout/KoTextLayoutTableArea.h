/* This file is part of the KDE project
 * Copyright (C) 2011 Casper Boemann <cbo@kogmbh.com>
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

#ifndef KOTEXTLAYOUTTABLEAREA_H
#define KOTEXTLAYOUTTABLEAREA_H

#include "kotext_export.h"

#include "KoTextLayoutArea.h"

#include <QVector>
#include <QLineF>

class TableIterator;

/**
 * This class represent a (full width) piece of a table
 */
class KOTEXT_EXPORT KoTextLayoutTableArea : public KoTextLayoutArea
{
public:
    /// constructor
    explicit KoTextLayoutTableArea(QTextTable *table, KoTextLayoutArea *parent, KoTextDocumentLayout *documentLayout);
    virtual ~KoTextLayoutTableArea();

    /// Layouts as much as it can
    /// Returns true if it has reached the end of the table
    bool layout(TableIterator *cursor);

    void paint(QPainter *painter, const KoTextDocumentLayout::PaintContext &context);

    int hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const;

    /// Calc a bounding box rect of the selection
    QRectF selectionBoundingBox(QTextCursor &cursor) const;

    /// Returns true if the area contains the position
    bool containsPosition(int position) const;

private:
    void layoutColumns();
    bool layoutRow(TableIterator *cursor);
    QRectF cellBoundingRect(const QTextTableCell &cell) const;
    void paintCell(QPainter *painter, const KoTextDocumentLayout::PaintContext &context, QTextTableCell tableCell);
    void paintCellBorders(QPainter *painter, const KoTextDocumentLayout::PaintContext &context, bool collapsing, QTextTableCell tableCell, QVector<QLineF> *accuBlankBorders);

    class Private;
    Private * const d;
};

#endif
