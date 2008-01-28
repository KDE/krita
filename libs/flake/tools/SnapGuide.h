/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#ifndef SNAPGUIDE_H
#define SNAPGUIDE_H

#include <QtCore/QPointF>
#include <QtCore/QList>
#include <QtCore/QRectF>
#include <QtCore/QPair>
#include <QtGui/QPainterPath>

class KoPathShape;
class KoViewConverter;
class QPainter;

class SnapGuide
{
public:
    SnapGuide( KoPathShape * path );
    virtual ~SnapGuide();

    /// snaps the mouse position, returns if mouse was snapped
    QPointF snap( const QPointF &mousePosition, double maxSnapDistance );

    /// paints the guide
    void paint( QPainter &painter, const KoViewConverter &converter );

    /// returns the bounding rect of the guide
    QRectF boundingRect();

protected:
    // snaps mouse position to given path point
    virtual QPointF snapToPoints( const QPointF &mousePosition, QList<QPointF> &pathPoints, double maxSnapDistance ) = 0;

    double fastDistance( const QPointF &p1, const QPointF &p2 );

    KoPathShape * m_path;
    QPainterPath m_lineGuide;
};

class OrthogonalSnapGuide : public SnapGuide
{
public:
    OrthogonalSnapGuide( KoPathShape * path );
    virtual QPointF snapToPoints( const QPointF &mousePosition, QList<QPointF> &pathPoints, double maxSnapDistance );
};

#endif // SNAPGUIDE_H
