/* This file is part of the KDE project
 * Copyright (C) 2008-2009 Jan Hambrecht <jaham@gmx.net>
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

#include "KoSnapGuide.h"
#include "KoSnapProxy.h"
#include "KoSnapStrategy.h"

#include <KoPathShape.h>
#include <KoPathPoint.h>
#include <KoViewConverter.h>
#include <KoCanvasBase.h>
#include <KoShapeManager.h>

#include <QtGui/QPainter>

#include <math.h>


class KoSnapGuide::Private
{
public:
    Private(KoCanvasBase *parentCanvas)
    : canvas(parentCanvas), editedShape(0), currentStrategy(0),
    active(true),
    snapDistance(10)
    {
    }

    ~Private()
    {
        qDeleteAll( strategies );
        strategies.clear();
    }

    KoCanvasBase * canvas;
    KoShape * editedShape;

    QList<KoSnapStrategy*> strategies;
    KoSnapStrategy * currentStrategy;

    KoSnapGuide::Strategies usedStrategies;
    bool active;
    int snapDistance;
    QList<KoPathPoint*> ignoredPoints;
    QList<KoShape*> ignoredShapes;
};

KoSnapGuide::KoSnapGuide(KoCanvasBase * canvas)
: d(new Private(canvas))
{
    d->strategies.append(new GridSnapStrategy());
    d->strategies.append(new NodeSnapStrategy());
    d->strategies.append(new OrthogonalSnapStrategy());
    d->strategies.append(new ExtensionSnapStrategy());
    d->strategies.append(new IntersectionSnapStrategy());
    d->strategies.append(new BoundingBoxSnapStrategy());
    d->strategies.append(new LineGuideSnapStrategy());
}

KoSnapGuide::~KoSnapGuide()
{
    delete d;
}

void KoSnapGuide::setEditedShape(KoShape * shape)
{
    d->editedShape = shape;
}

KoShape * KoSnapGuide::editedShape() const
{
    return d->editedShape;
}

void KoSnapGuide::enableSnapStrategies(Strategies strategies)
{
    d->usedStrategies = strategies;
}

KoSnapGuide::Strategies KoSnapGuide::enabledSnapStrategies() const
{
    return d->usedStrategies;
}

bool KoSnapGuide::addCustomSnapStrategy(KoSnapStrategy * customStrategy)
{
    if (!customStrategy || customStrategy->type() != CustomSnapping)
        return false;

    d->strategies.append(customStrategy);
    return true;
}

void KoSnapGuide::enableSnapping(bool on)
{
    d->active = on;
}

bool KoSnapGuide::isSnapping() const
{
    return d->active;
}

void KoSnapGuide::setSnapDistance(int distance)
{
    d->snapDistance = qAbs(distance);
}

int KoSnapGuide::snapDistance() const
{
    return d->snapDistance;
}

QPointF KoSnapGuide::snap(const QPointF &mousePosition, Qt::KeyboardModifiers modifiers)
{
    d->currentStrategy = 0;

    if (! d->active || (modifiers & Qt::ShiftModifier))
        return mousePosition;

    KoSnapProxy proxy(this);

    qreal minDistance = HUGE_VAL;

    qreal maxSnapDistance = d->canvas->viewConverter()->viewToDocument(QSizeF(d->snapDistance, d->snapDistance)).width();

    foreach(KoSnapStrategy * strategy, d->strategies) {
        if (d->usedStrategies & strategy->type()
                || strategy->type() == GridSnapping || strategy->type() == CustomSnapping) {
            if (! strategy->snap(mousePosition, &proxy, maxSnapDistance))
                continue;

            QPointF snapCandidate = strategy->snappedPosition();
            qreal distance = KoSnapStrategy::squareDistance(snapCandidate, mousePosition);
            if (distance < minDistance) {
                d->currentStrategy = strategy;
                minDistance = distance;
            }
        }
    }

    if (! d->currentStrategy)
        return mousePosition;

    return d->currentStrategy->snappedPosition();
}

QRectF KoSnapGuide::boundingRect()
{
    QRectF rect;

    if (d->currentStrategy) {
        rect = d->currentStrategy->decoration(*d->canvas->viewConverter()).boundingRect();
        return rect.adjusted(-2, -2, 2, 2);
    } else {
        return rect;
    };
}

void KoSnapGuide::paint(QPainter &painter, const KoViewConverter &converter)
{
    if (! d->currentStrategy || ! d->active)
        return;

    QPainterPath decoration = d->currentStrategy->decoration(converter);

    painter.setBrush(Qt::NoBrush);

    QPen whitePen(Qt::white);
    whitePen.setStyle(Qt::SolidLine);
    painter.setPen(whitePen);
    painter.drawPath(decoration);

    QPen redPen(Qt::red);
    redPen.setStyle(Qt::DotLine);
    painter.setPen(redPen);
    painter.drawPath(decoration);
}

KoCanvasBase * KoSnapGuide::canvas() const
{
    return d->canvas;
}

void KoSnapGuide::setIgnoredPathPoints(const QList<KoPathPoint*> &ignoredPoints)
{
    d->ignoredPoints = ignoredPoints;
}

QList<KoPathPoint*> KoSnapGuide::ignoredPathPoints() const
{
    return d->ignoredPoints;
}

void KoSnapGuide::setIgnoredShapes(const QList<KoShape*> &ignoredShapes)
{
    d->ignoredShapes = ignoredShapes;
}

QList<KoShape*> KoSnapGuide::ignoredShapes() const
{
    return d->ignoredShapes;
}

void KoSnapGuide::reset()
{
    d->currentStrategy = 0;
    d->editedShape = 0;
    d->ignoredPoints.clear();
    d->ignoredShapes.clear();
    // remove all custom strategies
    int strategyCount = d->strategies.count();
    for(int i = strategyCount-1; i >= 0; --i) {
        if (d->strategies[i]->type() == CustomSnapping) {
            delete d->strategies[i];
            d->strategies.removeAt(i);
        }
    }
}

