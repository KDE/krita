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
class KoCanvasBase;
class QPainter;
class SnapStrategy;
class KoCanvasBase;

class SnapGuide
{
public:
    /// the different possible snap types
    enum SnapType {
        Orthogonal = 1,
        Node = 2
    };

    /// Creates the snap guide to work on the given canvas
    SnapGuide( KoCanvasBase * canvas );

    virtual ~SnapGuide();

    /// snaps the mouse position, returns if mouse was snapped
    QPointF snap( const QPointF &mousePosition );

    /// paints the guide
    void paint( QPainter &painter, const KoViewConverter &converter );

    /// returns the bounding rect of the guide
    QRectF boundingRect();

    /// Sets an additional path shape to snap to (useful when creating a path)
    void setPathShape( KoPathShape * path );

    /// enables the strategies used for snapping 
    void enableSnapStrategies( int strategies );

    /// returns the enabled snap strategies
    int enabledSnapStrategies() const;

    /// enables the snapping guides
    void enableSnapping( bool on );

    /// returns if snapping is enabled
    bool isSnapping() const;

    /// sets the snap distances in pixels
    void setSnapDistance( int distance );

    /// returns the snap distance in pixels
    int snapDistance() const;

protected:
    KoCanvasBase * m_canvas;
    KoPathShape * m_path;

    QList<SnapStrategy*> m_strategies;
    SnapStrategy * m_currentStrategy;

    int m_usedStrategies;
    bool m_active;
    int m_snapDistance;
};

class SnapStrategy
{
public:
    SnapStrategy( SnapGuide::SnapType type );
    virtual ~SnapStrategy() {};

    virtual bool snapToPoints( const QPointF &mousePosition, QList<QPointF> &pathPoints, double maxSnapDistance ) = 0;

    /// returns the current snap strategy decoration
    QPainterPath decoration() const;

    /// returns the strategies type
    SnapGuide::SnapType type() const;

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
    SnapGuide::SnapType m_snapType;
    QPointF m_snappedPosition;
};

class OrthogonalSnapStrategy : public SnapStrategy
{
public:
    OrthogonalSnapStrategy();
    virtual bool snapToPoints( const QPointF &mousePosition, QList<QPointF> &pathPoints, double maxSnapDistance );
};

class NodeSnapStrategy : public SnapStrategy
{
public:
    NodeSnapStrategy();
    virtual bool snapToPoints( const QPointF &mousePosition, QList<QPointF> &pathPoints, double maxSnapDistance );
};

#endif // SNAPGUIDE_H
