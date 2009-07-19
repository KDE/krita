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
    Edge() : spacing(0.0) { }
    QPen innerPen;
    QPen outerPen;
    qreal spacing;
    qreal padding;
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
        if (qAbs(d->edges[i].spacing - border.d->edges[i].spacing) > 1E-10)
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
        const qreal hpw = pen.widthF() / 2.0; // half pen width
        const qreal t = bounds.top() + hpw;
        innerBounds.setTop(bounds.top() + d->edges[Top].spacing + pen.widthF());
        painter.drawLine(QLineF(bounds.left() + hpw, t, bounds.right() - hpw, t));
    }
    if (d->edges[Bottom].outerPen.widthF() > 0) {
        QPen pen = d->edges[Bottom].outerPen;
        painter.setPen(pen);
        const qreal hpw = pen.widthF() / 2.0; // half pen width
        const qreal b = bounds.bottom() - hpw;
        innerBounds.setBottom(bounds.bottom() - d->edges[Bottom].spacing - pen.widthF());
        painter.drawLine(QLineF(bounds.left() + hpw, b, bounds.right() - hpw, b));
    }
    if (d->edges[Left].outerPen.widthF() > 0) {
        QPen pen = d->edges[Left].outerPen;
        painter.setPen(pen);
        const qreal hpw = pen.widthF() / 2.0; // half pen width
        const qreal l = bounds.left() + hpw;
        innerBounds.setLeft(bounds.left() + d->edges[Left].spacing + pen.widthF());
        painter.drawLine(QLineF(l, bounds.top() + hpw, l, bounds.bottom() - hpw));
    }
    if (d->edges[Right].outerPen.widthF() > 0) {
        QPen pen = d->edges[Right].outerPen;
        painter.setPen(pen);
        const qreal hpw = pen.widthF() / 2.0; // half pen width
        const qreal r = bounds.right() - hpw;
        innerBounds.setRight(bounds.right() - d->edges[Right].spacing - pen.widthF());
        painter.drawLine(QLineF(r, bounds.top() + hpw, r, bounds.bottom() - hpw));
    }
    // inner lines
    if (d->edges[Top].innerPen.widthF() > 0) {
        QPen pen = d->edges[Top].innerPen;
        painter.setPen(pen);
        const qreal hpw = pen.widthF() / 2.0; // half pen width
        const qreal t = innerBounds.top() + hpw;
        painter.drawLine(QLineF(innerBounds.left() + hpw, t, innerBounds.right() - hpw, t));
    }
    if (d->edges[Bottom].innerPen.widthF() > 0) {
        QPen pen = d->edges[Bottom].innerPen;
        painter.setPen(pen);
        const qreal hpw = pen.widthF() / 2.0; // half pen width
        const qreal b = innerBounds.bottom() - hpw;
        painter.drawLine(QLineF(innerBounds.left() + hpw, b, innerBounds.right() - hpw, b));
    }
    if (d->edges[Left].innerPen.widthF() > 0) {
        QPen pen = d->edges[Left].innerPen;
        painter.setPen(pen);
        const qreal hpw = pen.widthF() / 2.0; // half pen width
        const qreal l = innerBounds.left() + hpw;
        painter.drawLine(QLineF(l, innerBounds.top() + hpw, l, innerBounds.bottom() - hpw));
    }
    if (d->edges[Right].innerPen.widthF() > 0) {
        QPen pen = d->edges[Right].innerPen;
        painter.setPen(pen);
        const qreal hpw = pen.widthF() / 2.0; // half pen width
        const qreal r = innerBounds.right() - hpw;
        painter.drawLine(QLineF(r, innerBounds.top() + hpw, r, innerBounds.bottom() - hpw));
    }
}

void TableCellBorderData::applyStyle(QTextTableCellFormat &format)
{
    format.setProperty(TopBorderOuterPen, d->edges[Top].outerPen);
    format.setProperty(TopBorderSpacing,  d->edges[Top].spacing);
    format.setProperty(TopBorderInnerPen, d->edges[Top].innerPen);
    format.setProperty(LeftBorderOuterPen, d->edges[Left].outerPen);
    format.setProperty(LeftBorderSpacing,  d->edges[Left].spacing);
    format.setProperty(LeftBorderInnerPen, d->edges[Left].innerPen);
    format.setProperty(BottomBorderOuterPen, d->edges[Bottom].outerPen);
    format.setProperty(BottomBorderSpacing,  d->edges[Bottom].spacing);
    format.setProperty(BottomBorderInnerPen, d->edges[Bottom].innerPen);
    format.setProperty(RightBorderOuterPen, d->edges[Right].outerPen);
    format.setProperty(RightBorderSpacing,  d->edges[Right].spacing);
    format.setProperty(RightBorderInnerPen, d->edges[Right].innerPen);

    format.setTopPadding(d->edges[Top].padding);
    format.setLeftPadding(d->edges[Left].padding);
    format.setRightPadding(d->edges[Right].padding);
    format.setBottomPadding(d->edges[Bottom].padding);
}

void TableCellBorderData::load(const QTextTableCellFormat &format)
{
    d->edges[Top].outerPen = format.penProperty(TopBorderOuterPen);
    d->edges[Top].spacing = format.doubleProperty(TopBorderSpacing);
    d->edges[Top].innerPen = format.penProperty(TopBorderInnerPen);

    d->edges[Left].outerPen = format.penProperty(LeftBorderOuterPen);
    d->edges[Left].spacing = format.doubleProperty(LeftBorderSpacing);
    d->edges[Left].innerPen = format.penProperty(LeftBorderInnerPen);

    d->edges[Bottom].outerPen =format.penProperty(BottomBorderOuterPen);
    d->edges[Bottom].spacing = format.doubleProperty(BottomBorderSpacing);
    d->edges[Bottom].innerPen = format.penProperty(BottomBorderInnerPen);

    d->edges[Right].outerPen = format.penProperty(RightBorderOuterPen);
    d->edges[Right].spacing = format.doubleProperty(RightBorderSpacing);
    d->edges[Right].innerPen = format.penProperty(RightBorderInnerPen);

    d->edges[Top].padding = format.topPadding();
    d->edges[Left].padding = format.leftPadding();
    d->edges[Right].padding = format.rightPadding();
    d->edges[Bottom].padding = format.bottomPadding();
}

QRectF TableCellBorderData::contentRect(const QRectF &boundingRect) const
{
    return boundingRect.adjusted(
                d->edges[Left].outerPen.widthF() + d->edges[Left].spacing + d->edges[Left].innerPen.widthF() + d->edges[Left].padding,
                d->edges[Top].outerPen.widthF() + d->edges[Top].spacing + d->edges[Top].innerPen.widthF() + d->edges[Top].padding,
                - d->edges[Right].outerPen.widthF() - d->edges[Right].spacing - d->edges[Right].innerPen.widthF() - d->edges[Right].padding,
                - d->edges[Bottom].outerPen.widthF() - d->edges[Bottom].spacing - d->edges[Bottom].innerPen.widthF() - d->edges[Bottom].padding
   );
}

QRectF TableCellBorderData::boundingRect(const QRectF &contentRect) const
{
    return contentRect.adjusted(
                - d->edges[Left].outerPen.widthF() - d->edges[Left].spacing - d->edges[Left].innerPen.widthF() - d->edges[Left].padding,
                - d->edges[Top].outerPen.widthF() - d->edges[Top].spacing - d->edges[Top].innerPen.widthF() - d->edges[Top].padding,
                d->edges[Right].outerPen.widthF() + d->edges[Right].spacing + d->edges[Right].innerPen.widthF() + d->edges[Right].padding,
                d->edges[Bottom].outerPen.widthF() + d->edges[Bottom].spacing + d->edges[Bottom].innerPen.widthF() + d->edges[Bottom].padding
   );
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

    edge.spacing = width; // set the spacing between to be the same as the outer width
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
