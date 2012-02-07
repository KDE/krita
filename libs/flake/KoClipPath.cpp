/* This file is part of the KDE project
 * Copyright (C) 2011 Jan Hambrecht <jaham@gmx.net>
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

#include "KoClipPath.h"
#include "KoPathShape.h"
#include "KoViewConverter.h"

#include <QTransform>
#include <QPainterPath>
#include <QPainter>
#include <QGraphicsItem>
#include <QVarLengthArray>

QTransform scaleToPercent(const QSizeF &size)
{
    const qreal w = qMax(static_cast<qreal>(1e-5), size.width());
    const qreal h = qMax(static_cast<qreal>(1e-5), size.height());
    return QTransform().scale(100/w, 100/h);
}

QTransform scaleFromPercent(const QSizeF &size)
{
    const qreal w = qMax(static_cast<qreal>(1e-5), size.width());
    const qreal h = qMax(static_cast<qreal>(1e-5), size.height());
    return QTransform().scale(w/100, h/100);
}

class KoClipData::Private
{
public:
    Private() : deleteClipShapes(true)
    {
    }

    ~Private()
    {
        if (deleteClipShapes)
            qDeleteAll(clipPathShapes);
    }

    QList<KoPathShape*> clipPathShapes;
    bool deleteClipShapes;
};

KoClipData::KoClipData(KoPathShape *clipPathShape)
        : d(new Private())
{
    Q_ASSERT(clipPathShape);
    d->clipPathShapes.append(clipPathShape);
}

KoClipData::KoClipData(const QList<KoPathShape*> &clipPathShapes)
        : d(new Private())
{
    Q_ASSERT(clipPathShapes.count());
    d->clipPathShapes = clipPathShapes;
}

KoClipData::~KoClipData()
{
    delete d;
}

QList<KoPathShape*> KoClipData::clipPathShapes() const
{
    return d->clipPathShapes;
}

void KoClipData::removeClipShapesOwnership()
{
    d->deleteClipShapes = false;
}

class KoClipPath::Private
{
public:
    Private(KoClipData *data)
            : clipData(data)
    {}

    ~Private()
    {
    }

    void compileClipPath(KoShape *clippedShape)
    {
        QList<KoPathShape*> clipShapes = clipData->clipPathShapes();
        if (!clipShapes.count())
            return;

        initialShapeSize = clippedShape->outline().boundingRect().size();
        initialTransformToShape = clippedShape->absoluteTransformation(0).inverted();

        QTransform transformToShape = initialTransformToShape * scaleToPercent(initialShapeSize);

        foreach(KoPathShape *path, clipShapes) {
            if (!path)
                continue;
            // map clip path to shape coordinates of clipped shape
            QTransform m = path->absoluteTransformation(0) * transformToShape;
            if (clipPath.isEmpty())
                clipPath = m.map(path->outline());
            else
                clipPath |= m.map(path->outline());
        }
    }

    QExplicitlySharedDataPointer<KoClipData> clipData; ///< the clip path data
    QPainterPath clipPath; ///< the compiled clip path in shape coordinates of the clipped shape
    QTransform initialTransformToShape; ///< initial transformation to shape coordinates of the clipped shape
    QSizeF initialShapeSize; ///< initial size of clipped shape
};

KoClipPath::KoClipPath(KoShape *clippedShape, KoClipData *clipData)
        : d( new Private(clipData) )
{
    d->compileClipPath(clippedShape);
}

KoClipPath::~KoClipPath()
{
    delete d;
}

void KoClipPath::setClipRule(Qt::FillRule clipRule)
{
    d->clipPath.setFillRule(clipRule);
}

Qt::FillRule KoClipPath::clipRule() const
{
    return d->clipPath.fillRule();
}

void KoClipPath::applyClipping(KoShape *clippedShape, QPainter &painter, const KoViewConverter &converter)
{
    QPainterPath clipPath;
    KoShape *shape = clippedShape;
    while (shape) {
        if (shape->clipPath()) {
            QTransform m = scaleFromPercent(shape->outline().boundingRect().size()) * shape->absoluteTransformation(0);
            if (clipPath.isEmpty())
                clipPath = m.map(shape->clipPath()->path());
            else
                clipPath |= m.map(shape->clipPath()->path());
        }
        shape = shape->parent();
    }

    if (!clipPath.isEmpty()) {
        QTransform viewMatrix;
        qreal zoomX, zoomY;
        converter.zoom(&zoomX, &zoomY);
        viewMatrix.scale(zoomX, zoomY);
        painter.setClipPath(viewMatrix.map(clipPath), Qt::IntersectClip);
    }
}

QPainterPath KoClipPath::path() const
{
    return d->clipPath;
}

QPainterPath KoClipPath::pathForSize(const QSizeF &size) const
{
    return scaleFromPercent(size).map(d->clipPath);
}

QList<KoPathShape*> KoClipPath::clipPathShapes() const
{
    return d->clipData->clipPathShapes();
}

QTransform KoClipPath::clipDataTransformation(KoShape *clippedShape) const
{
    if (!clippedShape)
        return d->initialTransformToShape;

    // the current transformation of the clipped shape
    QTransform currentShapeTransform = clippedShape->absoluteTransformation(0);

    // calculate the transformation which represents any resizing of the clipped shape
    const QSizeF currentShapeSize = clippedShape->outline().boundingRect().size();
    const qreal sx = currentShapeSize.width() / d->initialShapeSize.width();
    const qreal sy = currentShapeSize.height() / d->initialShapeSize.height();
    QTransform scaleTransform = QTransform().scale(sx, sy);

    // 1. transform to initial clipped shape coordinates
    // 2. apply resizing transfromation
    // 3. convert to current clipped shape document coordinates
    return d->initialTransformToShape * scaleTransform * currentShapeTransform;
}
