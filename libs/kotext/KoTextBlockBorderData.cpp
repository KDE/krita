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

KoTextBlockBorderData::KoTextBlockBorderData(const QRectF &paragRect)
    : m_refCount(0)
{
    m_bounds = paragRect;
}

KoTextBlockBorderData::KoTextBlockBorderData(const KoTextBlockBorderData &other)
    : m_refCount(0)
{
    for(int i=Top; i <= Right; i++)
        m_edges[i] = other.m_edges[i];
    m_bounds = other.m_bounds;
}

bool KoTextBlockBorderData::hasBorders() const {
    for(int i=Top; i <= Right; i++)
        if(m_edges[i].outerPen.widthF() > 0.0)
            return true;
    return false;
}

bool KoTextBlockBorderData::operator==(const KoTextBlockBorderData &border) {
    return equals(border);
}
bool KoTextBlockBorderData::equals(const KoTextBlockBorderData &border) {
    for(int i=Top; i <= Right; i++) {
        if(m_edges[i].outerPen != border.m_edges[i].outerPen)
            return false;
        if(m_edges[i].innerPen != border.m_edges[i].innerPen)
            return false;
        if(qAbs(m_edges[i].distance - border.m_edges[i].distance) > 1E-10)
            return false;
    }
    return true;
}

void KoTextBlockBorderData::applyInsets(KoInsets &insets, double paragStart) const {
    insets.left += inset(Left);
    insets.right += inset(Right);

    // only apply top when the parag is the top parag in the border-set
    if(qAbs(m_bounds.top() - paragStart) < 1E-10)
        insets.top += inset(Top);
}

void KoTextBlockBorderData::setParagraphBottom(double bottom) {
    m_bounds.setBottom(bottom + inset(Bottom));
}

void KoTextBlockBorderData::paint(QPainter &painter) const {
    QRectF bounds = m_bounds;
    QRectF innerBounds = m_bounds;
    if(m_edges[Top].outerPen.widthF() > 0) {
        QPen pen = m_edges[Top].outerPen;

        painter.setPen(pen);
        const double t = m_bounds.top() + pen.widthF() / 2.0;
        painter.drawLine(QLineF(m_bounds.left(), t, m_bounds.right(), t));
        innerBounds.setTop(m_bounds.top() + m_edges[Top].distance + pen.widthF());
    }
    if(m_edges[Bottom].outerPen.widthF() > 0) {
        QPen pen = m_edges[Bottom].outerPen;
        painter.setPen(pen);
        const double b = m_bounds.bottom() - pen.widthF() / 2.0;
        innerBounds.setBottom(m_bounds.bottom() - m_edges[Bottom].distance - pen.widthF());
        painter.drawLine(QLineF(m_bounds.left(), b, m_bounds.right(), b));
    }
    if(m_edges[Left].outerPen.widthF() > 0) {
        QPen pen = m_edges[Left].outerPen;
        painter.setPen(pen);
        const double l = m_bounds.left() + pen.widthF() / 2.0;
        innerBounds.setLeft(m_bounds.left() + m_edges[Left].distance + pen.widthF());
        painter.drawLine(QLineF(l, m_bounds.top(), l, m_bounds.bottom()));
    }
    if(m_edges[Right].outerPen.widthF() > 0) {
        QPen pen = m_edges[Right].outerPen;
        painter.setPen(pen);
        const double r = m_bounds.right() - pen.widthF() / 2.0;
        innerBounds.setRight(m_bounds.right() - m_edges[Right].distance - pen.widthF());
        painter.drawLine(QLineF(r, m_bounds.top(), r, m_bounds.bottom()));
    }
    // inner lines
    if(m_edges[Top].innerPen.widthF() > 0) {
        QPen pen = m_edges[Top].innerPen;
        painter.setPen(pen);
        const double t = innerBounds.top() + pen.widthF() / 2.0;
        painter.drawLine(QLineF(innerBounds.left(), t, innerBounds.right(), t));
    }
    if(m_edges[Bottom].innerPen.widthF() > 0) {
        QPen pen = m_edges[Bottom].innerPen;
        painter.setPen(pen);
        const double b = innerBounds.bottom() - pen.widthF() / 2.0;
        painter.drawLine(QLineF(innerBounds.left(), b, innerBounds.right(), b));
    }
    if(m_edges[Left].innerPen.widthF() > 0) {
        QPen pen = m_edges[Left].innerPen;
        painter.setPen(pen);
        const double l = innerBounds.left() + pen.widthF() / 2.0;
        painter.drawLine(QLineF(l, innerBounds.top(), l, innerBounds.bottom()));
    }
    if(m_edges[Right].innerPen.widthF() > 0) {
        QPen pen = m_edges[Right].innerPen;
        painter.setPen(pen);
        const double r = innerBounds.right() - pen.widthF() / 2.0;
        painter.drawLine(QLineF(r, innerBounds.top(), r, innerBounds.bottom()));
    }
}

double KoTextBlockBorderData::inset(Side side) const {
    return m_edges[side].outerPen.widthF() + m_edges[side].distance + m_edges[side].innerPen.widthF();
}

void KoTextBlockBorderData::setEdge(Side side, const QTextBlockFormat &bf,
        KoParagraphStyle::Property style, KoParagraphStyle::Property width,
        KoParagraphStyle::Property color, KoParagraphStyle::Property space,
        KoParagraphStyle::Property innerWidth) {

    Edge edge;
    KoParagraphStyle::BorderStyle borderStyle;
    borderStyle = static_cast<KoParagraphStyle::BorderStyle> (bf.intProperty(style));
    switch(borderStyle) {
        case KoParagraphStyle::BorderDotted: edge.innerPen.setStyle(Qt::DotLine); break;
        case KoParagraphStyle::BorderDashed: edge.innerPen.setStyle(Qt::DashLine); break;
        case KoParagraphStyle::BorderDashDotPattern: edge.innerPen.setStyle(Qt::DashDotLine); break;
        case KoParagraphStyle::BorderDashDotDotPattern: edge.innerPen.setStyle(Qt::DashDotDotLine); break;
        case KoParagraphStyle::BorderGroove: /* TODO */ break;
        case KoParagraphStyle::BorderRidge: /* TODO */ break;
        case KoParagraphStyle::BorderInset: /* TODO */ break;
        case KoParagraphStyle::BorderOutset: /* TODO */ break;
        default:
            edge.innerPen.setStyle(Qt::SolidLine);
    }
    edge.innerPen.setColor(bf.colorProperty(color));
    edge.innerPen.setJoinStyle( Qt::MiterJoin );
    edge.innerPen.setCapStyle(Qt::FlatCap);
    edge.outerPen = edge.innerPen;
    edge.outerPen.setWidthF( bf.doubleProperty(width) ); // TODO check if this does not need any conversion

    edge.distance = bf.doubleProperty(space);
    edge.innerPen.setWidthF( bf.doubleProperty(innerWidth) );

    m_edges[side] = edge;
}
