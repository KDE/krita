/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#include "KoPathShapeLoader.h"
#include "KoPathShape.h"

class KoPathShapeLoader::Private
{
public:
    Private( KoPathShape * p ) : path( p ) 
    {
        Q_ASSERT( path );
        path->clear();
    }

    KoPathShape * path; ///< the path shape to work on
    QPointF lastPoint;
};

KoPathShapeLoader::KoPathShapeLoader( KoPathShape * path )
    : d( new Private( path ) )
{
}

KoPathShapeLoader::~KoPathShapeLoader()
{
    delete d;
}

void KoPathShapeLoader::svgMoveTo( qreal x1, qreal y1, bool abs )
{
    if( abs )
        d->lastPoint = QPointF( x1, y1 );
    else
        d->lastPoint += QPointF( x1, y1 );
    d->path->moveTo( d->lastPoint );
}

void KoPathShapeLoader::svgLineTo( qreal x1, qreal y1, bool abs )
{
    if( abs )
        d->lastPoint = QPointF( x1, y1 );
    else
        d->lastPoint += QPointF( x1, y1 );

    d->path->lineTo( d->lastPoint );
}

void KoPathShapeLoader::svgLineToHorizontal( qreal x, bool abs )
{
    if( abs )
        d->lastPoint.setX( x );
    else
        d->lastPoint.rx() += x;

    d->path->lineTo( d->lastPoint );
}

void KoPathShapeLoader::svgLineToVertical( qreal y, bool abs )
{
    if( abs )
        d->lastPoint.setY( y );
    else
        d->lastPoint.ry() += y;

    d->path->lineTo( d->lastPoint );
}

void KoPathShapeLoader::svgCurveToCubic( qreal x1, qreal y1, qreal x2, qreal y2, qreal x, qreal y, bool abs )
{
    QPointF p1, p2;
    if( abs )
    {
        p1 = QPointF( x1, y1 );
        p2 = QPointF( x2, y2 );
        d->lastPoint = QPointF( x, y );
    }
    else
    {
        p1 = d->lastPoint + QPointF( x1, y1 );
        p2 = d->lastPoint + QPointF( x2, y2 );
        d->lastPoint += QPointF( x, y );
    }

    d->path->curveTo( p1, p2, d->lastPoint );
}

void KoPathShapeLoader::svgCurveToCubicSmooth( qreal x, qreal y, qreal x2, qreal y2, bool abs )
{
    Q_UNUSED ( x );
    Q_UNUSED ( y );
    Q_UNUSED ( x2 );
    Q_UNUSED ( y2 );
    Q_UNUSED ( abs );
    // TODO implement
}

void KoPathShapeLoader::svgCurveToQuadratic( qreal x, qreal y, qreal x1, qreal y1, bool abs )
{
    Q_UNUSED ( x );
    Q_UNUSED ( y );
    Q_UNUSED ( x1 );
    Q_UNUSED ( y1 );
    Q_UNUSED ( abs );
    // TODO implement
}

void KoPathShapeLoader::svgCurveToQuadraticSmooth( qreal x, qreal y, bool abs )
{
    Q_UNUSED ( x );
    Q_UNUSED ( y );
    Q_UNUSED ( abs );
    // TODO implement
}

void KoPathShapeLoader::svgArcTo( qreal x, qreal y, qreal r1, qreal r2, qreal angle, bool largeArcFlag, bool sweepFlag, bool abs )
{
    Q_UNUSED ( x );
    Q_UNUSED ( y );
    Q_UNUSED ( r1 );
    Q_UNUSED ( r2 );
    Q_UNUSED ( angle );
    Q_UNUSED ( largeArcFlag );
    Q_UNUSED ( sweepFlag );
    Q_UNUSED ( abs );
    // TODO implement
}

void KoPathShapeLoader::svgClosePath()
{
    d->path->close();
}
