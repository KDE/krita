/* This file is part of the KDE project
 *
 * Copyright (C) 2009 Jean-Nicolas Artaud <jeannicolasartaud@gmail.com>
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
 
#include "KoConnectionTool.h"

#include <KoCanvasBase.h>
#include <KoPointerEvent.h>
#include <KoShapeManager.h>
#include <KoShapeFactory.h>
#include <KoShape.h>
#include <KDebug>
#include <KoShapeController.h>
#include <KoShapeLayer.h>
#include <KoShapeRegistry.h>
#include <KoSelection.h>
#include <KoLineBorder.h>

#include <QUndoCommand>
#include <QPointF>

KoConnectionTool::KoConnectionTool( KoCanvasBase * canvas )
    : KoPathTool(canvas)
    , m_firstShape(0)
    , m_shapeOn(0)
    , m_pointSelected(0)
{
}

KoConnectionTool::~KoConnectionTool()
{
}

void KoConnectionTool::paint( QPainter &painter, const KoViewConverter &converter)
{
    float x = 0;
    float y = 0;
    painter.setRenderHint(QPainter::Antialiasing, true);
    if( m_shapeOn !=  0)
    {
        painter.save();
        
        KoShape::applyConversion(painter, converter);
        //m_shapeOn->applyConversion(painter, converter);    
        painter.setMatrix(m_shapeOn->absoluteTransformation(&converter) * painter.matrix());
        foreach( QPointF point, m_shapeOn->connectionPoints() )
        {
            x = point.x() - 3;// + m_shapeOn->position().x();
            y = point.y() - 3;// + m_shapeOn->position().y();
            // x = m_shapeOn->absoluteTransformation(&converter).map( point ).x();
            // y = m_shapeOn->absoluteTransformation(&converter).map( point ).y();
            if( distanceSquare(m_mouse, QPointF( x + 3, y + 3 )) > 225 )
                painter.fillRect( x, y, 6, 6, QColor(Qt::green) );
            else
                painter.fillRect( x, y, 6, 6, QColor(Qt::blue) );
                
        }
        painter.restore();
    }    
}

void KoConnectionTool::getPointSelected(KoShape * shape)
{
    float x,y;
    foreach( QPointF point, shape->connectionPoints() )
    {
        x = point.x() + m_shapeOn->position().x();
        y = point.y() + m_shapeOn->position().y();
        if( distanceSquare(m_mouse, QPointF( x, y )) <= 225 )
        {
            m_pointSelected = new QPointF( x, y );
        }
    }
}

void KoConnectionTool::mousePressEvent( KoPointerEvent *event )
{
    KoShape * tempShape = m_canvas->shapeManager()->shapeAt( event->point );
    if( dynamic_cast<KoSelection*>( tempShape ) )
        tempShape = 0;
    if( tempShape != 0 )
    { // If the shape selected is not the background
        if( m_firstShape == 0 )
        { // If any connections is beginning
            m_firstShape = tempShape;
            getPointSelected( m_firstShape );
        }
        else if( m_firstShape != tempShape )
        { //If a connection is on

            // All sizes and positions are hardcoded for now
            KoShapeFactory *factory = KoShapeRegistry::instance()->value("KoConnectionShape");
            KoShape *shape = factory->createDefaultShapeAndInit( m_canvas->shapeController()->dataCenterMap() );
            KoConnectionShape * connectionShape = dynamic_cast<KoConnectionShape*>( shape );
            
            KoConnection myConnection1;
            if( m_pointSelected != 0 )
            {
                int index, i = 0;
                QList<QPointF> connectionPoints1 = m_firstShape->connectionPoints();
                for( i = 0; i < connectionPoints1.count(); i++)
                {
                    QPointF point = connectionPoints1[i] + m_firstShape->position();
                    if( approx(m_pointSelected->x(), point.x()) and approx(m_pointSelected->y(), point.y()) )
                       index = i;
                }
                
                myConnection1 = KoConnection( m_firstShape, index );
                m_pointSelected = 0;
            }
            else
                myConnection1 = getConnection( m_firstShape, tempShape );
            
            connectionShape->setConnection1( myConnection1.first, myConnection1.second );
            // Le second point
            getPointSelected( tempShape );
            KoConnection myConnection2;
            if( m_pointSelected != 0 )
            {
                int index, i = 0;
                QList<QPointF> connectionPoints1 = tempShape->connectionPoints();
                for( i = 0; i < connectionPoints1.count(); i++)
                {
                    QPointF point = connectionPoints1[i] + tempShape->position();
                    if( approx(m_pointSelected->x(), point.x()) and approx(m_pointSelected->y(), point.y()) )
                       index = i;
                }
                
                myConnection2 = KoConnection( tempShape, index );
                m_pointSelected = 0;
            }
            else
                myConnection2 = getConnection( tempShape, m_firstShape );
            connectionShape->setConnection2( myConnection2.first, myConnection2.second );
            
            QUndoCommand * cmd;
            if( connectionShape != 0 )
                cmd = m_canvas->shapeController()->addShape( connectionShape );
        
            if (cmd) {
                m_canvas->addCommand(cmd);
            } else {
                m_canvas->updateCanvas(connectionShape->boundingRect());
                delete connectionShape;
            }
        
            connectionShape->updateConnections();
            m_firstShape = 0;
            m_pointSelected = 0;
        }
    }
}

void KoConnectionTool::mouseMoveEvent( KoPointerEvent *event )
{
    m_shapeOn = m_canvas->shapeManager()->shapeAt( event->point );
    if( dynamic_cast<KoSelection*>( m_shapeOn ) )
        m_shapeOn = 0;
    m_mouse = QPointF( event->point );
    m_canvas->updateCanvas(QRectF( 0, 0, m_canvas->canvasWidget()->width(), m_canvas->canvasWidget()->height() ));
}

void KoConnectionTool::mouseReleaseEvent( KoPointerEvent *event )
{
}

void KoConnectionTool::activate( bool temporary )
{
    m_canvas->canvasWidget()->setCursor( Qt::PointingHandCursor );
}

void KoConnectionTool::deactivate()
{
    m_firstShape = 0;
    m_pointSelected = 0;
    repaint(QRectF( 0, 0, m_canvas->canvasWidget()->x(), m_canvas->canvasWidget()->y() ));
    m_canvas->updateCanvas(QRectF( 0, 0, m_canvas->canvasWidget()->x(), m_canvas->canvasWidget()->y() ));
}

KoConnection KoConnectionTool::getConnection( KoShape * shape1, KoShape * shape2 )
{
    float MAX_DISTANCE = m_canvas->canvasWidget()->width()*m_canvas->canvasWidget()->width();
    MAX_DISTANCE += m_canvas->canvasWidget()->height()*m_canvas->canvasWidget()->height();
    int nearestPointIndex, i, j;
    // Get all the points
    QList<QPointF> connectionPoints1 = shape1->connectionPoints();
    QList<QPointF> connectionPoints2 = shape2->connectionPoints();
    
    // Find the nearest point and stock the index
    for( i = 0; i < connectionPoints1.count(); i++)
    {
        for( j = 0; j < connectionPoints2.count(); j++)
        {
            float distance = distanceSquare( connectionPoints1[i] + shape1->position(), connectionPoints2[j] + shape2->position());
            
            if( distance < MAX_DISTANCE )
            {
                MAX_DISTANCE = distance;
                nearestPointIndex =  i;
                break;
            }
        }
    }
    // Create and return the connection
    KoConnection newConnection = KoConnection(shape1, nearestPointIndex);
    return newConnection;
}

float KoConnectionTool::distanceSquare( QPointF p1, QPointF p2 )
{
    // Square of the distance
    float distx = ( p2.x() - p1.x() ) * ( p2.x() - p1.x() );
    float disty = ( p2.y() - p1.y() ) * ( p2.y() - p1.y() );
    float dist = distx + disty;
    return dist;
}

bool KoConnectionTool::approx( float x, float y )
{
    int x1, y1;
    x1 = (int) (x * 1000);
    y1 = (int) (y * 1000);
    if( x1 == y1)
        return true;
    return false;
}
