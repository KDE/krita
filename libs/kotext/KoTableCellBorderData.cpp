/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2009 KO GmbH <cbo@kogmbh.com>
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

#include "KoTableCellBorderData.h"

#include <kdebug.h>

struct Edge {
    Edge() : distance(0.0) { }
    QPen innerPen;
    QPen outerPen;
    qreal distance;
};

class TableCellBorderData::Private
{
public:
    Private() : refCount(0) {}
    Edge edges[4];

    int refCount;
};

TableCellBorderData::TableCellBorderData()
        : d(new Private())
{
    // set thise so we don't have to on each load()
    d->edges[Top].innerPen.setJoinStyle(Qt::MiterJoin);
    d->edges[Top].innerPen.setCapStyle(Qt::FlatCap);
    d->edges[Left].innerPen.setJoinStyle(Qt::MiterJoin);
    d->edges[Left].innerPen.setCapStyle(Qt::FlatCap);
    d->edges[Bottom].innerPen.setJoinStyle(Qt::MiterJoin);
    d->edges[Bottom].innerPen.setCapStyle(Qt::FlatCap);
    d->edges[Right].innerPen.setJoinStyle(Qt::MiterJoin);
    d->edges[Right].innerPen.setCapStyle(Qt::FlatCap);
    d->edges[Top].outerPen.setJoinStyle(Qt::MiterJoin);
    d->edges[Top].outerPen.setCapStyle(Qt::FlatCap);
    d->edges[Left].outerPen.setJoinStyle(Qt::MiterJoin);
    d->edges[Left].outerPen.setCapStyle(Qt::FlatCap);
    d->edges[Bottom].outerPen.setJoinStyle(Qt::MiterJoin);
    d->edges[Bottom].outerPen.setCapStyle(Qt::FlatCap);
    d->edges[Right].outerPen.setJoinStyle(Qt::MiterJoin);
    d->edges[Right].outerPen.setCapStyle(Qt::FlatCap);
}

TableCellBorderData::~TableCellBorderData()
{
    delete d;
}

TableCellBorderData::TableCellBorderData(const TableCellBorderData &other)
        : d(new Private())
{
    for (int i = Top; i <= Right; i++)
        d->edges[i] = other.d->edges[i];
}

bool TableCellBorderData::hasBorders() const
{
    for (int i = Top; i <= Right; i++)
        if (d->edges[i].outerPen.widthF() > 0.0)
            return true;
    return false;
}

bool TableCellBorderData::operator==(const TableCellBorderData &border)
{
    return equals(border);
}
bool TableCellBorderData::equals(const TableCellBorderData &border)
{
    for (int i = Top; i <= Right; i++) {
        if (d->edges[i].outerPen != border.d->edges[i].outerPen)
            return false;
        if (d->edges[i].innerPen != border.d->edges[i].innerPen)
            return false;
        if (qAbs(d->edges[i].distance - border.d->edges[i].distance) > 1E-10)
            return false;
    }
    return true;
}

void TableCellBorderData::paint(QPainter &painter, const QRectF &bounds) const
{
    QRectF innerBounds = bounds;
    if (d->edges[Top].outerPen.widthF() > 0) {
        QPen pen = d->edges[Top].outerPen;

        painter.setPen(pen);
        const qreal t = bounds.top() + pen.widthF() / 2.0;
        painter.drawLine(QLineF(bounds.left(), t, bounds.right(), t));
        innerBounds.setTop(bounds.top() + d->edges[Top].distance + pen.widthF());
    }
    if (d->edges[Bottom].outerPen.widthF() > 0) {
        QPen pen = d->edges[Bottom].outerPen;
        painter.setPen(pen);
        const qreal b = bounds.bottom() - pen.widthF() / 2.0;
        innerBounds.setBottom(bounds.bottom() - d->edges[Bottom].distance - pen.widthF());
        painter.drawLine(QLineF(bounds.left(), b, bounds.right(), b));
    }
    if (d->edges[Left].outerPen.widthF() > 0) {
        QPen pen = d->edges[Left].outerPen;
        painter.setPen(pen);
        const qreal l = bounds.left() + pen.widthF() / 2.0;
        innerBounds.setLeft(bounds.left() + d->edges[Left].distance + pen.widthF());
        painter.drawLine(QLineF(l, bounds.top(), l, bounds.bottom()));
    }
    if (d->edges[Right].outerPen.widthF() > 0) {
        QPen pen = d->edges[Right].outerPen;
        painter.setPen(pen);
        const qreal r = bounds.right() - pen.widthF() / 2.0;
        innerBounds.setRight(bounds.right() - d->edges[Right].distance - pen.widthF());
        painter.drawLine(QLineF(r, bounds.top(), r, bounds.bottom()));
    }
    // inner lines
    if (d->edges[Top].innerPen.widthF() > 0) {
        QPen pen = d->edges[Top].innerPen;
        painter.setPen(pen);
        const qreal t = innerBounds.top() + pen.widthF() / 2.0;
        painter.drawLine(QLineF(innerBounds.left(), t, innerBounds.right(), t));
    }
    if (d->edges[Bottom].innerPen.widthF() > 0) {
        QPen pen = d->edges[Bottom].innerPen;
        painter.setPen(pen);
        const qreal b = innerBounds.bottom() - pen.widthF() / 2.0;
        painter.drawLine(QLineF(innerBounds.left(), b, innerBounds.right(), b));
    }
    if (d->edges[Left].innerPen.widthF() > 0) {
        QPen pen = d->edges[Left].innerPen;
        painter.setPen(pen);
        const qreal l = innerBounds.left() + pen.widthF() / 2.0;
        painter.drawLine(QLineF(l, innerBounds.top(), l, innerBounds.bottom()));
    }
    if (d->edges[Right].innerPen.widthF() > 0) {
        QPen pen = d->edges[Right].innerPen;
        painter.setPen(pen);
        const qreal r = innerBounds.right() - pen.widthF() / 2.0;
        painter.drawLine(QLineF(r, innerBounds.top(), r, innerBounds.bottom()));
    }
}

void TableCellBorderData::save(QTextTableCellFormat &format)
{
    format.setProperty(TopBorderOuterPen, d->edges[Top].outerPen);
    format.setProperty(TopBorderSpacing,  d->edges[Top].distance);
    format.setProperty(TopBorderInnerPen, d->edges[Top].innerPen);
    format.setProperty(LeftBorderOuterPen, d->edges[Left].outerPen);
    format.setProperty(LeftBorderSpacing,  d->edges[Left].distance);
    format.setProperty(LeftBorderInnerPen, d->edges[Left].innerPen);
    format.setProperty(BottomBorderOuterPen, d->edges[Bottom].outerPen);
    format.setProperty(BottomBorderSpacing,  d->edges[Bottom].distance);
    format.setProperty(BottomBorderInnerPen, d->edges[Bottom].innerPen);
    format.setProperty(RightBorderOuterPen, d->edges[Right].outerPen);
    format.setProperty(RightBorderSpacing,  d->edges[Right].distance);
    format.setProperty(RightBorderInnerPen, d->edges[Right].innerPen);
}

void TableCellBorderData::load(const QTextTableCellFormat &format)
{
    d->edges[Top].outerPen = format.penProperty(TopBorderOuterPen);
    d->edges[Top].distance = format.doubleProperty(TopBorderSpacing);
    d->edges[Top].innerPen = format.penProperty(TopBorderInnerPen);

    d->edges[Left].outerPen = format.penProperty(LeftBorderOuterPen);
    d->edges[Left].distance = format.doubleProperty(LeftBorderSpacing);
    d->edges[Left].innerPen = format.penProperty(LeftBorderInnerPen);

    d->edges[Bottom].outerPen =format.penProperty(BottomBorderOuterPen);
    d->edges[Bottom].distance = format.doubleProperty(BottomBorderSpacing);
    d->edges[Bottom].innerPen = format.penProperty(BottomBorderInnerPen);

    d->edges[Right].outerPen = format.penProperty(RightBorderOuterPen);
    d->edges[Right].distance = format.doubleProperty(RightBorderSpacing);
    d->edges[Right].innerPen = format.penProperty(RightBorderInnerPen);
}

QRectF TableCellBorderData::contentRect(const QRectF &boundingRect)
{
    return boundingRect.adjusted(
                d->edges[Left].outerPen.widthF() + d->edges[Left].distance + d->edges[Left].innerPen.widthF(),
                d->edges[Top].outerPen.widthF() + d->edges[Top].distance + d->edges[Top].innerPen.widthF(),
                - d->edges[Right].outerPen.widthF() - d->edges[Right].distance - d->edges[Right].innerPen.widthF(),
                - d->edges[Bottom].outerPen.widthF() - d->edges[Bottom].distance - d->edges[Bottom].innerPen.widthF()
   ) ;
}

void TableCellBorderData::setEdge(Side side, Style style, qreal width, KoColor color, qreal innerWidth)
{
    Edge edge;
    switch (style) {
//    case Dotted: edge.innerPen.setStyle(Qt::DotLine); break;
    default:
        edge.innerPen.setStyle(Qt::SolidLine);
    }
    edge.innerPen.setColor(color.toQColor());
    edge.innerPen.setJoinStyle(Qt::MiterJoin);
    edge.innerPen.setCapStyle(Qt::FlatCap);
    edge.outerPen = edge.innerPen;
    edge.outerPen.setWidthF(width);

    edge.distance = width; // set the distance between to be the same as the outer width
    edge.innerPen.setWidthF(innerWidth);

    d->edges[side] = edge;
}

void TableCellBorderData::addUser()
{
    d->refCount++;
}

int TableCellBorderData::removeUser()
{
    return --d->refCount;
}

int TableCellBorderData::useCount() const
{
    return d->refCount;
}
