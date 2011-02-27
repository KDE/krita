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

#include <QtGui/QTransform>
#include <QtGui/QPainterPath>
#include <QtGui/QPainter>
#include <qgraphicsitem.h>

class KoClipData::Private
{
public:
    Private() : deleteClipShapes(true)
    {
    }
    
    ~Private()
    {
        if(deleteClipShapes)
            qDeleteAll(clipPathShapes);
    }
    
    QList<KoPathShape*> clipPathShapes;
    bool deleteClipShapes;
};

KoClipData::KoClipData(KoPathShape * clipPathShape)
    : d(new Private())
{
    Q_ASSERT(clipPathShape);
    d->clipPathShapes.append(clipPathShape);
}

KoClipData::KoClipData(const QList<KoPathShape*> & clipPathShapes)
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
    Private(KoClipData * data)
        : clipData(data)
    {}
    
    ~Private()
    {
    }

    void compileClipPath(KoShape *shapeToClip)
    {
        QList<KoPathShape*> clipShapes = clipData->clipPathShapes();
        if(!clipShapes.count())
            return;

        QTransform transformToShape = shapeToClip->absoluteTransformation(0).inverted();
        
        foreach(KoPathShape * path, clipShapes) {
            if(!path)
                continue;
            // map clip path to shape coordinates of clipped shape
            QTransform m = path->absoluteTransformation(0) * transformToShape;
            if (clipPath.isEmpty())
                clipPath = m.map(path->outline());
            else
                clipPath |= m.map(path->outline());
        }
        initialTransformToShape = transformToShape;
    }

    QExplicitlySharedDataPointer<KoClipData> clipData; ///< the clip path data
    QPainterPath clipPath; ///< the compiled clip path in shape coordinates of the clipped shape
    QTransform initialTransformToShape; ///< initial transformation to shape coordinates of the clipped shape
};

KoClipPath::KoClipPath(KoShape *shapeToClip, KoClipData * clipData)
    : d( new Private(clipData) )
{
    d->compileClipPath(shapeToClip);
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

void KoClipPath::applyClipping(KoShape *shape, QPainter & painter, const KoViewConverter &converter)
{
    QPainterPath clipPath;
    while(shape) {
        if (shape->clipPath())
            clipPath |= shape->absoluteTransformation(0).map(shape->clipPath()->path());
        shape = shape->parent();
    }

    if (!clipPath.isEmpty()) {
        QTransform viewMatrix;
        double zoomX, zoomY;
        converter.zoom(&zoomX, &zoomY);
        viewMatrix.scale(zoomX, zoomY);
        painter.setClipPath(viewMatrix.map(clipPath), Qt::IntersectClip);
    }
}

QPainterPath KoClipPath::path() const
{
    return d->clipPath;
}

QList<KoPathShape*> KoClipPath::clipPathShapes() const
{
    return d->clipData->clipPathShapes();
}

QTransform KoClipPath::clipDataTransformation() const
{
    return d->initialTransformToShape;
}
