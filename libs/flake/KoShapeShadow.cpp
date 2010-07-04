/* This file is part of the KDE project
 * Copyright (C) 2008-2009 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
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

#include "KoShapeShadow.h"
#include "KoShapeSavingContext.h"
#include "KoShapeBorderModel.h"
#include "KoShape.h"
#include "KoInsets.h"
#include "KoPathShape.h"
#include <KoGenStyle.h>
#include <KoViewConverter.h>
#include <QtGui/QPainter>
#include <QtCore/QAtomicInt>

class KoShapeShadow::Private
{
public:
    Private()
            : offset(10, 10), color(Qt::black), visible(true), refCount(0) {
    }
    QPointF offset;
    QColor color;
    bool visible;
    QAtomicInt refCount;
};

KoShapeShadow::KoShapeShadow()
        : d(new Private())
{
}

KoShapeShadow::~KoShapeShadow()
{
    delete d;
}

void KoShapeShadow::fillStyle(KoGenStyle &style, KoShapeSavingContext &context)
{
    Q_UNUSED(context);

    style.addProperty("draw:shadow", d->visible ? "visible" : "hidden");
    style.addProperty("draw:shadow-color", d->color.name());
    if (d->color.alphaF() != 1.0)
        style.addProperty("draw:shadow-opacity", QString("%1%").arg(d->color.alphaF() * 100.0));
    style.addProperty("draw:shadow-offset-x", QString("%1pt").arg(d->offset.x()));
    style.addProperty("draw:shadow-offset-y", QString("%1pt").arg(d->offset.y()));
}

void KoShapeShadow::paint(KoShape *shape, QPainter &painter, const KoViewConverter &converter)
{
    if (! d->visible)
        return;

    // calculate the shadow offset independent of shape transformation
    QTransform tm;
    tm.translate(d->offset.x(), d->offset.y());
    QTransform tr = shape->absoluteTransformation(&converter);
    QTransform offsetMatrix = tr * tm * tr.inverted();

    if (shape->background()) {
        painter.save();

        KoShape::applyConversion(painter, converter);

        // the shadow direction is independent of the shapes transformation
        // please only change if you know what you are doing
        painter.setTransform(offsetMatrix * painter.transform());
        painter.setBrush(QBrush(d->color));

        QPainterPath path(shape->outline());
        KoPathShape * pathShape = dynamic_cast<KoPathShape*>(shape);
        if (pathShape)
            path.setFillRule(pathShape->fillRule());
        painter.drawPath(path);
        painter.restore();
    }

    if (shape->border()) {
        QTransform oldPainterMatrix = painter.transform();
        KoShape::applyConversion(painter, converter);
        QTransform newPainterMatrix = painter.transform();

        // the shadow direction is independent of the shapes transformation
        // please only change if you know what you are doing
        painter.setTransform(offsetMatrix * painter.transform());

        // compensate applyConversion call in paint
        QTransform scaleMatrix = newPainterMatrix * oldPainterMatrix.inverted();
        painter.setTransform(scaleMatrix.inverted() * painter.transform());

        shape->border()->paint(shape, painter, converter, d->color);
    }
}

void KoShapeShadow::setOffset(const QPointF & offset)
{
    d->offset = offset;
}

QPointF KoShapeShadow::offset() const
{
    return d->offset;
}

void KoShapeShadow::setColor(const QColor &color)
{
    d->color = color;
}

QColor KoShapeShadow::color() const
{
    return d->color;
}

void KoShapeShadow::setVisible(bool visible)
{
    d->visible = visible;
}

bool KoShapeShadow::isVisible() const
{
    return d->visible;
}

void KoShapeShadow::insets(KoInsets &insets) const
{
    if (!d->visible) {
        insets.top = 0;
        insets.bottom = 0;
        insets.left = 0;
        insets.right = 0;
        return;
    }

    insets.left = (d->offset.x() < 0.0) ? qAbs(d->offset.x()) : 0.0;
    insets.top = (d->offset.y() < 0.0) ? qAbs(d->offset.y()) : 0.0;
    insets.right = (d->offset.x() > 0.0) ? d->offset.x() : 0.0;
    insets.bottom = (d->offset.y() > 0.0) ? d->offset.y() : 0.0;
}

bool KoShapeShadow::ref()
{
    return d->refCount.ref();
}

bool KoShapeShadow::deref()
{
    return d->refCount.deref();
}

int KoShapeShadow::useCount() const
{
    return d->refCount;
}
