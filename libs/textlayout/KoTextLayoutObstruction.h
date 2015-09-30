/* This file is part of the KDE project
 * Copyright (C) 2006-2007, 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 Ko Gmbh <cbo@kogmbh.com>
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

#ifndef KOTEXTLAYOUTOBSTRUCTION_H
#define KOTEXTLAYOUTOBSTRUCTION_H

#include "kritatextlayout_export.h"

#include <QMultiMap>
#include <QPolygonF>
#include <QRectF>
#include <QLineF>

class KoShape;
class QTransform;
class QPainterPath;

///  Class that allows us with the runaround of QPainterPaths
class KRITATEXTLAYOUT_EXPORT KoTextLayoutObstruction
{
public:
    KoTextLayoutObstruction(KoShape *shape, const QTransform &matrix);

    KoTextLayoutObstruction(const QRectF &rect, bool rtl);

    void init(const QTransform &matrix, const QPainterPath &obstruction, qreal distanceLeft, qreal distanceTop, qreal distanceRight, qreal distanceBottom, qreal borderHalfWidth);

    QRectF limit(const QRectF &content);

    KoShape *shape() const { return m_shape; }

    static qreal xAtY(const QLineF &line, qreal y);

    void changeMatrix(const QTransform &matrix);

    //-------------------------------------------------------------------------------

    QRectF cropToLine(const QRectF &lineRect);

    QRectF getLeftLinePart(const QRectF &lineRect) const;

    QRectF getRightLinePart(const QRectF &lineRect) const;

    bool textOnLeft() const;

    bool textOnRight() const;

    bool textOnBiggerSide() const;

    bool textOnEnoughSides() const;

    bool noTextAround() const;

    // Don't run around unless available space is > than this when m_side == Enough.
    qreal runAroundThreshold() const;

    static bool compareRectLeft(KoTextLayoutObstruction *o1, KoTextLayoutObstruction *o2);
private:
    QPainterPath decoratedOutline(const KoShape *shape, qreal &borderHalfWidth) const;

    enum Side { None, Left, Right, Empty, Both, Bigger, Enough };
    Side m_side;
    QRectF m_bounds;
    QPolygonF m_polygon;
    public:
    QRectF m_line;
    QMultiMap<qreal, QLineF> m_edges; //sorted with y-coord
    KoShape *m_shape;
    QRectF m_rect;
    qreal m_distanceLeft;
    qreal m_distanceTop;
    qreal m_distanceRight;
    qreal m_distanceBottom;
    qreal m_borderHalfWidth;
    qreal m_runAroundThreshold;
};

#endif
