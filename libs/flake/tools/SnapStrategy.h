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

#ifndef SNAPSTRATEGY_H
#define SNAPSTRATEGY_H

#include "SnapGuide.h"

#include <QtCore/QPointF>
#include <QtGui/QPainterPath>

class KoPathPoint;

class SnapStrategy
{
public:
    /// the different possible snap types
    enum SnapType {
        Orthogonal = 1,
        Node = 2,
        Extension = 4,
        Intersection = 8
    };

    SnapStrategy( SnapType type );
    virtual ~SnapStrategy() {};

    virtual bool snapToPoints( const QPointF &mousePosition, SnapProxy * proxy, double maxSnapDistance ) = 0;

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

class OrthogonalSnapStrategy : public SnapStrategy
{
public:
    OrthogonalSnapStrategy();
    virtual bool snapToPoints( const QPointF &mousePosition, SnapProxy * proxy, double maxSnapDistance );
};

class NodeSnapStrategy : public SnapStrategy
{
public:
    NodeSnapStrategy();
    virtual bool snapToPoints( const QPointF &mousePosition, SnapProxy * proxy, double maxSnapDistance );
};

class ExtensionSnapStrategy : public SnapStrategy
{
public:
    ExtensionSnapStrategy();
    virtual bool snapToPoints( const QPointF &mousePosition, SnapProxy * proxy, double maxSnapDistance );
private:
    double project( const QPointF &lineStart , const QPointF &lineEnd, const QPointF &point );
    QPointF extensionDirection( KoPathPoint * point, const QMatrix &matrix );
    bool snapToExtension( QPointF &position, KoPathPoint * point, const QMatrix &matrix );
};

class IntersectionSnapStrategy : public SnapStrategy
{
public:
    IntersectionSnapStrategy();
    virtual bool snapToPoints( const QPointF &mousePosition, SnapProxy * proxy, double maxSnapDistance );
};
#endif // SNAPSTRATEGY_H
