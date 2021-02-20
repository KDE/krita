/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2008-2009 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoSnapGuide.h"
#include "KoSnapProxy.h"
#include "KoSnapStrategy.h"

#include <KoPathShape.h>
#include <KoPathPoint.h>
#include <KoViewConverter.h>
#include <KoCanvasBase.h>

#include <QPainter>
#include <QPainterPath>

#include <math.h>

template <class T>
inline QSharedPointer<T> toQShared(T* ptr) {
    return QSharedPointer<T>(ptr);
}

class Q_DECL_HIDDEN KoSnapGuide::Private
{
public:
    Private(KoCanvasBase *parentCanvas)
        : canvas(parentCanvas), additionalEditedShape(0), currentStrategy(0),
        active(true),
        snapDistance(10)
    {
    }

    ~Private()
    {
        strategies.clear();
    }

    KoCanvasBase *canvas;
    KoShape *additionalEditedShape;

    typedef QSharedPointer<KoSnapStrategy> KoSnapStrategySP;
    typedef QList<KoSnapStrategySP> StrategiesList;
    StrategiesList strategies;
    KoSnapStrategySP currentStrategy;

    KoSnapGuide::Strategies usedStrategies;
    bool active;
    int snapDistance;
    QList<KoPathPoint*> ignoredPoints;
    QList<KoShape*> ignoredShapes;
};

KoSnapGuide::KoSnapGuide(KoCanvasBase *canvas)
    : d(new Private(canvas))
{
    d->strategies.append(toQShared(new GridSnapStrategy()));
    d->strategies.append(toQShared(new NodeSnapStrategy()));
    d->strategies.append(toQShared(new OrthogonalSnapStrategy()));
    d->strategies.append(toQShared(new ExtensionSnapStrategy()));
    d->strategies.append(toQShared(new IntersectionSnapStrategy()));
    d->strategies.append(toQShared(new BoundingBoxSnapStrategy()));
}

KoSnapGuide::~KoSnapGuide()
{
}

void KoSnapGuide::setAdditionalEditedShape(KoShape *shape)
{
    d->additionalEditedShape = shape;
}

KoShape *KoSnapGuide::additionalEditedShape() const
{
    return d->additionalEditedShape;
}

void KoSnapGuide::enableSnapStrategy(Strategy type, bool value)
{
    if (value) {
        d->usedStrategies |= type;
    } else {
        d->usedStrategies &= ~type;
    }
}

bool KoSnapGuide::isStrategyEnabled(Strategy type) const
{
    return d->usedStrategies & type;
}

void KoSnapGuide::enableSnapStrategies(Strategies strategies)
{
    d->usedStrategies = strategies;
}

KoSnapGuide::Strategies KoSnapGuide::enabledSnapStrategies() const
{
    return d->usedStrategies;
}

bool KoSnapGuide::addCustomSnapStrategy(KoSnapStrategy *customStrategy)
{
    if (!customStrategy || customStrategy->type() != CustomSnapping)
        return false;

    d->strategies.append(toQShared(customStrategy));
    return true;
}

void KoSnapGuide::overrideSnapStrategy(Strategy type, KoSnapStrategy *strategy)
{
    for (auto it = d->strategies.begin(); it != d->strategies.end(); /*noop*/) {
        if ((*it)->type() == type) {
            if (strategy) {
                *it = toQShared(strategy);
            } else {
                it = d->strategies.erase(it);
            }
            return;
        } else {
            ++it;
        }
    }

    if (strategy) {
        d->strategies.append(toQShared(strategy));
    }
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

QPointF KoSnapGuide::snap(const QPointF &mousePosition, const QPointF &dragOffset, Qt::KeyboardModifiers modifiers)
{
    QPointF pos = mousePosition + dragOffset;
    pos = snap(pos, modifiers);
    return pos - dragOffset;
}

QPointF KoSnapGuide::snap(const QPointF &mousePosition, Qt::KeyboardModifiers modifiers)
{
    d->currentStrategy.clear();

    if (! d->active || (modifiers & Qt::ShiftModifier))
        return mousePosition;

    KoSnapProxy proxy(this);

    qreal minDistance = HUGE_VAL;

    qreal maxSnapDistance = d->canvas->viewConverter()->viewToDocument(QSizeF(d->snapDistance,
                d->snapDistance)).width();

    foreach (Private::KoSnapStrategySP strategy, d->strategies) {
        if (d->usedStrategies & strategy->type() ||
            strategy->type() == GridSnapping ||
            strategy->type() == CustomSnapping) {

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
    }
}

void KoSnapGuide::paint(QPainter &painter, const KoViewConverter &converter)
{
    if (! d->currentStrategy || ! d->active)
        return;

    QPainterPath decoration = d->currentStrategy->decoration(converter);

    painter.setBrush(Qt::NoBrush);

    QPen whitePen(Qt::white, 0);
    whitePen.setStyle(Qt::SolidLine);
    painter.setPen(whitePen);
    painter.drawPath(decoration);

    QPen redPen(Qt::red, 0);
    redPen.setStyle(Qt::DotLine);
    painter.setPen(redPen);
    painter.drawPath(decoration);
}

KoCanvasBase *KoSnapGuide::canvas() const
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
    d->currentStrategy.clear();
    d->additionalEditedShape = 0;
    d->ignoredPoints.clear();
    d->ignoredShapes.clear();
    // remove all custom strategies
    int strategyCount = d->strategies.count();
    for (int i = strategyCount-1; i >= 0; --i) {
        if (d->strategies[i]->type() == CustomSnapping) {
            d->strategies.removeAt(i);
        }
    }
}

