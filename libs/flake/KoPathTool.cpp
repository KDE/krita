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
{
}

KoPathTool::~KoPathTool() {
}

void KoPathTool::paint( QPainter &painter, KoViewConverter &converter) {
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

}

void KoPathTool::mouseDoubleClickEvent( KoPointerEvent *event ) {
}

void KoPathTool::mouseMoveEvent( KoPointerEvent *event ) {
    m_activePoint = 0;

    QWMatrix unwindMatrix = m_pathShape->transformationMatrix(0).inverted();
    QRectF roi = unwindMatrix.mapRect( handleRect( event->point ) );
    QList<KoPathPoint*> points = m_pathShape->pointsAt( roi );
    if( points.empty() )
    {
        useCursor(Qt::ArrowCursor);
        repaint();
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

    repaint();

    if(event->buttons() == Qt::NoButton)
        return;
}

void KoPathTool::mouseReleaseEvent( KoPointerEvent *event ) {
    // TODO
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
            repaint();
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
    repaint();
}

void KoPathTool::deactivate() {
    m_pathShape = 0;
    m_activePoint = 0;
}

void KoPathTool::repaint() {
    QRectF repaintRect = m_pathShape->outline().controlPointRect().translated( m_pathShape->position() );
    m_canvas->updateCanvas( repaintRect.adjusted( -m_handleRadius, -m_handleRadius, m_handleRadius, m_handleRadius ) );
}

QRectF KoPathTool::handleRect( const QPointF &p ) {
    return QRectF( p.x()-m_handleRadius, p.y()-m_handleRadius, 2*m_handleRadius, 2*m_handleRadius );
}
