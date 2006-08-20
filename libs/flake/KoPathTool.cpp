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

#include <KoPathTool.h>
#include <KoCanvasBase.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoPointerEvent.h>

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
    int mcount = outline.elementCount();

    QRectF handle = converter.viewToDocument( handleRect( QPoint(0,0) ) );

    for( int i = 0; i < mcount; i++)
    {
        QPainterPath::Element elem = outline.elementAt( i );
        painter.drawEllipse( handle.translated( elem.x, elem.y ) );
        if(elem.type == QPainterPath::CurveToElement) 
        {
            painter.drawLine( QPointF(outline.elementAt( i - 1 ).x, outline.elementAt( i - 1 ).y),
                    QPointF(outline.elementAt( i ).x, outline.elementAt( i ).y ));
            painter.drawLine( QPointF(outline.elementAt( i + 1 ).x, outline.elementAt( i + 1 ).y),
                    QPointF(outline.elementAt( i + 2 ).x, outline.elementAt( i + 2 ).y ));
        }
    }

    if( m_activePoint )
    {
        painter.setBrush( Qt::red );
        if( m_activePointType == Normal )
        {
            painter.drawEllipse( handle.translated( m_activePoint->point() ) );
            if( m_activePoint->properties() & KoPathPoint::HasControlPoint1 )
                painter.drawEllipse( handle.translated( m_activePoint->controlPoint1() ) );
            if( m_activePoint->properties() & KoPathPoint::HasControlPoint2 )
                painter.drawEllipse( handle.translated( m_activePoint->controlPoint2() ) );
        }
        else if( m_activePointType == ControlPoint1 )
            painter.drawEllipse( handle.translated( m_activePoint->controlPoint1() ) );
        else if( m_activePointType == ControlPoint2 )
            painter.drawEllipse( handle.translated( m_activePoint->controlPoint2() ) );
    }
}

void KoPathTool::mousePressEvent( KoPointerEvent *event ) {
    m_pointMoving = m_activePoint;
    m_lastPosition = untransformed( event->point );
}

void KoPathTool::mouseDoubleClickEvent( KoPointerEvent *event ) {
}

void KoPathTool::mouseMoveEvent( KoPointerEvent *event ) {
    if( m_pointMoving )
    {
        QRectF oldControlRect = m_pathShape->outline().controlPointRect();
        QPointF move = untransformed( event->point ) - m_lastPosition;
        m_lastPosition = untransformed( event->point );

        if( m_activePointType == Normal )
        {
            m_activePoint->setPoint( m_activePoint->point() + move );
            if( m_activePoint->properties() & KoPathPoint::HasControlPoint1 )
                m_activePoint->setControlPoint1( m_activePoint->controlPoint1() + move );
            if( m_activePoint->properties() & KoPathPoint::HasControlPoint2 )
                m_activePoint->setControlPoint2( m_activePoint->controlPoint2() + move );
        }
        else if( m_activePointType == ControlPoint1 )
            m_activePoint->setControlPoint1( m_activePoint->controlPoint1() + move );
        else if( m_activePointType == ControlPoint2 )
            m_activePoint->setControlPoint2( m_activePoint->controlPoint2() + move );

        // the bounding rect has changed -> normalize and adjust position of shape 
        QPointF tlOld = m_pathShape->boundingRect().topLeft();
        m_pathShape->normalize();
        QPointF tlNew = m_pathShape->boundingRect().topLeft();
        m_pathShape->moveBy( tlOld.x()-tlNew.x(), tlOld.y()-tlNew.y() );

        repaint( transformed( oldControlRect.unite( m_pathShape->outline().controlPointRect() ) ) );
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
        if( roi.contains( m_activePoint->point() ) )
            m_activePointType = Normal;
        else if( m_activePoint->properties() & KoPathPoint::HasControlPoint1 && roi.contains( m_activePoint->controlPoint1() ) )
            m_activePointType = ControlPoint1;
        else if( m_activePoint->properties() & KoPathPoint::HasControlPoint2 && roi.contains( m_activePoint->controlPoint2() ) )
            m_activePointType = ControlPoint2;

        repaint( transformed( m_pathShape->outline().controlPointRect() ) );

        if(event->buttons() == Qt::NoButton)
            return;
    }
}

void KoPathTool::mouseReleaseEvent( KoPointerEvent *event ) {
    // TODO
    m_pointMoving = false;
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
