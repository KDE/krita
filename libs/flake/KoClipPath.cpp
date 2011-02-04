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

KoClipData::KoClipData( KoPathShape * clipPathShape )
    : m_deleteClipShapes(true)
{
    Q_ASSERT( clipPathShape );
    m_clipPathShapes.append( clipPathShape );
}

KoClipData::KoClipData( const QList<KoPathShape*> & clipPathShapes )
    : m_deleteClipShapes(true)
{
    Q_ASSERT( clipPathShapes.count() );
    m_clipPathShapes = clipPathShapes;
}

KoClipData::~KoClipData()
{
    if( m_deleteClipShapes )
        qDeleteAll( m_clipPathShapes );
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
    Private( KoClipData * data )
        : clipData( data )
    {}
    ~Private()
    {
    }

    void compileClipPath( const QTransform &shapeMatrix )
    {
        QList<KoPathShape*> clipShapes = clipData->clipPathShapes();
        uint pathCount = clipShapes.count();
        if( ! pathCount )
            return;

        QTransform invShapeMatrix = shapeMatrix.inverted();
        foreach( KoPathShape * path, clipShapes )
        {
            if( ! path )
                continue;
            QTransform m = invShapeMatrix * path->absoluteTransformation(0);
            if( clipPath.isEmpty() )
                clipPath = m.map( path->outline() );
            else
                clipPath = clipPath.united( m.map( path->outline() ) );
        }
    }

    QExplicitlySharedDataPointer<KoClipData> clipData;
    QPainterPath clipPath;
    QTransform shapeMatrix;
};

KoClipPath::KoClipPath( KoClipData * clipData, const QTransform & shapeMatrix )
    : d( new Private(clipData) )
{
    d->compileClipPath( shapeMatrix );
}

KoClipPath::~KoClipPath()
{
    delete d;
}

void KoClipPath::applyClipping( QPainter & painter, const KoViewConverter &converter )
{
    if( ! painter.hasClipping() )
        return;
    if( d->clipPath.isEmpty() )
        return;

    QTransform viewMatrix;
    double zoomX, zoomY;
    converter.zoom(&zoomX, &zoomY);
    viewMatrix.scale(zoomX, zoomY);

    painter.setClipPath( viewMatrix.map( d->clipPath ), Qt::IntersectClip );
}
