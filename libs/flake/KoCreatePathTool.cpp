/* This file is part of the KDE project
 *
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

#include <KoCreatePathTool.h>

#include "KoPathShape.h"
#include "KoPointerEvent.h"
#include "KoLineBorder.h"
#include "KoShapeBorderModel.h"
#include "KoCanvasBase.h"
#include "KoShapeManager.h"
#include "KoCommand.h"

#include <QDebug>
#include <QPainter>


KoCreatePathTool::KoCreatePathTool( KoCanvasBase * canvas )
: KoTool( canvas )
, m_shape( 0 )
, m_activePoint( 0 )    
{
}

KoCreatePathTool::~KoCreatePathTool()
{
}

void KoCreatePathTool::paint( QPainter &painter, KoViewConverter &converter )
{
    if ( m_shape )
    {
        //qDebug() << "KoCreatePathTool::paint" << m_shape;
        painter.save();
        painter.setMatrix( m_shape->transformationMatrix( &converter ) * painter.matrix() );

        painter.save();
        m_shape->paint( painter, converter );
        painter.restore();
        if ( m_shape->border() ) 
        {
            painter.save();
            m_shape->border()->paintBorder( m_shape, painter, converter );
            painter.restore();
        }
        painter.restore();
    }
}

void KoCreatePathTool::mousePressEvent( KoPointerEvent *event )
{
    //qDebug() << "KoCreatePathTool::mousePressEvent" << m_shape << "point = " << event->point;
    if ( m_shape )
    {
        m_activePoint->setPoint( event->point );
    }
    else
    {
        m_shape = new KoPathShape();
        m_shape->setShapeId( KoPathShapeId );
        // TODO take properties form the resource provider
        m_shape->setBorder( new KoLineBorder( 1, Qt::black ) );
        m_activePoint = m_shape->moveTo( event->point );
    }
    m_canvas->updateCanvas( m_shape->boundingRect() );
}

void KoCreatePathTool::mouseDoubleClickEvent( KoPointerEvent *event )
{
    //qDebug() << "KoCreatePathTool::mouseDoubleClickEvent" << m_shape << "point = " << event->point;
    if ( m_shape )
    {
        m_activePoint->setPoint( event->point );

        m_shape->normalize();

        // this is done so that nothing happens when the mouseReleaseEvent for the this event is received 
        KoPathShape *pathShape = m_shape;
        m_shape = 0;

        KCommand * cmd = m_canvas->shapeController()->addShape( pathShape );
        if ( cmd )
        {
            KoSelection *selection = m_canvas->shapeManager()->selection();
            selection->deselectAll();
            selection->select(pathShape);

            m_canvas->addCommand( cmd, true );
        }
        else
        {
            m_canvas->updateCanvas( pathShape->boundingRect() );
            delete pathShape;
        }
    }
}

void KoCreatePathTool::mouseMoveEvent( KoPointerEvent *event )
{
    if ( m_shape )
    {
        //qDebug() << "KoCreatePathTool::mouseMoveEvent" << m_shape << "point = " << event->point;
        m_canvas->updateCanvas( m_shape->boundingRect() );
        if ( event->buttons() & Qt::LeftButton )
        {
            m_activePoint->setControlPoint2( event->point );
            if ( m_activePoint->properties() & KoPathPoint::CanHaveControlPoint1 )
            {
                m_activePoint->setControlPoint1( m_activePoint->point() + ( m_activePoint->point() - event->point ) );
            }
            m_canvas->updateCanvas( m_shape->boundingRect() );
        }
        else
        {
            m_activePoint->setPoint( event->point );
            m_canvas->updateCanvas( m_shape->boundingRect() );
        }
    }
}

void KoCreatePathTool::mouseReleaseEvent( KoPointerEvent *event )
{
    //qDebug() << "KoCreatePathTool::mouseReleaseEvent" << m_shape << "point = " << event->point;
    if ( m_shape )
    {
        m_activePoint = m_shape->lineTo( event->point );
    }
}

void KoCreatePathTool::activate( bool temporary )
{
    Q_UNUSED( temporary );
    useCursor(Qt::ArrowCursor, true);
}
