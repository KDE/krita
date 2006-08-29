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

#include <KoPathTool.h>
#include <KoCanvasBase.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoPointerEvent.h>
#include <KoPathCommand.h>

#include <kdebug.h>
#include <QKeyEvent>

KoPathTool::KoPathTool(KoCanvasBase *canvas)
: KoTool(canvas)
, m_pathShape(0)
, m_activePoint(0)
, m_handleRadius( 3 )
, m_pointMoving( false )
{
}

KoPathTool::~KoPathTool() {
}

void KoPathTool::paint( QPainter &painter, KoViewConverter &converter) {
    if( ! m_pathShape )
        return;
    QPainterPath outline = m_pathShape->outline();
    if(painter.hasClipping()) {
        QRect shape = converter.documentToView( outline.controlPointRect() ).toRect();
        if(painter.clipRegion().intersect( QRegion(shape) ).isEmpty())
            return;
    }

    painter.setMatrix( m_pathShape->transformationMatrix(&converter) * painter.matrix() );
    double zoomX, zoomY;
    converter.zoom(&zoomX, &zoomY);
    painter.scale(zoomX, zoomY);

    painter.setBrush( Qt::blue );
    painter.setPen( Qt::blue );

    QRectF handle = converter.viewToDocument( handleRect( QPoint(0,0) ) );

    foreach( KoPathPoint *p, m_selectedPoints )
        p->paint( painter, handle.size(), true );

    if( m_activePoint )
    {
        painter.setBrush( Qt::red );
        painter.setPen( Qt::NoPen );
        m_activePoint->paint( painter, handle.size(), m_selectedPoints.contains( m_activePoint ) );
    }
}

void KoPathTool::mousePressEvent( KoPointerEvent *event ) {
    m_pointMoving = m_activePoint;
    m_lastPosition = event->point;
    m_move = QPointF( 0, 0 );
    if( m_activePoint && event->button() & Qt::LeftButton)
    {
        m_selectedPoints.clear();
        m_selectedPoints << m_activePoint;
        repaint( transformed( m_pathShape->outline().controlPointRect() ) );
    }
    else if( m_activePoint && event->button() & Qt::RightButton )
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

void KoPathTool::mouseDoubleClickEvent( KoPointerEvent * ) {
}

void KoPathTool::mouseMoveEvent( KoPointerEvent *event ) {
    if( m_pointMoving )
    {
        QPointF docPoint = snapToGrid( event->point, event->modifiers() );
        QPointF move = untransformed( docPoint ) - untransformed( m_lastPosition );
        // as the last position can change when the top left is changed we have 
        // to save it in document pos and not in shape pos
        m_lastPosition = event->point;

        m_move += move;

        KoPointMoveCommand cmd( m_pathShape, m_activePoint, move, m_activePointType );
        cmd.execute();
    }
    else
    {
        m_activePoint = 0;

        QRectF roi = handleRect( untransformed( event->point ) );
        QList<KoPathPoint*> points = m_pathShape->pointsAt( roi );
        if( points.empty() )
        {
            useCursor(Qt::ArrowCursor);
            repaint( transformed( m_pathShape->outline().controlPointRect() ) );
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

        repaint( transformed( m_pathShape->outline().controlPointRect() ) );

        if(event->buttons() == Qt::NoButton)
            return;
    }
}

void KoPathTool::mouseReleaseEvent( KoPointerEvent * ) {
    // TODO
    m_pointMoving = false;
    if( ! m_move.isNull() )
    {
        KoPointMoveCommand *cmd = new KoPointMoveCommand( m_pathShape, m_activePoint, m_move, m_activePointType );
        m_canvas->addCommand( cmd, false );
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
            repaint( transformed( m_pathShape->outline().controlPointRect() ) );
        break;
    }
    event->accept();
}

void KoPathTool::keyReleaseEvent(QKeyEvent *event) {
    event->accept();
}

void KoPathTool::activate (bool temporary) {
    Q_UNUSED(temporary);
    KoShape *shape = m_canvas->shapeManager()->selection()->firstSelectedShape();
    m_pathShape = dynamic_cast<KoPathShape*> (shape);
    if(m_pathShape == 0) {
        emit sigDone();
        return;
    }
    useCursor(Qt::ArrowCursor, true);
    repaint( transformed( m_pathShape->outline().controlPointRect() ) );
}

void KoPathTool::deactivate() {
    QRectF repaintRect = transformed( m_pathShape->outline().controlPointRect() );
    m_selectedPoints.clear();
    m_pathShape = 0;
    m_activePoint = 0;
    repaint( repaintRect );
}

void KoPathTool::repaint( const QRectF &repaintRect ) {
    m_canvas->updateCanvas( repaintRect.adjusted( -m_handleRadius, -m_handleRadius, m_handleRadius, m_handleRadius ) );
}

QRectF KoPathTool::handleRect( const QPointF &p ) {
    return QRectF( p.x()-m_handleRadius, p.y()-m_handleRadius, 2*m_handleRadius, 2*m_handleRadius );
}

QRectF KoPathTool::transformed( const QRectF &r ) {
    return m_pathShape->transformationMatrix(0).mapRect( r );
}

QPointF KoPathTool::transformed( const QPointF &p ) {
    return m_pathShape->transformationMatrix(0).map( p );
}

QRectF KoPathTool::untransformed( const QRectF &r ) {
    return m_pathShape->transformationMatrix(0).inverted().mapRect( r );
}

QPointF KoPathTool::untransformed( const QPointF &p ) {
    return m_pathShape->transformationMatrix(0).inverted().map( p );
}

QPointF KoPathTool::snapToGrid( const QPointF &p, Qt::KeyboardModifiers modifiers ) {
    if( ! m_canvas->snapToGrid() || modifiers & Qt::ShiftModifier )
        return p;

    double gridX, gridY;
    m_canvas->gridSize( &gridX, &gridY );
    return QPointF( static_cast<int>( p.x() / gridX + 1e-10 ) * gridX,
                    static_cast<int>( p.y() / gridY + 1e-10 ) * gridY );
}
