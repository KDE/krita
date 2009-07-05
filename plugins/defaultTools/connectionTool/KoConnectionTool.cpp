/* This file is part of the KDE project
 *
 * Copyright (C) 2009 Thorsten Zachmann <zachmann@kde.org>
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
#include <KoShapeController.h>
#include <KoShapeLayer.h>
#include <KoShapeRegistry.h>
#include <KoSelection.h>
#include <KoLineBorder.h>
#include <KoCanvasResourceProvider.h>

#include <QUndoCommand>
#include <QPointF>
#include <QKeyEvent>

#include <KDebug>

KoConnectionTool::KoConnectionTool( KoCanvasBase * canvas )
    : KoPathTool(canvas)
    , m_shapeOn(0)
    , m_lastShapeOn(0)
    , m_connectionShape(0)
{
    m_isTied = new QPair<bool, bool>(false,false);
}

KoConnectionTool::~KoConnectionTool()
{
}

void KoConnectionTool::paint( QPainter &painter, const KoViewConverter &converter)
{
    float x = 0;
    float y = 0;

    // get the correctly sized rect for painting handles
    QRectF handleRect = handlePaintRect(QPointF());

    painter.setRenderHint(QPainter::Antialiasing, true);
    if( m_shapeOn !=  0) {
        // save the painter to restore it later
        painter.save();
        // Apply the conversion make by the matrix transformation
        QMatrix transform = m_shapeOn->absoluteTransformation(0);
        KoShape::applyConversion(painter, converter);
        foreach( const QPointF &point, m_shapeOn->connectionPoints() )
        { // Draw all the connection point of the shape
            handleRect.moveCenter(transform.map(point));
            painter.fillRect( handleRect, QColor(Qt::green) );
        }
        painter.restore();
    }
    
    if( isInRoi() ){
        // save the painter to restore it later
        painter.save();   
        // Apply the conversion make by the matrix transformation
        QMatrix transform = m_lastShapeOn->absoluteTransformation(0);
        KoShape::applyConversion(painter, converter);
        QPointF pointSelected = m_lastShapeOn->connectionPoints().value( getConnectionIndex(m_lastShapeOn, m_mouse) );
        handleRect.moveCenter(transform.map( pointSelected ));
        painter.fillRect( handleRect, QColor(Qt::blue) );

        painter.restore();
    }
    
    if( m_connectionShape != 0 ) {
        painter.save();

        // Apply the conversion make by the matrix transformation
        painter.setMatrix(m_connectionShape->absoluteTransformation(&converter) * painter.matrix());
        m_connectionShape->paint( painter, converter );
        if ( m_connectionShape->border() ) {
            painter.save();
            m_connectionShape->border()->paintBorder( m_connectionShape, painter, converter );
            painter.restore();
        }

        painter.restore();
    }    
}

void KoConnectionTool::mousePressEvent( KoPointerEvent *event )
{
    if( event->modifiers() & Qt::ControlModifier )
            return;
    KoShape * tempShape;
    if( isInRoi() )
        tempShape = m_lastShapeOn;
    else
        tempShape = m_canvas->shapeManager()->shapeAt( event->point );
    if( dynamic_cast<KoSelection*>( tempShape ) )
        tempShape = 0;
    // First click
    if( m_connectionShape == 0 ) {
        // All sizes and positions are hardcoded for now
        KoShapeFactory *factory = KoShapeRegistry::instance()->value("KoConnectionShape");
        KoShape *shape = factory->createDefaultShapeAndInit( m_canvas->shapeController()->dataCenterMap() );
        m_connectionShape = dynamic_cast<KoConnectionShape*>( shape );
        // If the shape selected is not the background
        if( tempShape != 0 ) {
            if( isInRoi() ) {
                m_connectionShape->setConnection1( tempShape, getConnectionIndex( tempShape, m_mouse ));
                m_isTied->first = true;
            } else {
                m_connectionShape->setConnection1( tempShape, 0 );
            }
            m_connectionShape->moveHandle( 1, event->point );
        } else {
            // If the cursor points the background
            if( isInRoi() ) {
                m_connectionShape->setConnection1( tempShape, getConnectionIndex( tempShape, m_mouse ));
                m_isTied->first = true;
            } else
                m_connectionShape->moveHandle( 0, event->point );
            m_connectionShape->moveHandle( 1, event->point );
        }
        
        // The connection is now done, so update for apply
        m_connectionShape->updateConnections();
        m_canvas->updateCanvas(m_connectionShape->boundingRect());

    } else { 
    // Second click
        // Apply the connection shape for now
        command();
        // If the shape selected is not the background
        if( tempShape != 0 ) {
            if( isInRoi() ) {
                m_connectionShape->setConnection2( tempShape, getConnectionIndex( tempShape, m_mouse ));
                m_isTied->second = true;
            } else {
                m_connectionShape->setConnection2( tempShape, 0 );
            }
        } else { 
        // If the cursor points the background
            if( isInRoi() ) {
                m_connectionShape->setConnection2( tempShape, getConnectionIndex( tempShape, m_mouse ));
                m_isTied->second = true;
            } else {
                m_connectionShape->moveHandle( 1, event->point );
            }
        }
        // Will find the nearest point and update the connection shape
        updateConnections();
        m_connectionShape = 0;
    }
}

void KoConnectionTool::mouseMoveEvent( KoPointerEvent *event )
{
    if( m_shapeOn != 0 )
        m_lastShapeOn = m_shapeOn;
    m_mouse = event->point;
    m_shapeOn = m_canvas->shapeManager()->shapeAt( event->point );
    if( dynamic_cast<KoSelection*>( m_shapeOn ) )
        m_shapeOn = 0;
    
    if( m_connectionShape != 0 ) {
        if( isInRoi() ) {
            // Make the connection
            m_connectionShape->setConnection2( m_lastShapeOn, getConnectionIndex( m_lastShapeOn, m_mouse ));
            m_connectionShape->updateConnections();
        } else if( m_shapeOn != 0 ) {
            // Make the connection
            m_connectionShape->setConnection2( m_shapeOn, 0);
            
            updateConnections();
        } else {
            m_connectionShape->setConnection2( 0, 0);
            m_connectionShape->moveHandle( 1, m_mouse );

            updateConnections();
        }
    }
    m_canvas->updateCanvas(QRectF( 0, 0, m_canvas->canvasWidget()->width(), m_canvas->canvasWidget()->height() ));
}

void KoConnectionTool::mouseReleaseEvent( KoPointerEvent *event )
{
    if( event->modifiers() & Qt::ControlModifier ) {
        if(isInRoi()) {
            // delete a connection Point
            m_shapeOn->removeConnectionPoint( getConnectionIndex( m_lastShapeOn, m_mouse ) );
        }else{
            // add a connection Point
            m_shapeOn = m_canvas->shapeManager()->shapeAt( event->point );
            QPointF point = m_shapeOn->documentToShape(event->point);
            if( dynamic_cast<KoSelection*>( m_shapeOn ) )
                m_shapeOn = 0;
            m_shapeOn->addConnectionPoint( point );
        }
    }
}

void KoConnectionTool::keyPressEvent(QKeyEvent *event)
{
    if( event->key() == Qt::Key_Escape ) {
        deactivate();
    }
}

void KoConnectionTool::activate( bool temporary )
{
    m_canvas->canvasWidget()->setCursor( Qt::PointingHandCursor );
}

void KoConnectionTool::deactivate()
{
    repaint(QRectF( 0, 0, m_canvas->canvasWidget()->x(), m_canvas->canvasWidget()->y() ));
    m_canvas->updateCanvas(QRectF( 0, 0, m_canvas->canvasWidget()->x(), m_canvas->canvasWidget()->y() ));
    m_connectionShape = 0;
}

void KoConnectionTool::updateConnections()
{
    if( m_connectionShape == 0 )
        return;
    
    KoConnection connection1 = m_connectionShape->connection1();
    KoConnection connection2 = m_connectionShape->connection2();
    // If two shapes are connected
    if( connection1.first != 0 and connection2.first != 0 ) {
        KoShape* shape1 = connection1.first;
        KoShape* shape2 = connection2.first;
        if( !m_isTied->first ){
            m_connectionShape->setConnection1( shape1 , getConnectionIndex( shape1, shape2->absolutePosition() ) );
        }
        if( !m_isTied->second ){
            m_connectionShape->setConnection2( shape2 , getConnectionIndex( shape2, shape1->absolutePosition() ) );
        }
    // If only the first item of the connection is a shape
    } else if( connection1.first != 0 ) {
        KoShape* shape = connection1.first;
        QPointF point = m_connectionShape->handlePosition(1) + m_connectionShape->absolutePosition();
        if( !m_isTied->first ) {
            m_connectionShape->setConnection1( shape , getConnectionIndex( shape, point ) );
        }
    // If only the second item of the connection is a shape
    } else if( connection2.first != 0 ) {
        KoShape* shape = connection2.first;
        QPointF point = m_connectionShape->handlePosition(0) + m_connectionShape->absolutePosition();
        if( !m_isTied->second )
            m_connectionShape->setConnection2( shape , getConnectionIndex( shape, point ) );
    }
    // The connection is now done, so update and put everything to 0
    m_connectionShape->updateConnections();
}

int KoConnectionTool::getConnectionIndex( KoShape * shape, QPointF point )
{
    float MAX_DISTANCE = m_canvas->canvasWidget()->width()*m_canvas->canvasWidget()->width();
    MAX_DISTANCE += m_canvas->canvasWidget()->height()*m_canvas->canvasWidget()->height();
    int nearestPointIndex, i;
    // Get all the points
    QList<QPointF> connectionPoints = shape->connectionPoints();

    point = shape->documentToShape( point );
    // Find the nearest point and stock the index
    for( i = 0; i < connectionPoints.count(); i++)
    {
        float distance = distanceSquare( connectionPoints[i], point);
        if( distance < MAX_DISTANCE ) {
            MAX_DISTANCE = distance;
            nearestPointIndex =  i;
        }
    }
    // return the nearest point index
    return nearestPointIndex;
}


float KoConnectionTool::distanceSquare( QPointF p1, QPointF p2 )
{
    // Square of the distance
    float distx = ( p2.x() - p1.x() ) * ( p2.x() - p1.x() );
    float disty = ( p2.y() - p1.y() ) * ( p2.y() - p1.y() );
    float dist = distx + disty;
    return dist;
}

bool KoConnectionTool::isInRoi()
{
    int grabSensitivity = m_canvas->resourceProvider()->grabSensitivity() * m_canvas->resourceProvider()->grabSensitivity();
    if( m_lastShapeOn == 0 )
        return false;
    QPointF mouse = m_lastShapeOn->documentToShape( m_mouse );
    foreach( QPointF point, m_lastShapeOn->connectionPoints() )
        if( distanceSquare( mouse, point) <= grabSensitivity )
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
