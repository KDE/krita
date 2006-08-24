/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
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

#ifndef KOPATHTOOL_H
#define KOPATHTOOL_H

#include "KoPathShape.h"

#include <KoTool.h>

class KoPathTool : public KoTool {
public:
    KoPathTool(KoCanvasBase *canvas);
    ~KoPathTool();

    void paint( QPainter &painter, KoViewConverter &converter );

    void mousePressEvent( KoPointerEvent *event ) ;
    void mouseDoubleClickEvent( KoPointerEvent *event );
    void mouseMoveEvent( KoPointerEvent *event );
    void mouseReleaseEvent( KoPointerEvent *event );
    void keyPressEvent(QKeyEvent *event);
    void keyReleaseEvent(QKeyEvent *event);

    void activate (bool temporary=false);
    void deactivate();

private:
    /// repaints the specified rect
    void repaint( const QRectF &repaintRect );
    /// returns a handle rect at the given position
    QRectF handleRect( const QPointF &p );

    /// transform rect form local shape coordinates to world coordinates
    QRectF transformed( const QRectF &r );
    /// transform point form local shape coordinates to world coordinates
    QPointF transformed( const QPointF &p );
    /// transform rect form world coordinates to local shape coordinates
    QRectF untransformed( const QRectF &r );
    /// transform point form world coordinates to local shape coordinates
    QPointF untransformed( const QPointF &p );
    /// snaps given point to grid point
    QPointF snapToGrid( const QPointF &p, Qt::KeyboardModifiers modifiers );
private:
    KoPathShape *m_pathShape;          ///< the actual selected path shape
    KoPathPoint* m_activePoint;        ///< the currently active path point
    int m_handleRadius;                ///< the radius of the control point handles
    KoPathPoint::KoPointType m_activePointType; ///< the type of currently active path point
    bool m_pointMoving;                ///< shows if points are actually moved
    QPointF m_lastPosition;            ///< the last mouse position
    QList<KoPathPoint*> m_selectedPoints; ///< list of selected path points
    QPointF m_move;                    ///< the accumulated point move amount
};

#endif
