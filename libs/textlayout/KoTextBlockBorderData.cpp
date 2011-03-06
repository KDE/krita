/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include "KoTextBlockBorderData.h"

#include <kdebug.h>

struct Edge {
    Edge() : distance(0.0) { }
    QPen innerPen;
    QPen outerPen;
    qreal distance;
};

class KoTextBlockBorderData::Private
{
public:
    Private() : refCount(0) {}
    Edge edges[4];

    QRectF bounds;
    QAtomicInt refCount;
};

KoTextBlockBorderData::KoTextBlockBorderData(const QRectF &paragRect)
        : d(new Private())
{
    d->bounds = paragRect;
}

KoTextBlockBorderData::~KoTextBlockBorderData()
{
    delete d;
}

KoTextBlockBorderData::KoTextBlockBorderData(const KoTextBlockBorderData &other)
        : d(new Private())
{
    for (int i = Top; i <= Right; i++)
        d->edges[i] = other.d->edges[i];
    d->bounds = other.d->bounds;
}

bool KoTextBlockBorderData::hasBorders() const
{
    for (int i = Top; i <= Right; i++)
        if (d->edges[i].outerPen.widthF() > 0.0)
            return true;
    return false;
}

bool KoTextBlockBorderData::operator==(const KoTextBlockBorderData &border)
{
    return equals(border);
}
bool KoTextBlockBorderData::equals(const KoTextBlockBorderData &border)
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

void KoTextBlockBorderData::applyInsets(KoInsets &insets, qreal paragStart, bool startUnderBorder) const
{
    insets.left += inset(Left);
    insets.right += inset(Right);

    // only apply top when the parag is the top parag in the border-set
    qreal insetTop = startUnderBorder ? inset(Top) : 0;
    if (qAbs(d->bounds.top() + insetTop - paragStart) < 1E-10)
        insets.top += startUnderBorder ? insetTop : inset(Top);
}

void KoTextBlockBorderData::setParagraphBottom(qreal bottom)
{
    d->bounds.setBottom(bottom + inset(Bottom));
}

void KoTextBlockBorderData::paint(QPainter &painter) const
{
    QRectF bounds = d->bounds;
    QRectF innerBounds = d->bounds;
    if (d->edges[Top].outerPen.widthF() > 0) {
        QPen pen = d->edges[Top].outerPen;

        painter.setPen(pen);
        const qreal t = d->bounds.top() + pen.widthF() / 2.0;
        painter.drawLine(QLineF(d->bounds.left(), t, d->bounds.right(), t));
        innerBounds.setTop(d->bounds.top() + d->edges[Top].distance + pen.widthF());
    }
    if (d->edges[Bottom].outerPen.widthF() > 0) {
        QPen pen = d->edges[Bottom].outerPen;
        painter.setPen(pen);
        const qreal b = d->bounds.bottom() - pen.widthF() / 2.0;
        innerBounds.setBottom(d->bounds.bottom() - d->edges[Bottom].distance - pen.widthF());
        painter.drawLine(QLineF(d->bounds.left(), b, d->bounds.right(), b));
    }
    if (d->edges[Left].outerPen.widthF() > 0) {
        QPen pen = d->edges[Left].outerPen;
        painter.setPen(pen);
        const qreal l = d->bounds.left() + pen.widthF() / 2.0;
        innerBounds.setLeft(d->bounds.left() + d->edges[Left].distance + pen.widthF());
        painter.drawLine(QLineF(l, d->bounds.top(), l, d->bounds.bottom()));
    }
    if (d->edges[Right].outerPen.widthF() > 0) {
        QPen pen = d->edges[Right].outerPen;
        painter.setPen(pen);
        const qreal r = d->bounds.right() - pen.widthF() / 2.0;
        innerBounds.setRight(d->bounds.right() - d->edges[Right].distance - pen.widthF());
        painter.drawLine(QLineF(r, d->bounds.top(), r, d->bounds.bottom()));
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

qreal KoTextBlockBorderData::inset(Side side) const
{
    return d->edges[side].outerPen.widthF() + d->edges[side].distance + d->edges[side].innerPen.widthF();
}

void KoTextBlockBorderData::setEdge(Side side, const QTextBlockFormat &bf,
                                    KoParagraphStyle::Property style, KoParagraphStyle::Property width,
                                    KoParagraphStyle::Property color, KoParagraphStyle::Property space,
                                    KoParagraphStyle::Property innerWidth)
{

    Edge edge;
    KoBorder::BorderStyle  borderStyle;
    borderStyle = static_cast<KoBorder::BorderStyle>(bf.intProperty(style));
    switch (borderStyle) {
    case KoBorder::BorderDotted: edge.innerPen.setStyle(Qt::DotLine); break;
    case KoBorder::BorderDashed: edge.innerPen.setStyle(Qt::DashLine); break;
    case KoBorder::BorderDashDotPattern: edge.innerPen.setStyle(Qt::DashDotLine); break;
    case KoBorder::BorderDashDotDotPattern: edge.innerPen.setStyle(Qt::DashDotDotLine); break;
    case KoBorder::BorderGroove: /* TODO */ break;
    case KoBorder::BorderRidge: /* TODO */ break;
    case KoBorder::BorderInset: /* TODO */ break;
    case KoBorder::BorderOutset: /* TODO */ break;
    default:
        edge.innerPen.setStyle(Qt::SolidLine);
    }
    edge.innerPen.setColor(bf.colorProperty(color));
    edge.innerPen.setJoinStyle(Qt::MiterJoin);
    edge.innerPen.setCapStyle(Qt::FlatCap);
    edge.outerPen = edge.innerPen;
    edge.outerPen.setWidthF(bf.doubleProperty(width));   // TODO check if this does not need any conversion

    edge.distance = bf.doubleProperty(space);
    edge.innerPen.setWidthF(bf.doubleProperty(innerWidth));

    d->edges[side] = edge;
}

bool KoTextBlockBorderData::ref()
{
    return d->refCount.ref();
}

bool KoTextBlockBorderData::deref()
{
    return d->refCount.deref();
}

int KoTextBlockBorderData::useCount() const
{
    return d->refCount;
}

QRectF KoTextBlockBorderData::rect() const
{
    return d->bounds;
}

