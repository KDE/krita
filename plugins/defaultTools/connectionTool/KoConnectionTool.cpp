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

#include <QUndoCommand>

KoConnectionTool::KoConnectionTool( KoCanvasBase * canvas )
    : KoPathTool(canvas)
    , m_firstShape(0)
{
}

KoConnectionTool::~KoConnectionTool()
{
}

void KoConnectionTool::paint( QPainter &painter, const KoViewConverter &converter)
{
}

void KoConnectionTool::mousePressEvent( KoPointerEvent *event )
{
    KoShape * tempShape = m_canvas->shapeManager()->shapeAt( event->point );
    if( tempShape != 0 )
    { // If the shape selected is not the background
	if( m_firstShape == 0 )
        { // If any connections is beginning
            m_firstShape = tempShape;
        }
        else
        { //If a connection is on

            // All sizes and positions are hardcoded for now
            KoShapeFactory *factory = KoShapeRegistry::instance()->value("KoConnectionShape");
            KoShape *shape = factory->createDefaultShapeAndInit( m_canvas->shapeController()->dataCenterMap() );
            KoConnectionShape * connectionShape = dynamic_cast<KoConnectionShape*>( shape );

            KoConnection newConnection = KoConnection(m_firstShape, 1);
            connectionShape->setConnection1( newConnection.first, newConnection.second );
	    
            newConnection = KoConnection(tempShape, 1);
            connectionShape->setConnection2( newConnection.first, newConnection.second );
            
	    QUndoCommand * cmd;
	    if( connectionShape != 0 )
                cmd = m_canvas->shapeController()->addShape( connectionShape );
	    
            if (cmd) {
                m_canvas->addCommand(cmd);
            } else {
                m_canvas->updateCanvas(connectionShape->boundingRect());
                delete connectionShape;
            }
	    
            m_firstShape = 0;
        }
    }
}

void KoConnectionTool::mouseMoveEvent( KoPointerEvent *event )
{
    event->ignore();
}

void KoConnectionTool::mouseReleaseEvent( KoPointerEvent *event )
{
}

void KoConnectionTool::activate( bool temporary )
{
}

void KoConnectionTool::deactivate()
{
    m_firstShape = 0;
}