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

#ifndef KOSNAPSTRATEGY_H
#define KOSNAPSTRATEGY_H

#include <QtCore/QPointF>
#include <QtGui/QPainterPath>

class KoPathPoint;
class KoSnapProxy;

class KoSnapStrategy
{
public:
    /// the different possible snap types
    enum SnapType {
        Orthogonal = 1,
        Node = 2,
        Extension = 4,
        Intersection = 8,
        Grid = 16
    };

    KoSnapStrategy( SnapType type );
    virtual ~KoSnapStrategy() {};

    virtual bool snap( const QPointF &mousePosition, KoSnapProxy * proxy, double maxSnapDistance ) = 0;

    /// returns the current snap strategy decoration
    QPainterPath decoration() const;

    /// returns the strategies type
    SnapType type() const;

    static double fastDistance( const QPointF &p1, const QPointF &p2 );

    /// returns the snapped position form the last call to snapToPoints
    QPointF snappedPosition() const;

protected:
    /// sets the current snap strategy decoration
    void setDecoration( const QPainterPath &decoration );

    /// sets the current snapped position
    void setSnappedPosition( const QPointF &position );

private:
    QPainterPath m_decoration;
    SnapType m_snapType;
    QPointF m_snappedPosition;
};

/// snaps to x- or y-coordinates of path points
class OrthogonalSnapStrategy : public KoSnapStrategy
{
public:
    OrthogonalSnapStrategy();
    virtual bool snap( const QPointF &mousePosition, KoSnapProxy * proxy, double maxSnapDistance );
};

/// snaps to path points
class NodeSnapStrategy : public KoSnapStrategy
{
public:
    NodeSnapStrategy();
    virtual bool snap( const QPointF &mousePosition, KoSnapProxy * proxy, double maxSnapDistance );
};

/// snaps extension lines of path shapes
class ExtensionSnapStrategy : public KoSnapStrategy
{
public:
    ExtensionSnapStrategy();
    virtual bool snap( const QPointF &mousePosition, KoSnapProxy * proxy, double maxSnapDistance );
private:
    double project( const QPointF &lineStart , const QPointF &lineEnd, const QPointF &point );
    QPointF extensionDirection( KoPathPoint * point, const QMatrix &matrix );
    bool snapToExtension( QPointF &position, KoPathPoint * point, const QMatrix &matrix );
};

/// snaps to intersections of shapes
class IntersectionSnapStrategy : public KoSnapStrategy
{
public:
    IntersectionSnapStrategy();
    virtual bool snap( const QPointF &mousePosition, KoSnapProxy * proxy, double maxSnapDistance );
};

/// snaps to the canvas grid
class GridSnapStrategy : public KoSnapStrategy
{
public:
    GridSnapStrategy();
    virtual bool snap( const QPointF &mousePosition, KoSnapProxy * proxy, double maxSnapDistance );
};

#endif // KOSNAPSTRATEGY_H
