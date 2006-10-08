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
#include "KoParameterShape.h"
#include "KoInsets.h"
#include "KoShapeBorderModel.h"
#include "KoPathPointMoveStrategy.h"
#include "KoPathPointRubberSelectStrategy.h"
#include <kdebug.h>
#include <QKeyEvent>

KoPathTool::KoPathTool(KoCanvasBase *canvas)
: KoTool(canvas)
, m_pathShape(0)
, m_activeHandle( this, 0, KoPathPoint::Node )    
, m_handleRadius( 3 )
, m_currentStrategy(0)
{
}

KoPathTool::~KoPathTool() {
}

void KoPathTool::paint( QPainter &painter, KoViewConverter &converter) {
    // TODO using the member m_pathShape is incorrect, use m_canvas to reach the KoSelection object
    // instead and iterator over the selected shapes.
    if( ! m_pathShape )
        return;

    KoSelectionSet selectedShapes = m_canvas->shapeManager()->selection()->selectedShapes( KoFlake::TopLevelSelection );

    painter.save();
    painter.setRenderHint( QPainter::Antialiasing, true );
    painter.setBrush( Qt::blue ); //TODO make configurable
    painter.setPen( Qt::blue );

    foreach( KoShape *shape, selectedShapes ) 
    {
        KoPathShape *pathShape = dynamic_cast<KoPathShape*>( shape );

        if ( !shape->isLocked() && pathShape )
        {
            painter.save();
            painter.setMatrix( shape->transformationMatrix( &converter ) * painter.matrix() );
            KoParameterShape * parameterShape = dynamic_cast<KoParameterShape*>( shape );
            if ( parameterShape )
            {
                painter.setBrush( Qt::red ); //TODO make configureable
                parameterShape->paintHandles( painter, converter );
            }
            else
            {
                pathShape->paintPoints( painter, converter );
            }
            painter.restore();
        }
    }

    painter.restore();

    QPainterPath outline = m_pathShape->outline();
    if(painter.hasClipping()) {
        if ( m_currentStrategy )
        {
            painter.save();
            m_currentStrategy->paint( painter, converter );
            painter.restore();
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

    painter.save();
    painter.setMatrix( m_pathShape->transformationMatrix(&converter) * painter.matrix() );
    double zoomX, zoomY;
    converter.zoom(&zoomX, &zoomY);
    painter.scale(zoomX, zoomY);

    painter.setBrush( Qt::green ); // TODO make color configurable
    painter.setPen( Qt::blue );

    QRectF handle = converter.viewToDocument( handleRect( QPoint(0,0) ) );

    foreach( KoPathPoint *p, m_selectedPoints )
        p->paint( painter, handle.size(), true );

    m_activeHandle.paint( painter, converter );
    painter.restore();
}

void KoPathTool::mousePressEvent( KoPointerEvent *event ) {
    // we are moving if we hit a point and use the left mouse button
    if( m_activeHandle.isActive() )
    {
        if( event->button() & Qt::LeftButton )
        {
            // control select adds/removes points to/from the selection
            if( event->modifiers() & Qt::ControlModifier )
            {
                if( m_selectedPoints.contains( m_activeHandle.m_activePoint ) )
                {
                    // active point is already selected, so deselect it
                    int index = m_selectedPoints.indexOf( m_activeHandle.m_activePoint );
                    m_selectedPoints.removeAt( index );
                }
                else
                    m_selectedPoints << m_activeHandle.m_activePoint;
            }
            else
            {
                // no control modifier, so clear selection and select active point
                if( ! m_selectedPoints.contains( m_activeHandle.m_activePoint ) )
                {
                    m_selectedPoints.clear();
                    m_selectedPoints << m_activeHandle.m_activePoint;
                }
            }
            repaint( m_pathShape->shapeToDocument( m_pathShape->outline().controlPointRect() ) );
            if ( m_selectedPoints.size() > 0 )
            {
                m_currentStrategy = new KoPathPointMoveStrategy( this, m_canvas, event->point );
            }
        }
        else if( event->button() & Qt::RightButton )
        {
            KoPathPoint::KoPointProperties props = m_activeHandle.m_activePoint->properties();
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

            KoPointPropertyCommand *cmd = new KoPointPropertyCommand( m_pathShape, m_activeHandle.m_activePoint, props );
            m_canvas->addCommand( cmd, true );
        }
    }
    else
    {
        if( event->button() & Qt::LeftButton )
        {
            if( event->modifiers() & Qt::ControlModifier == 0 )
                m_selectedPoints.clear();
            // TODO only repaint if selection changed
            repaint( m_pathShape->shapeToDocument( m_pathShape->outline().controlPointRect() ) );
            // starts rubberband selection
            Q_ASSERT(m_currentStrategy == 0);
            m_currentStrategy = new KoPathPointRubberSelectStrategy( this, m_canvas, event->point );
        }
    }
}

void KoPathTool::mouseDoubleClickEvent( KoPointerEvent * ) {
}

void KoPathTool::mouseMoveEvent( KoPointerEvent *event ) {
    if( event->button() & Qt::RightButton )
        return;

    if ( m_currentStrategy )
    {
        m_lastPoint = event->point;
        m_currentStrategy->handleMouseMove( event->point, event->modifiers() );
        return;
    }
    
    QRectF roi = handleRect( m_pathShape->documentToShape( event->point ) );
    QList<KoPathPoint*> points = m_pathShape->pointsAt( roi );
    if( points.empty() )
    {
        useCursor(Qt::ArrowCursor);
        if ( m_activeHandle.m_activePoint )
        {
            m_activeHandle.deactivate();
            repaint( m_pathShape->shapeToDocument( m_pathShape->outline().controlPointRect() ) );
        }
        return;
    }

    useCursor(Qt::SizeAllCursor);

    KoPathPoint *p = points.first();
    KoPathPoint::KoPointType type = KoPathPoint::Node;

    // check for the control points as otherwise it is no longer 
    // possible to change the control points when they are the same as the point
    if( p->properties() & KoPathPoint::HasControlPoint1 && roi.contains( p->controlPoint1() ) )
        type = KoPathPoint::ControlPoint1;
    else if( p->properties() & KoPathPoint::HasControlPoint2 && roi.contains( p->controlPoint2() ) )
        type = KoPathPoint::ControlPoint2;

    m_activeHandle = ActiveHandle( this, p, type );

    repaint( m_pathShape->shapeToDocument( m_pathShape->outline().controlPointRect() ) );
}

void KoPathTool::mouseReleaseEvent( KoPointerEvent *event ) {
    if ( m_currentStrategy )
    {
        m_currentStrategy->finishInteraction( event->modifiers() );
        KCommand *command = m_currentStrategy->createCommand();
        if ( command )
            m_canvas->addCommand( command, false );
        delete m_currentStrategy;
        m_currentStrategy = 0;
    }
}

void KoPathTool::keyPressEvent(QKeyEvent *event) {
    if ( m_currentStrategy )
    {
        switch ( event->key() )
        {
            case Qt::Key_Control:
            case Qt::Key_Alt:
            case Qt::Key_Shift:
            case Qt::Key_Meta:
                m_currentStrategy->handleMouseMove( m_lastPoint, event->modifiers() );
                break;
            case Qt::Key_Escape:
                m_currentStrategy->cancelInteraction();
                delete m_currentStrategy;
                m_currentStrategy = 0;
                break;
            default:
                break;
        }
    }
    else
    {
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
                // TODO finish current action or should this not possible during actions???
                if( m_selectedPoints.size() )
                {
                    KoPointRemoveCommand *cmd = new KoPointRemoveCommand( m_pathShape, m_selectedPoints );
                    if( m_selectedPoints.contains( m_activeHandle.m_activePoint ) )
                    {
                        m_activeHandle.deactivate();
                    }
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
    m_activeHandle.deactivate();
    delete m_currentStrategy;
    m_currentStrategy = 0;
}

void KoPathTool::selectPoints( const QRectF &rect, bool clearSelection )
{
    if( clearSelection )
    {
        m_selectedPoints = m_pathShape->pointsAt( m_pathShape->documentToShape( rect ) );
    }
    else
    {
        QList<KoPathPoint*> selected = m_pathShape->pointsAt( m_pathShape->documentToShape( rect ) );
        foreach( KoPathPoint* point, selected )
            if( ! m_selectedPoints.contains( point ) )
                m_selectedPoints << point;
    }
    repaint( m_pathShape->shapeToDocument( m_pathShape->outline().controlPointRect() ) );
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

void KoPathTool::ActiveHandle::paint( QPainter &painter, KoViewConverter &converter )
{ 
    if ( m_activePoint )
    {
        QRectF handle = converter.viewToDocument( m_tool->handleRect( QPoint(0,0) ) );
        painter.setBrush( Qt::red ); // TODO make color configurable
        painter.setPen( Qt::NoPen );
        m_activePoint->paint( painter, handle.size(), m_tool->m_selectedPoints.contains( m_activePoint ) );
    }
}
