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
    , m_shapeOn(0)
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
    if( m_shapeOn !=  0) {
        // Apply the conversion make by the matrix transformation
        painter.setMatrix(m_shapeOn->absoluteTransformation(&converter) * painter.matrix());
        KoShape::applyConversion(painter, converter);
        foreach( QPointF point, m_shapeOn->connectionPoints() )
        { // Draw all the connection point of the shape
            x = point.x() - 3;
            y = point.y() - 3;
            painter.fillRect( x, y, 6, 6, QColor(Qt::green) );    
        }
    }
    painter.restore();

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
    KoShape * tempShape = m_canvas->shapeManager()->shapeAt( event->point );
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
            m_connectionShape->setConnection1( tempShape, 0 );
            m_connectionShape->moveHandle( 1, event->point );
        } else {
          // If the cursor points the background
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
            m_connectionShape->setConnection2( tempShape, 0 );
            // The connection is now done, so update and put everything to 0
            m_connectionShape->updateConnections();
        } else { 
        // If the cursor points the background
            m_connectionShape->moveHandle( 1, event->point );
        }
        m_connectionShape = 0;
    }
}

void KoConnectionTool::mouseMoveEvent( KoPointerEvent *event )
{
    m_shapeOn = m_canvas->shapeManager()->shapeAt( event->point );
    if( dynamic_cast<KoSelection*>( m_shapeOn ) )
        m_shapeOn = 0;
    if( m_connectionShape != 0 ) {
        m_connectionShape->moveHandle( 1, event->point );
    }
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
    repaint(QRectF( 0, 0, m_canvas->canvasWidget()->x(), m_canvas->canvasWidget()->y() ));
    m_canvas->updateCanvas(QRectF( 0, 0, m_canvas->canvasWidget()->x(), m_canvas->canvasWidget()->y() ));
    m_connectionShape = 0;
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
