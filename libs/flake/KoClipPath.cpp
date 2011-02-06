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

KoClipData::KoClipData(KoPathShape * clipPathShape)
    : m_deleteClipShapes(true)
{
    Q_ASSERT(clipPathShape);
    m_clipPathShapes.append(clipPathShape);
}

KoClipData::KoClipData(const QList<KoPathShape*> & clipPathShapes)
    : m_deleteClipShapes(true)
{
    Q_ASSERT(clipPathShapes.count());
    m_clipPathShapes = clipPathShapes;
}

KoClipData::~KoClipData()
{
    if(m_deleteClipShapes)
        qDeleteAll(m_clipPathShapes);
}

QList<KoPathShape*> KoClipData::clipPathShapes() const
{
    return m_clipPathShapes;
}

void KoClipData::removeClipShapesOwnership()
{
    m_deleteClipShapes = false;
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

    void compileClipPath(const QTransform &transformToShape)
    {
        QList<KoPathShape*> clipShapes = clipData->clipPathShapes();
        if(!clipShapes.count())
            return;

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
    }

    QExplicitlySharedDataPointer<KoClipData> clipData;
    QPainterPath clipPath;
};

KoClipPath::KoClipPath(KoClipData * clipData, const QTransform & transformToShape)
    : d( new Private(clipData) )
{
    d->compileClipPath(transformToShape);
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
