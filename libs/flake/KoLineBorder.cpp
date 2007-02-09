/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
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

#include "KoLineBorder.h"
#include "KoViewConverter.h"
#include "KoShape.h"

#include <QPainterPath>

KoLineBorder::KoLineBorder()
: m_color(Qt::black)
{
    m_pen.setWidthF( 0.0 );
}

KoLineBorder::KoLineBorder(double lineWidth, QColor color)
: m_color(color)
{
    m_pen.setWidthF( qMax(0.0,lineWidth) );
    m_pen.setJoinStyle(Qt::MiterJoin);
}

KoInsets* KoLineBorder::borderInsets(const KoShape *shape, KoInsets &insets) {
    Q_UNUSED(shape);
    double lineWidth = m_pen.widthF();
    if(lineWidth < 0)
         lineWidth = 1;
    lineWidth /= 2; // since we draw a line half inside, and half outside the object.
    insets.top = lineWidth;
    insets.bottom = lineWidth;
    insets.left = lineWidth;
    insets.right = lineWidth;
    return &insets;
}

bool KoLineBorder::hasTransparency() {
    return m_color.alpha() > 0;
}

void KoLineBorder::paintBorder(KoShape *shape, QPainter &painter, const KoViewConverter &converter) {
    KoShape::applyConversion( painter, converter );

    m_pen.setColor(m_color);
    painter.strokePath( shape->outline(), m_pen );
}

void KoLineBorder::setCapStyle( Qt::PenCapStyle style ) {
    m_pen.setCapStyle( style );
}

Qt::PenCapStyle KoLineBorder::capStyle() const {
    return m_pen.capStyle();
}

void KoLineBorder::setJoinStyle( Qt::PenJoinStyle style ) {
    m_pen.setJoinStyle( style );
}

Qt::PenJoinStyle KoLineBorder::joinStyle() const {
    return m_pen.joinStyle();
}

void KoLineBorder::setLineWidth( double lineWidth ) {
    m_pen.setWidthF( qMax(0.0,lineWidth) );
}

double KoLineBorder::lineWidth() const {
    return m_pen.widthF();
}

void KoLineBorder::setMiterLimit( double miterLimit ) {
    m_pen.setMiterLimit( miterLimit );
}

double KoLineBorder::miterLimit() const {
    return m_pen.miterLimit();
}

const QColor & KoLineBorder::color() const
{
    return m_color;
}

void KoLineBorder::setColor( const QColor & color )
{
    m_color = color;
}
