/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
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

#include <KoGenStyle.h>

class KoLineBorder::Private {
public:
    QColor color;
    QPen pen;
};

KoLineBorder::KoLineBorder()
    : d(new Private())
{
    d->color = QColor(Qt::black);
    d->pen.setWidthF( 0.0 );
}

KoLineBorder::KoLineBorder(double lineWidth, QColor color)
    : d(new Private())
{
    d->pen.setWidthF( qMax(0.0,lineWidth) );
    d->pen.setJoinStyle(Qt::MiterJoin);
}

KoLineBorder::~KoLineBorder() {
    delete d;
}

void KoLineBorder::fillStyle( KoGenStyle &style, KoShapeSavingContext &context )
{
    Q_UNUSED( context );
    // TODO implement all possibilities
    style.addProperty( "draw:stroke", "solid" );
    style.addProperty( "svg:stroke-color", color().name() );
    style.addPropertyPt( "svg:stroke-width", lineWidth() );
}

KoInsets* KoLineBorder::borderInsets(const KoShape *shape, KoInsets &insets) {
    Q_UNUSED(shape);
    double lineWidth = d->pen.widthF();
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
    return d->color.alpha() > 0;
}

void KoLineBorder::paintBorder(KoShape *shape, QPainter &painter, const KoViewConverter &converter) {
    KoShape::applyConversion( painter, converter );

    d->pen.setColor(d->color);
    painter.strokePath( shape->outline(), d->pen );
}

void KoLineBorder::setCapStyle( Qt::PenCapStyle style ) {
    d->pen.setCapStyle( style );
}

Qt::PenCapStyle KoLineBorder::capStyle() const {
    return d->pen.capStyle();
}

void KoLineBorder::setJoinStyle( Qt::PenJoinStyle style ) {
    d->pen.setJoinStyle( style );
}

Qt::PenJoinStyle KoLineBorder::joinStyle() const {
    return d->pen.joinStyle();
}

void KoLineBorder::setLineWidth( double lineWidth ) {
    d->pen.setWidthF( qMax(0.0,lineWidth) );
}

double KoLineBorder::lineWidth() const {
    return d->pen.widthF();
}

void KoLineBorder::setMiterLimit( double miterLimit ) {
    d->pen.setMiterLimit( miterLimit );
}

double KoLineBorder::miterLimit() const {
    return d->pen.miterLimit();
}

const QColor & KoLineBorder::color() const
{
    return d->color;
}

void KoLineBorder::setColor( const QColor & color )
{
    d->color = color;
}
