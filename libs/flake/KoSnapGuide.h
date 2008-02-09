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

#ifndef KOSNAPGUIDE_H
#define KOSNAPGUIDE_H

#include <QtCore/QPointF>
#include <QtCore/QList>
#include <QtCore/QRectF>
#include <QtGui/QPainterPath>

class KoSnapStrategy;
class KoShape;
class KoPathShape;
class KoPathPoint;
class KoViewConverter;
class KoCanvasBase;
class QPainter;
class KoCanvasBase;

class KoSnapGuide
{
public:

    /// Creates the snap guide to work on the given canvas
    KoSnapGuide( KoCanvasBase * canvas );

    virtual ~KoSnapGuide();

    /// snaps the mouse position, returns if mouse was snapped
    QPointF snap( const QPointF &mousePosition, Qt::KeyboardModifiers modifiers );

    /// paints the guide
    void paint( QPainter &painter, const KoViewConverter &converter );

    /// returns the bounding rect of the guide
    QRectF boundingRect();

    /// Adds an additional shape to snap to (useful when creating a path)
    void setEditedShape( KoShape * shape );

    /// returns the extra shapes to use
    KoShape * editedShape() const;

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

    /// returns the canvas the snap guide is working on
    KoCanvasBase * canvas() const;

    /// Sets a list of path points to ignore
    void setIgnoredPathPoints( const QList<KoPathPoint*> &ignoredPoints );

    /// Returns list of ignored points
    QList<KoPathPoint*> ignoredPathPoints() const;

private:
    KoCanvasBase * m_canvas;
    KoShape * m_editedShape;

    QList<KoSnapStrategy*> m_strategies;
    KoSnapStrategy * m_currentStrategy;

    int m_usedStrategies;
    bool m_active;
    int m_snapDistance;
    QList<KoPathPoint*> m_ignoredPoints;
};

class KoSnapProxy
{
public:
    KoSnapProxy( KoSnapGuide * snapGuide );

    /// returns list of points in given rectangle in document coordinates
    QList<QPointF> pointsInRect( const QRectF &rect );

    /// returns list of shape in given rectangle in document coordinates
    QList<KoShape*> shapesInRect( const QRectF &rect, bool omitEditedShape = false );

    /// returns list of points from given shape
    QList<QPointF> pointsFromShape( KoShape * shape );

    /// returns list of all shapes
    QList<KoShape*> shapes( bool omitEditedShape = false );

private:
    KoSnapGuide * m_snapGuide;
};

#endif // KOSNAPGUIDE_H
