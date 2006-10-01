/* This file is part of the KDE project
 * Copyright (C) 2006 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoPathTool.h"
#include "KoCanvasBase.h"
#include "KoSelection.h"
#include "KoShapeManager.h"
#include "KoPointerEvent.h"
#include "KoPathCommand.h"
#include "KoInsets.h"
#include "KoShapeBorderModel.h"
#include <kdebug.h>
#include <QKeyEvent>

KoPathTool::KoPathTool(KoCanvasBase *canvas)
: KoTool(canvas)
, m_pathShape(0)
, m_activePoint(0)
, m_handleRadius( 3 )
, m_pointMoving( false )
, m_rubberSelect(0)
{
}

KoPathTool::~KoPathTool() {
}

void KoPathTool::paint( QPainter &painter, KoViewConverter &converter) {
    // TODO using the member m_pathShape is incorrect, use m_canvas to reach the KoSelection object
    // instead and iterator over the selected shapes.
    if( ! m_pathShape )
        return;
    QPainterPath outline = m_pathShape->outline();
    if(painter.hasClipping()) {
        if( m_rubberSelect )
        {
            QRect rubberBand = converter.documentToView( m_rubberSelect->selectionRect() ).toRect();
            if( ! painter.clipRegion().intersect( rubberBand ).isEmpty() )
                m_rubberSelect->paint( painter, converter );
        }

        QRectF controlPointRect = outline.controlPointRect();
        if( m_pathShape->border() )
        {
            KoInsets insets;
            m_pathShape->border()->borderInsets( m_pathShape, insets );
            controlPointRect.adjust( -insets.left, -insets.top, insets.right, insets.bottom );
        }
        QRect shape = converter.documentToView( m_pathShape->shapeToDocument( controlPointRect ) ).toRect();
        if(painter.clipRegion().intersect( QRegion(shape) ).isEmpty())
            return;
    }

    painter.setMatrix( m_pathShape->transformationMatrix(&converter) * painter.matrix() );
    double zoomX, zoomY;
    converter.zoom(&zoomX, &zoomY);
    painter.scale(zoomX, zoomY);

    painter.setBrush( Qt::blue ); // TODO make color configurable
    painter.setPen( Qt::blue );

    QRectF handle = converter.viewToDocument( handleRect( QPoint(0,0) ) );

    foreach( KoPathPoint *p, m_selectedPoints )
        p->paint( painter, handle.size(), true );

    if( m_activePoint )
    {
        painter.setBrush( Qt::red ); // TODO make color configurable
        painter.setPen( Qt::NoPen );
        m_activePoint->paint( painter, handle.size(), m_selectedPoints.contains( m_activePoint ) );
    }
}

void KoPathTool::mousePressEvent( KoPointerEvent *event ) {
    // we are moving if we hit a point and use the left mouse button
    m_pointMoving = m_activePoint && event->button() & Qt::LeftButton;
    m_lastPosition = event->point;
    m_move = QPointF( 0, 0 );
    if( m_activePoint )
    {
        if( event->button() & Qt::LeftButton )
        {
            // control select adds/removes points to/from the selection
            if( event->modifiers() & Qt::ControlModifier )
            {
                if( m_selectedPoints.contains( m_activePoint ) )
                {
                    // active point is already selected, so deselect it
                    int index = m_selectedPoints.indexOf( m_activePoint );
                    m_selectedPoints.removeAt( index );
                }
                else
                    m_selectedPoints << m_activePoint;
            }
            else
            {
                // no control modifier, so clear selection and select active point
                if( ! m_selectedPoints.contains( m_activePoint ) )
                {
                    m_selectedPoints.clear();
                    m_selectedPoints << m_activePoint;
                }
            }
            repaint( m_pathShape->shapeToDocument( m_pathShape->outline().controlPointRect() ) );
        }
        else if( event->button() & Qt::RightButton )
        {
            KoPathPoint::KoPointProperties props = m_activePoint->properties();
            if( (props & KoPathPoint::HasControlPoint1) == 0 || (props & KoPathPoint::HasControlPoint2) == 0 )
                return;

            // cycle the smooth->symmetric->unsmooth state of the path point
            if( props & KoPathPoint::IsSmooth )
            {
                props &= ~KoPathPoint::IsSmooth;
                props |= KoPathPoint::IsSymmetric;
            }
            else if( props & KoPathPoint::IsSymmetric )
                props &= ~KoPathPoint::IsSymmetric;
            else
                props |= KoPathPoint::IsSmooth;

            KoPointPropertyCommand *cmd = new KoPointPropertyCommand( m_pathShape, m_activePoint, props );
            m_canvas->addCommand( cmd, true );
        }
    }
    else
    {
        if( event->button() & Qt::LeftButton )
        {
            if( event->modifiers() & Qt::ControlModifier == 0 )
                m_selectedPoints.clear();
            repaint( m_pathShape->shapeToDocument( m_pathShape->outline().controlPointRect() ) );
            // starts rubberband selection
            Q_ASSERT(m_rubberSelect == 0);
            m_rubberSelect = new KoPointRubberSelectStrategy( this, m_canvas, event->point );
        }
    }
}

void KoPathTool::mouseDoubleClickEvent( KoPointerEvent * ) {
}

void KoPathTool::mouseMoveEvent( KoPointerEvent *event ) {
    if( event->button() & Qt::RightButton )
        return;

    if( m_pointMoving )
    {
        QPointF docPoint = snapToGrid( event->point, event->modifiers() );
        QPointF move = m_pathShape->documentToShape( docPoint ) - m_pathShape->documentToShape( m_lastPosition );
        // as the last position can change when the top left is changed we have 
        // to save it in document pos and not in shape pos
        m_lastPosition = docPoint;

        m_move += move;

        // only multiple nodes can be moved at once
        if( m_activePointType == KoPathPoint::Node )
        {
            KoPointMoveCommand cmd( m_pathShape, m_selectedPoints, move );
            cmd.execute();
        }
        else
        {
            KoPointMoveCommand cmd( m_pathShape, m_activePoint, move, m_activePointType );
            cmd.execute();
        }
    }
    else if( m_rubberSelect )
    {
        m_rubberSelect->handleMouseMove( event->point, event->modifiers() );
    }
    else
    {
        m_activePoint = 0;

        QRectF roi = handleRect( m_pathShape->documentToShape( event->point ) );
        QList<KoPathPoint*> points = m_pathShape->pointsAt( roi );
        if( points.empty() )
        {
            useCursor(Qt::ArrowCursor);
            repaint( m_pathShape->shapeToDocument( m_pathShape->outline().controlPointRect() ) );
            return;
        }

        useCursor(Qt::SizeAllCursor);

        m_activePoint = points.first();
        // check first for the control points as otherwise it is no longer 
        // possible to change the control points when they are the same as the point
        if( m_activePoint->properties() & KoPathPoint::HasControlPoint1 && roi.contains( m_activePoint->controlPoint1() ) )
            m_activePointType = KoPathPoint::ControlPoint1;
        else if( m_activePoint->properties() & KoPathPoint::HasControlPoint2 && roi.contains( m_activePoint->controlPoint2() ) )
            m_activePointType = KoPathPoint::ControlPoint2;
        else if( roi.contains( m_activePoint->point() ) )
            m_activePointType = KoPathPoint::Node;

        repaint( m_pathShape->shapeToDocument( m_pathShape->outline().controlPointRect() ) );

        if(event->buttons() == Qt::NoButton)
            return;
    }
}

void KoPathTool::mouseReleaseEvent( KoPointerEvent *event ) {
    if( m_pointMoving )
    {
        if( m_move.isNull() )
        {
            // it was just a click without moving the mouse
            if( event->modifiers() & Qt::ControlModifier == 0 )
            {
                // no control modifier pressed, so clear the selection
                m_selectedPoints.clear();
                // readd the active point
                if( m_activePoint )
                    m_selectedPoints << m_activePoint;
                repaint( m_pathShape->shapeToDocument( m_pathShape->outline().controlPointRect() ) );
            }
        }
        else
        {
            KoPointMoveCommand *cmd = 0;
            // only multiple nodes can be moved at once
            if( m_activePointType == KoPathPoint::Node )
                cmd = new KoPointMoveCommand( m_pathShape, m_selectedPoints, m_move );
            else
                cmd = new KoPointMoveCommand( m_pathShape, m_activePoint, m_move, m_activePointType );
            m_canvas->addCommand( cmd, false );
        }
        m_pointMoving = false;
    }
    else if( m_rubberSelect )
    {
        m_rubberSelect->finishInteraction();
        QRectF selectRect = m_rubberSelect->selectionRect();
        if( event->modifiers() & Qt::ControlModifier )
        {
            QList<KoPathPoint*> selected = m_pathShape->pointsAt( m_pathShape->documentToShape( selectRect ) );
            foreach( KoPathPoint* point, selected )
                if( ! m_selectedPoints.contains( point ) )
                    m_selectedPoints << point;
        }
        else
            m_selectedPoints = m_pathShape->pointsAt( m_pathShape->documentToShape( selectRect ) );

        delete m_rubberSelect;
        m_rubberSelect = 0;
        repaint( selectRect.united( m_pathShape->shapeToDocument( m_pathShape->outline().controlPointRect() ) ) );
    }
}

void KoPathTool::keyPressEvent(QKeyEvent *event) {
    switch(event->key()) 
    {
        case Qt::Key_I:
            if(event->modifiers() & Qt::ControlModifier) 
            {
                if( m_handleRadius > 3 )
                    m_handleRadius--;
            }
            else
                m_handleRadius++;
            repaint( m_pathShape->shapeToDocument( m_pathShape->outline().controlPointRect() ) );
        break;
        case Qt::Key_Delete:
            m_pointMoving = false;
            if( m_selectedPoints.size() )
            {
                KoPointRemoveCommand *cmd = new KoPointRemoveCommand( m_pathShape, m_selectedPoints );
                if( m_selectedPoints.contains( m_activePoint ) )
                    m_activePoint = 0;
                m_selectedPoints.clear();
                m_canvas->addCommand( cmd, true );
            }
        break;
        case Qt::Key_Insert:
            if( m_selectedPoints.size() )
            {
                QList<KoPathSegment> segments;
                foreach( KoPathPoint* p, m_selectedPoints )
                {
                    KoPathPoint *n = m_pathShape->nextPoint( p );
                    if( m_selectedPoints.contains( n ) )
                        segments << qMakePair( p, n );
                }
                if( segments.size() )
                {
                    KoSegmentSplitCommand *cmd = new KoSegmentSplitCommand( m_pathShape, segments, 0.5 );
                    m_canvas->addCommand( cmd, true );
                }
            }
        break;
        case Qt::Key_D:
            if ( m_pathShape )
            {
                m_pathShape->debugPath();
            }
        break;
        case Qt::Key_B:
        {
            KoSubpathBreakCommand *cmd = 0;
            if( m_selectedPoints.size() == 1 )
                cmd = new KoSubpathBreakCommand( m_pathShape, m_selectedPoints.first() );
            else if( m_selectedPoints.size() >= 2 )
                cmd = new KoSubpathBreakCommand( m_pathShape, KoPathSegment( m_selectedPoints[0], m_selectedPoints[1] ) );
            m_canvas->addCommand( cmd, true );
        }
        break;
        case Qt::Key_J:
            if( m_selectedPoints.size() >= 2 )
            {
                KoPointJoinCommand *cmd = new KoPointJoinCommand( m_pathShape, m_selectedPoints[0], m_selectedPoints[1] );
                m_canvas->addCommand( cmd, true );
            }
        break;
        case Qt::Key_F:
            if( m_selectedPoints.size() )
            {
                QList<KoPathSegment> segments;
                foreach( KoPathPoint* p, m_selectedPoints )
                {
                    KoPathPoint *n = m_pathShape->nextPoint( p );
                    if( m_selectedPoints.contains( n ) )
                        segments << qMakePair( p, n );
                }
                if( segments.size() )
                {
                    KoSegmentTypeCommand *cmd = new KoSegmentTypeCommand( m_pathShape, segments, true );
                    m_canvas->addCommand( cmd, true );
                }
            }
        break;
        case Qt::Key_C:
            if( m_selectedPoints.size() )
            {
                QList<KoPathSegment> segments;
                foreach( KoPathPoint* p, m_selectedPoints )
                {
                    KoPathPoint *n = m_pathShape->nextPoint( p );
                    if( m_selectedPoints.contains( n ) )
                        segments << qMakePair( p, n );
                }
                if( segments.size() )
                {
                    KoSegmentTypeCommand *cmd = new KoSegmentTypeCommand( m_pathShape, segments, false );
                    m_canvas->addCommand( cmd, true );
                }
            }
        break;
    }
    event->accept();
}

void KoPathTool::keyReleaseEvent(QKeyEvent *event) {
    event->accept();
}

void KoPathTool::activate (bool temporary) {
    Q_UNUSED(temporary);
    foreach(KoShape *shape, m_canvas->shapeManager()->selection()->selectedShapes()) {
        m_pathShape = dynamic_cast<KoPathShape*> (shape);
        if(m_pathShape)
            break;
    }
    if(m_pathShape == 0) {
        emit sigDone();
        return;
    }
    useCursor(Qt::ArrowCursor, true);
    repaint( m_pathShape->shapeToDocument( m_pathShape->outline().controlPointRect() ) );
}

void KoPathTool::deactivate() {
    if(m_pathShape) {
        QRectF repaintRect = m_pathShape->shapeToDocument( m_pathShape->outline().controlPointRect() );
        m_selectedPoints.clear();
        repaint( repaintRect );
    }
    m_pathShape = 0;
    m_activePoint = 0;
    delete m_rubberSelect;
    m_rubberSelect = 0;
}

void KoPathTool::repaint( const QRectF &repaintRect ) {
    m_canvas->updateCanvas( repaintRect.adjusted( -m_handleRadius, -m_handleRadius, m_handleRadius, m_handleRadius ) );
}

QRectF KoPathTool::handleRect( const QPointF &p ) {
    return QRectF( p.x()-m_handleRadius, p.y()-m_handleRadius, 2*m_handleRadius, 2*m_handleRadius );
}

QPointF KoPathTool::snapToGrid( const QPointF &p, Qt::KeyboardModifiers modifiers ) {
    if( ! m_canvas->snapToGrid() || modifiers & Qt::ShiftModifier )
        return p;

    double gridX, gridY;
    m_canvas->gridSize( &gridX, &gridY );
    return QPointF( static_cast<int>( p.x() / gridX + 1e-10 ) * gridX,
                    static_cast<int>( p.y() / gridY + 1e-10 ) * gridY );
}
