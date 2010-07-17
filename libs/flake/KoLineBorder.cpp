/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006-2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2007,2009 Thorsten Zachmann <zachmann@kde.org>
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
#include "KoShapeSavingContext.h"

#include <QPainterPath>

#include <KoGenStyle.h>
#include <KoGenStyles.h>
#include <KoOdfGraphicStyles.h>

#include <math.h>

class KoLineBorder::Private
{
public:
    QColor color;
    QPen pen;
    QBrush brush;
};

KoLineBorder::KoLineBorder()
        : d(new Private())
{
    d->color = QColor(Qt::black);
    // we are not rendering stroke with zero width anymore
    // so lets use a default width of 1.0
    d->pen.setWidthF(1.0);
}

KoLineBorder::KoLineBorder(const KoLineBorder &other)
        : KoShapeBorderModel(), d(new Private())
{
    d->color = other.d->color;
    d->pen = other.d->pen;
    d->brush = other.d->brush;
}

KoLineBorder::KoLineBorder(qreal lineWidth, const QColor &color)
        : d(new Private())
{
    d->pen.setWidthF(qMax(qreal(0.0), lineWidth));
    d->pen.setJoinStyle(Qt::MiterJoin);
    d->color = color;
}

KoLineBorder::~KoLineBorder()
{
    delete d;
}

KoLineBorder& KoLineBorder::operator = (const KoLineBorder &rhs)
{
    if (this == &rhs)
        return *this;

    d->pen = rhs.d->pen;
    d->color = rhs.d->color;
    d->brush = rhs.d->brush;

    return *this;
}

void KoLineBorder::fillStyle(KoGenStyle &style, KoShapeSavingContext &context) const
{
    QPen pen = d->pen;
    if (d->brush.gradient())
        pen.setBrush(d->brush);
    else
        pen.setColor(d->color);
    KoOdfGraphicStyles::saveOdfStrokeStyle(style, context.mainStyles(), pen);
}

void KoLineBorder::borderInsets(const KoShape *shape, KoInsets &insets) const
{
    Q_UNUSED(shape);
    qreal lineWidth = d->pen.widthF();
    if (lineWidth < 0)
        lineWidth = 1;
    lineWidth *= 0.5; // since we draw a line half inside, and half outside the object.

    // if we have square cap, we need a little more space
    // -> sqrt( (0.5*penWidth)^2 + (0.5*penWidth)^2 )
    if (capStyle() == Qt::SquareCap)
        lineWidth *= M_SQRT2;

    insets.top = lineWidth;
    insets.bottom = lineWidth;
    insets.left = lineWidth;
    insets.right = lineWidth;
}

bool KoLineBorder::hasTransparency() const
{
    return d->color.alpha() > 0;
}

void KoLineBorder::paint(KoShape *shape, QPainter &painter, const KoViewConverter &converter)
{
    KoShape::applyConversion(painter, converter);

    QPen pen = d->pen;

    if (d->brush.gradient())
        pen.setBrush(d->brush);
    else
        pen.setColor(d->color);

    if (!pen.isCosmetic())
        painter.strokePath(shape->outline(), pen);
}

void KoLineBorder::paint(KoShape *shape, QPainter &painter, const KoViewConverter &converter, const QColor &color )
{
    KoShape::applyConversion(painter, converter);

    QPen pen = d->pen;
    pen.setColor(color);

    if (!pen.isCosmetic()) {
        painter.strokePath(shape->outline(), pen);
    }
}

void KoLineBorder::setCapStyle(Qt::PenCapStyle style)
{
    d->pen.setCapStyle(style);
}

Qt::PenCapStyle KoLineBorder::capStyle() const
{
    return d->pen.capStyle();
}

void KoLineBorder::setJoinStyle(Qt::PenJoinStyle style)
{
    d->pen.setJoinStyle(style);
}

Qt::PenJoinStyle KoLineBorder::joinStyle() const
{
    return d->pen.joinStyle();
}

void KoLineBorder::setLineWidth(qreal lineWidth)
{
    d->pen.setWidthF(qMax(qreal(0.0), lineWidth));
}

qreal KoLineBorder::lineWidth() const
{
    return d->pen.widthF();
}

void KoLineBorder::setMiterLimit(qreal miterLimit)
{
    d->pen.setMiterLimit(miterLimit);
}

qreal KoLineBorder::miterLimit() const
{
    return d->pen.miterLimit();
}

const QColor & KoLineBorder::color() const
{
    return d->color;
}

void KoLineBorder::setColor(const QColor & color)
{
    d->color = color;
}

void KoLineBorder::setLineStyle(Qt::PenStyle style, const QVector<qreal> &dashes)
{
    if (style < Qt::CustomDashLine)
        d->pen.setStyle(style);
    else
        d->pen.setDashPattern(dashes);
}

Qt::PenStyle KoLineBorder::lineStyle() const
{
    return d->pen.style();
}

QVector<qreal> KoLineBorder::lineDashes() const
{
    return d->pen.dashPattern();
}

void KoLineBorder::setDashOffset(qreal dashOffset)
{
    d->pen.setDashOffset(dashOffset);
}

qreal KoLineBorder::dashOffset() const
{
    return d->pen.dashOffset();
}

void KoLineBorder::setLineBrush(const QBrush &brush)
{
    d->brush = brush;
}

QBrush KoLineBorder::lineBrush() const
{
    return d->brush;
}
