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
#include "KoPathPointMoveStrategy.h"
#include "KoPathPointRubberSelectStrategy.h"
#include <kdebug.h>
#include <QKeyEvent>

KoPathTool::KoPathTool(KoCanvasBase *canvas)
: KoTool(canvas)
, m_activeHandle( this, 0, KoPathPoint::Node )    
, m_handleRadius( 3 )
, m_pointSelection( this )    
, m_currentStrategy(0)
{
}

KoPathTool::~KoPathTool() {
}

void KoPathTool::paint( QPainter &painter, KoViewConverter &converter) {
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
            pathShape->paintPoints( painter, converter );
            painter.restore();
        }
    }

    painter.restore();

    if ( m_currentStrategy )
    {
        painter.save();
        m_currentStrategy->paint( painter, converter );
        painter.restore();
    }

    m_pointSelection.paint( painter, converter );
    
    if ( m_activeHandle.isActive() )
    {
        painter.save();
        painter.setMatrix( m_activeHandle.m_activePoint->parent()->transformationMatrix(&converter) * painter.matrix() );
        double zoomX, zoomY;
        converter.zoom(&zoomX, &zoomY);
        painter.scale(zoomX, zoomY);
        m_activeHandle.paint( painter, converter );
        painter.restore();
    }
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
                if ( m_pointSelection.contains( m_activeHandle.m_activePoint ) )
                {
                    m_pointSelection.remove( m_activeHandle.m_activePoint );
                }
                else
                {
                    m_pointSelection.add( m_activeHandle.m_activePoint, false ); 
                }
            }
            else
            {
                // no control modifier, so clear selection and select active point
                if ( !m_pointSelection.contains( m_activeHandle.m_activePoint ) )
                {
                    m_pointSelection.add( m_activeHandle.m_activePoint, true ); 
                }
            }
            m_currentStrategy = new KoPathPointMoveStrategy( this, m_canvas, event->point );
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

            KoPointPropertyCommand *cmd = new KoPointPropertyCommand( m_activeHandle.m_activePoint->parent(), m_activeHandle.m_activePoint, props );
            m_canvas->addCommand( cmd, true );
        }
    }
    else
    {
        if( event->button() & Qt::LeftButton )
        {
            if( event->modifiers() & Qt::ControlModifier == 0 )
            {
                m_pointSelection.clear();
            }
            // start rubberband selection
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

    m_pointSelection.repaint();
    if ( m_activeHandle.isActive() )
    {
        repaint( m_activeHandle.m_activePoint->boundingRect() );
    }
    
    if ( m_currentStrategy )
    {
        m_lastPoint = event->point;
        m_currentStrategy->handleMouseMove( event->point, event->modifiers() );
        return;
    }
    
    KoSelectionSet selectedShapes = m_canvas->shapeManager()->selection()->selectedShapes( KoFlake::TopLevelSelection );
    foreach( KoShape *shape, selectedShapes ) 
    {
        KoPathShape *pathShape = dynamic_cast<KoPathShape*>( shape );

        if ( !shape->isLocked() && pathShape )
        {
            QRectF roi = handleRect( pathShape->documentToShape( event->point ) );
            QList<KoPathPoint*> points = pathShape->pointsAt( roi );
            if( ! points.empty() )
            {
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

                repaint( pathShape->shapeToDocument( pathShape->outline().controlPointRect() ) );
                return;
            }
        }
    }

    useCursor(Qt::ArrowCursor);
    if ( m_activeHandle.isActive() )
    {
        m_activeHandle.deactivate();
    }
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
        // TODO remove this constraind 
        if ( m_pointSelection.objectCount() == 1 )
        {
            QList<KoPathPoint*> selectedPoints = m_pointSelection.selectedPoints().toList();
            KoPathShape * pathShape = selectedPoints[0]->parent();
                
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
                    repaint( pathShape->shapeToDocument( pathShape->outline().controlPointRect() ) );
                    break;
                case Qt::Key_Delete:
                    // TODO finish current action or should this not possible during actions???
                    {
                        KoPointRemoveCommand *cmd = new KoPointRemoveCommand( pathShape, selectedPoints );
                        if( m_pointSelection.contains( m_activeHandle.m_activePoint ) )
                        {
                            m_activeHandle.deactivate();
                        }
                        m_pointSelection.clear();
                        m_canvas->addCommand( cmd, true );
                    }
                    break;
                case Qt::Key_Insert:
                    {
                        QList<KoPathSegment> segments;
                        foreach( KoPathPoint* p, selectedPoints )
                        {
                            KoPathPoint *n = pathShape->nextPoint( p );
                            if( m_pointSelection.contains( n ) )
                                segments << qMakePair( p, n );
                        }
                        if( segments.size() )
                        {
                            KoSegmentSplitCommand *cmd = new KoSegmentSplitCommand( pathShape, segments, 0.5 );
                            m_canvas->addCommand( cmd, true );
                        }
                    }
                    break;
                case Qt::Key_D:
                    if ( pathShape )
                    {
                        pathShape->debugPath();
                    }
                    break;
                case Qt::Key_B:
                    {
                        KoSubpathBreakCommand *cmd = 0;
                        if( selectedPoints.size() == 1 )
                            cmd = new KoSubpathBreakCommand( pathShape, selectedPoints.first() );
                        else if( selectedPoints.size() >= 2 )
                            cmd = new KoSubpathBreakCommand( pathShape, KoPathSegment( selectedPoints[0], selectedPoints[1] ) );
                        m_canvas->addCommand( cmd, true );
                    }
                    break;
                case Qt::Key_J:
                    if( selectedPoints.size() == 2 )
                    {
                        KoPointJoinCommand *cmd = new KoPointJoinCommand( pathShape, selectedPoints[0], selectedPoints[1] );
                        m_canvas->addCommand( cmd, true );
                    }
                    break;
                case Qt::Key_F:
                    {
                        QList<KoPathSegment> segments;
                        foreach( KoPathPoint* p, selectedPoints )
                        {
                            KoPathPoint *n = pathShape->nextPoint( p );
                            if( m_pointSelection.contains( n ) )
                                segments << qMakePair( p, n );
                        }
                        if( segments.size() )
                        {
                            KoSegmentTypeCommand *cmd = new KoSegmentTypeCommand( pathShape, segments, true );
                            m_canvas->addCommand( cmd, true );
                        }
                    }
                    break;
                case Qt::Key_C:
                    {
                        QList<KoPathSegment> segments;
                        foreach( KoPathPoint* p, selectedPoints )
                        {
                            KoPathPoint *n = pathShape->nextPoint( p );
                            if( m_pointSelection.contains( n ) )
                                segments << qMakePair( p, n );
                        }
                        if( segments.size() )
                        {
                            KoSegmentTypeCommand *cmd = new KoSegmentTypeCommand( pathShape, segments, false );
                            m_canvas->addCommand( cmd, true );
                        }
                    }
                    break;
            }
        }
    }
    event->accept();
}

void KoPathTool::keyReleaseEvent(QKeyEvent *event) {
    event->accept();
}

void KoPathTool::activate (bool temporary) {
    Q_UNUSED(temporary);
    bool found = false;
    foreach(KoShape *shape, m_canvas->shapeManager()->selection()->selectedShapes()) 
    {
        KoPathShape *pathShape;
        pathShape = dynamic_cast<KoPathShape*> (shape);
        if( pathShape )
        {
            found = true;
            repaint( pathShape->shapeToDocument( pathShape->outline().controlPointRect() ) );
        }
    }
    if( !found ) {
        emit sigDone();
        return;
    }
    useCursor(Qt::ArrowCursor, true);
}

void KoPathTool::deactivate() {
    qDebug() << "KoPathTool::deactivate()";
    m_pointSelection.clear();
    m_activeHandle.deactivate();
    delete m_currentStrategy;
    m_currentStrategy = 0;
}

void KoPathTool::selectPoints( const QRectF &rect, bool clearSelection )
{
    if( clearSelection )
    {
        m_pointSelection.clear();
    }

    KoSelectionSet selectedShapes = m_canvas->shapeManager()->selection()->selectedShapes( KoFlake::TopLevelSelection );
    foreach( KoShape *shape, selectedShapes ) 
    {
        KoPathShape *pathShape = dynamic_cast<KoPathShape*>( shape );

        if ( !shape->isLocked() && pathShape )
        {
            QList<KoPathPoint*> selected = pathShape->pointsAt( pathShape->documentToShape( rect ) );
            foreach( KoPathPoint* point, selected )
            {
                m_pointSelection.add( point, false );
            }
        }
    }
}

void KoPathTool::repaint( const QRectF &repaintRect ) {
    qDebug() << "KoPathTool::repaint(" << repaintRect << ")";
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
        m_activePoint->paint( painter, handle.size(), m_tool->m_pointSelection.contains( m_activePoint ) );
    }
}

void KoPathTool::KoPathPointSelection::paint( QPainter &painter, KoViewConverter &converter )
{
    KoPathShapePointMap::iterator it( m_shapePointMap.begin() );
    for ( ; it != m_shapePointMap.end(); ++it )
    {
        painter.save();

        painter.setMatrix( it.key()->transformationMatrix(&converter) * painter.matrix() );
        double zoomX, zoomY;
        converter.zoom(&zoomX, &zoomY);
        painter.scale(zoomX, zoomY);

        painter.setBrush( Qt::green ); // TODO make color configurable
        painter.setPen( Qt::blue );

        QRectF handle = converter.viewToDocument( m_tool->handleRect( QPoint(0,0) ) );

        foreach( KoPathPoint *p, it.value() )
            p->paint( painter, handle.size(), true );

        painter.restore();
    }
}

void KoPathTool::KoPathPointSelection::add( KoPathPoint * point, bool clear )
{
    bool allreadyIn = false;
    if ( clear )
    {
        if ( size() == 1 && m_selectedPoints.contains( point ) )
        {
            allreadyIn = true;
        }
        else
        {
            this->clear();
        }
    }
    else
    {
        allreadyIn = m_selectedPoints.contains( point );
    }

    if ( !allreadyIn )
    {
        m_selectedPoints.insert( point );
        KoPathShape * pathShape = point->parent();
        KoPathShapePointMap::iterator it( m_shapePointMap.find( pathShape ) );
        if ( it == m_shapePointMap.end() )
        {
            it = m_shapePointMap.insert( pathShape, QSet<KoPathPoint *>() );
        }
        it.value().insert( point );
        m_tool->repaint( point->boundingRect() );
    }
}

void KoPathTool::KoPathPointSelection::remove( KoPathPoint * point )
{
    if ( m_selectedPoints.remove( point ) )
    {
        KoPathShape * pathShape = point->parent();
        m_shapePointMap[pathShape].remove( point );
        if ( m_shapePointMap[pathShape].size() == 0 )
        {
            m_shapePointMap.remove( pathShape );
        }
    }
    m_tool->repaint( point->boundingRect() );
}

void KoPathTool::KoPathPointSelection::clear()
{
    repaint();
    m_selectedPoints.clear();
    m_shapePointMap.clear();
}

void KoPathTool::KoPathPointSelection::repaint()
{
    foreach ( KoPathPoint *p, m_selectedPoints )
    {
        m_tool->repaint( p->boundingRect() );
    }
}
