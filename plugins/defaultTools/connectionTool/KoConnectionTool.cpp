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
    , m_beginPoint(0)
    , m_connectionShape(0)
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
    // save the painter to restore it later
    painter.save();
    if( m_shapeOn !=  0)
    {
        // Apply the conversion make by the matrix transformation
        painter.setMatrix(m_shapeOn->absoluteTransformation(&converter) * painter.matrix());
        KoShape::applyConversion(painter, converter);
        foreach( QPointF point, m_shapeOn->connectionPoints() )
        { // Draw all the connection point of the shape
            x = point.x() - 3;
            y = point.y() - 3;
            if( distanceSquare( m_mouse, QPointF( x + 3, y + 3 )) > 225 )
                painter.fillRect( x, y, 6, 6, QColor(Qt::green) );
            else
                painter.fillRect( x, y, 6, 6, QColor(Qt::blue) );
                
        }
    }
    painter.restore();    
    if( m_beginPoint != 0 )
    {
        // Draw the line if there is a begin point
        KoShape::applyConversion(painter, converter);
        painter.setBrush( QColor(Qt::lightGray) );
        painter.drawLine( m_mouse, *m_beginPoint );
    }
}

void KoConnectionTool::getPointSelected(KoShape * shape)
{
    float x,y;
    m_pointSelected = 0;
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
        if( m_beginPoint == 0)
        { // If any connections is beginning
            m_firstShape = tempShape;
            getPointSelected( m_firstShape );
            
            // All sizes and positions are hardcoded for now
            KoShapeFactory *factory = KoShapeRegistry::instance()->value("KoConnectionShape");
            KoShape *shape = factory->createDefaultShapeAndInit( m_canvas->shapeController()->dataCenterMap() );
            m_connectionShape = dynamic_cast<KoConnectionShape*>( shape );
            
            if( m_pointSelected != 0 )
            { // If the user explicitly choose a point
                int index, i = 0;
                QList<QPointF> connectionPoints1 = m_firstShape->connectionPoints();
                for( i = 0; i < connectionPoints1.count(); i++)
                {
                    QPointF point = connectionPoints1[i] + m_firstShape->position();
                    if( approx(m_pointSelected->x(), point.x()) and approx(m_pointSelected->y(), point.y()) )
                    {
                        index = i;
                        m_beginPoint = new QPointF( point );
                    }
                }
                // We can make a connection cause we have the first point of it
                KoConnection myConnection1 = KoConnection( m_firstShape, index );
                m_connectionShape->setConnection1( myConnection1.first, myConnection1.second );
                m_pointSelected = 0;
            }
            else
            { // else the beginPoint take the mouse one
                m_beginPoint = new QPointF( event->point );
            }
        }
        else if( m_firstShape != tempShape )
        { //If a connection is on   
            if( m_firstShape != 0 )
            {
                getPointSelected( m_firstShape );
                if( m_pointSelected == 0 )
                { // If no point of the shape had been selected, we have to find the nearest point
                    KoConnection myConnection1 = getConnection( m_firstShape, tempShape );
                    m_connectionShape->setConnection1( myConnection1.first, myConnection1.second );
                }
            }
            // Le second point
            getPointSelected( tempShape );
            KoConnection myConnection2;
            if( m_pointSelected != 0 )
            {
                int index, i = 0;
                QList<QPointF> connectionPoints1 = tempShape->connectionPoints();
                if( m_firstShape != 0 )
                {
                    for( i = 0; i < connectionPoints1.count(); i++)
                    {
                        QPointF point = connectionPoints1[i] + tempShape->position();
                        if( approx(m_pointSelected->x(), point.x()) and approx(m_pointSelected->y(), point.y()) )
                            index = i;
                    }
                }
                else 
                {   
                    for( i = 0; i < connectionPoints1.count(); i++)
                    {
                        QPointF point = connectionPoints1[i] + tempShape->position();
                        if( approx(m_beginPoint->x(), point.x()) and approx(m_beginPoint->y(), point.y()) )
                            index = i;
                    }   
                }
                
                myConnection2 = KoConnection( tempShape, index );
                m_pointSelected = 0;
            }
            else if( m_firstShape != 0 )
            {
                myConnection2 = getConnection( tempShape, m_firstShape );
            }
            else
            {
                myConnection2 = getConnectionPointShape( tempShape, m_beginPoint );
                m_pointSelected = 0;
            }
            m_connectionShape->setConnection2( myConnection2.first, myConnection2.second );
            command();
            // The connection is now done, so update and put everything to 0
            m_connectionShape->updateConnections();
            m_firstShape = 0;
            m_pointSelected = 0;
            m_beginPoint = 0;
        }
    }
    else // If the mouse point on the background
    {
        // The first point
        if( m_beginPoint == 0)
        { // If any connections is beginning
            
            // All sizes and positions are hardcoded for now
            KoShapeFactory *factory = KoShapeRegistry::instance()->value("KoConnectionShape");
            KoShape *shape = factory->createDefaultShapeAndInit( m_canvas->shapeController()->dataCenterMap() );
            m_connectionShape = dynamic_cast<KoConnectionShape*>( shape );
            
            m_beginPoint = new QPointF( event->point );
            m_connectionShape->moveHandle( 0, *m_beginPoint );
        }
        else // The second point
        {
            m_connectionShape->moveHandle( 1, event->point );
            if ( m_firstShape != 0 )
            {
                KoConnection myConnection2 = getConnectionPointShape( m_firstShape, new QPointF( event->point ) );
                m_connectionShape->setConnection1( myConnection2.first, myConnection2.second );
            }
            command();
            // The connection is now done, so update and put everything to 0
            m_connectionShape->updateConnections();
            m_firstShape = 0;
            m_pointSelected = 0;
            m_beginPoint = 0;
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
    m_beginPoint = 0;
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

KoConnection KoConnectionTool::getConnectionPointShape( KoShape * shape1, QPointF * point )
{
    float MAX_DISTANCE = m_canvas->canvasWidget()->width()*m_canvas->canvasWidget()->width();
    MAX_DISTANCE += m_canvas->canvasWidget()->height()*m_canvas->canvasWidget()->height();
    int nearestPointIndex, i;
    // Get all the points
    QList<QPointF> connectionPoints1 = shape1->connectionPoints();
    
    // Find the nearest point and stock the index
    for( i = 0; i < connectionPoints1.count(); i++)
    {
        float distance = distanceSquare( connectionPoints1[i] + shape1->position(), *point);
        if( distance < MAX_DISTANCE )
        {
            MAX_DISTANCE = distance;
            nearestPointIndex =  i;
            break;
        }
    }
    // Create and return the connection
    KoConnection newConnection = KoConnection( shape1, nearestPointIndex);
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

void KoConnectionTool::command()
{
    // Create the command which will make the connection
    QUndoCommand * cmd;
    if( m_connectionShape != 0 )
        cmd = m_canvas->shapeController()->addShape( m_connectionShape );

    if (cmd) {
        m_canvas->addCommand(cmd);
    } else {
        m_canvas->updateCanvas(m_connectionShape->boundingRect());
        delete m_connectionShape;
    }
}