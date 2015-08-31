/* This file is part of the KDE project
 *
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006-2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2007,2009 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2012      Inge Wallin <inge@lysator.liu.se>
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

// Own
#include "KoShapeStroke.h"

// Posix
#include <math.h>

// Qt
#include <QPainterPath>
#include <QPainter>

// Calligra
#include <KoGenStyles.h>
#include <KoOdfGraphicStyles.h>

// Flake
#include "KoViewConverter.h"
#include "KoShape.h"
#include "KoShapeSavingContext.h"
#include "KoPathShape.h"
#include "KoMarkerData.h"


class Q_DECL_HIDDEN KoShapeStroke::Private
{
public:
    void paintBorder(KoShape *shape, QPainter &painter, const QPen &pen) const;
    QColor color;
    QPen pen;
    QBrush brush;
};

void KoShapeStroke::Private::paintBorder(KoShape *shape, QPainter &painter, const QPen &pen) const
{
    if (!pen.isCosmetic() && pen.style() != Qt::NoPen) {
        KoPathShape *pathShape = dynamic_cast<KoPathShape *>(shape);
        if (pathShape) {
            QPainterPath path = pathShape->pathStroke(pen);
            painter.fillPath(path, pen.brush());
            return;
        }
        painter.strokePath(shape->outline(), pen);
    }
}


KoShapeStroke::KoShapeStroke()
        : d(new Private())
{
    d->color = QColor(Qt::black);
    // we are not rendering stroke with zero width anymore
    // so lets use a default width of 1.0
    d->pen.setWidthF(1.0);
}

KoShapeStroke::KoShapeStroke(const KoShapeStroke &other)
        : KoShapeStrokeModel(), d(new Private())
{
    d->color = other.d->color;
    d->pen = other.d->pen;
    d->brush = other.d->brush;
}

KoShapeStroke::KoShapeStroke(qreal lineWidth, const QColor &color)
        : d(new Private())
{
    d->pen.setWidthF(qMax(qreal(0.0), lineWidth));
    d->pen.setJoinStyle(Qt::MiterJoin);
    d->color = color;
}

KoShapeStroke::~KoShapeStroke()
{
    delete d;
}

KoShapeStroke &KoShapeStroke::operator = (const KoShapeStroke &rhs)
{
    if (this == &rhs)
        return *this;

    d->pen = rhs.d->pen;
    d->color = rhs.d->color;
    d->brush = rhs.d->brush;

    return *this;
}

void KoShapeStroke::fillStyle(KoGenStyle &style, KoShapeSavingContext &context) const
{
    QPen pen = d->pen;
    if (d->brush.gradient())
        pen.setBrush(d->brush);
    else
        pen.setColor(d->color);
    KoOdfGraphicStyles::saveOdfStrokeStyle(style, context.mainStyles(), pen);
}

void KoShapeStroke::strokeInsets(const KoShape *shape, KoInsets &insets) const
{
    Q_UNUSED(shape);
    qreal lineWidth = d->pen.widthF();
    if (lineWidth < 0) {
        lineWidth = 1;
    }
    lineWidth *= 0.5; // since we draw a line half inside, and half outside the object.

    // if we have square cap, we need a little more space
    // -> sqrt((0.5*penWidth)^2 + (0.5*penWidth)^2)
    if (capStyle() == Qt::SquareCap) {
        lineWidth *= M_SQRT2;
    }

    if (joinStyle() == Qt::MiterJoin) {
        lineWidth = qMax(lineWidth, miterLimit());
    }

    insets.top = lineWidth;
    insets.bottom = lineWidth;
    insets.left = lineWidth;
    insets.right = lineWidth;
}

bool KoShapeStroke::hasTransparency() const
{
    return d->color.alpha() > 0;
}

void KoShapeStroke::paint(KoShape *shape, QPainter &painter, const KoViewConverter &converter)
{
    KoShape::applyConversion(painter, converter);

    QPen pen = d->pen;

    if (d->brush.gradient())
        pen.setBrush(d->brush);
    else
        pen.setColor(d->color);

    d->paintBorder(shape, painter, pen);
}

void KoShapeStroke::paint(KoShape *shape, QPainter &painter, const KoViewConverter &converter, const QColor &color)
{
    KoShape::applyConversion(painter, converter);

    QPen pen = d->pen;
    pen.setColor(color);

    d->paintBorder(shape, painter, pen);
}

void KoShapeStroke::setCapStyle(Qt::PenCapStyle style)
{
    d->pen.setCapStyle(style);
}

Qt::PenCapStyle KoShapeStroke::capStyle() const
{
    return d->pen.capStyle();
}

void KoShapeStroke::setJoinStyle(Qt::PenJoinStyle style)
{
    d->pen.setJoinStyle(style);
}

Qt::PenJoinStyle KoShapeStroke::joinStyle() const
{
    return d->pen.joinStyle();
}

void KoShapeStroke::setLineWidth(qreal lineWidth)
{
    d->pen.setWidthF(qMax(qreal(0.0), lineWidth));
}

qreal KoShapeStroke::lineWidth() const
{
    return d->pen.widthF();
}

void KoShapeStroke::setMiterLimit(qreal miterLimit)
{
    d->pen.setMiterLimit(miterLimit);
}

qreal KoShapeStroke::miterLimit() const
{
    return d->pen.miterLimit();
}

QColor KoShapeStroke::color() const
{
    return d->color;
}

void KoShapeStroke::setColor(const QColor &color)
{
    d->color = color;
}

void KoShapeStroke::setLineStyle(Qt::PenStyle style, const QVector<qreal> &dashes)
{
    if (style < Qt::CustomDashLine)
        d->pen.setStyle(style);
    else
        d->pen.setDashPattern(dashes);
}

Qt::PenStyle KoShapeStroke::lineStyle() const
{
    return d->pen.style();
}

QVector<qreal> KoShapeStroke::lineDashes() const
{
    return d->pen.dashPattern();
}

void KoShapeStroke::setDashOffset(qreal dashOffset)
{
    d->pen.setDashOffset(dashOffset);
}

qreal KoShapeStroke::dashOffset() const
{
    return d->pen.dashOffset();
}

void KoShapeStroke::setLineBrush(const QBrush &brush)
{
    d->brush = brush;
}

QBrush KoShapeStroke::lineBrush() const
{
    return d->brush;
}
