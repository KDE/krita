/* This file is part of the KDE project
 * Copyright (C) 2008 Jan Hambrecht <jaham@gmx.net>
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

#include "KoGuidesTool.h"
#include "KoPointerEvent.h"
#include "KoCanvasBase.h"
#include "KoCanvasController.h"
#include <KoViewConverter.h>
#include <KoGuidesData.h>

#include <QtGui/QPainter>

class KoGuidesTool::Private
{
public:
    enum EditMode { None, AddGuide, EditGuide };
    Private() : orientation(Qt::Horizontal), index(-1), position(0), mode(None) {};
    Qt::Orientation orientation;
    int index;
    qreal position;
    EditMode mode;
};

KoGuidesTool::KoGuidesTool( KoCanvasBase * canvas )
    : KoTool( canvas ), d( new Private() )
{
}

KoGuidesTool::~KoGuidesTool()
{
    delete d;
}

void KoGuidesTool::paint( QPainter &painter, const KoViewConverter &converter )
{
    if( d->mode == Private::None )
        return;

    KoCanvasController * controller = m_canvas->canvasController();
    QPoint documentOrigin = m_canvas->documentOrigin();
    QPoint canvasOffset( controller->canvasOffsetX(), controller->canvasOffsetY() );

    QPointF start, end;
    if( d->orientation == Qt::Horizontal )
    {
        qreal left = -canvasOffset.x() - documentOrigin.x();
        qreal right = left + m_canvas->canvasWidget()->width();
        start = QPointF( left, converter.documentToViewY( d->position ) );
        end = QPointF( right, converter.documentToViewY( d->position ) );
    }
    else
    {
        qreal top = -canvasOffset.y() - documentOrigin.y();
        qreal bottom = top + m_canvas->canvasWidget()->height();
        start = QPointF( converter.documentToViewX( d->position ), top );
        end = QPointF( converter.documentToViewX( d->position ), bottom );
    }
    painter.setPen( Qt::red );
    painter.drawLine( start, end );
}

void KoGuidesTool::repaintDecorations()
{
    if( d->mode == Private::None )
        return;

    QRectF rect;
    KoCanvasController * controller = m_canvas->canvasController();
    QPoint documentOrigin = m_canvas->documentOrigin();
    QPoint canvasOffset( controller->canvasOffsetX(), controller->canvasOffsetY() ); 
    QRectF viewRect = m_canvas->viewConverter()->viewToDocument( m_canvas->canvasWidget()->rect() );
    if( d->orientation == Qt::Horizontal )
    {
        rect.setTop( d->position - 2.0 );
        rect.setBottom( d->position + 2.0 );
        rect.setLeft( m_canvas->viewConverter()->viewToDocumentX( -canvasOffset.x()-documentOrigin.x() ) );
        rect.setWidth( m_canvas->viewConverter()->viewToDocumentX( m_canvas->canvasWidget()->width() ) );
    }
    else
    {
        rect.setLeft( d->position - 2.0 );
        rect.setRight( d->position + 2.0 );
        rect.setTop( m_canvas->viewConverter()->viewToDocumentY( -canvasOffset.y()-documentOrigin.y() ) );
        rect.setHeight( m_canvas->viewConverter()->viewToDocumentY( m_canvas->canvasWidget()->height() ) );
    }
    m_canvas->updateCanvas( rect );
}

void KoGuidesTool::activate(bool temporary)
{
    if( d->mode != Private::None )
        useCursor( d->orientation == Qt::Horizontal ? Qt::SizeVerCursor : Qt::SizeHorCursor, true );
    else
        useCursor( Qt::ArrowCursor );
    m_canvas->canvasWidget()->grabMouse();
}

void KoGuidesTool::deactivate()
{
    m_canvas->canvasWidget()->releaseMouse();
}

void KoGuidesTool::mousePressEvent( KoPointerEvent *event )
{
}

void KoGuidesTool::mouseMoveEvent( KoPointerEvent *event )
{
    if( d->mode == Private::None )
        return;

    repaintDecorations();
    d->position = d->orientation == Qt::Horizontal ? event->point.y() : event->point.x();
    if( d->mode == Private::EditGuide )
    {
        KoGuidesData * guidesData = m_canvas->guidesData();
        if( guidesData )
        {
            if( d->orientation == Qt::Horizontal )
            {
                QList<double> guideLines = guidesData->horizontalGuideLines();
                guideLines[d->index] = d->position;
                guidesData->setHorizontalGuideLines( guideLines );
            }
            else
            {
                QList<double> guideLines = guidesData->verticalGuideLines();
                guideLines[d->index] = d->position;
                guidesData->setVerticalGuideLines( guideLines );
            }
        }
    }
    repaintDecorations();
}

void KoGuidesTool::mouseReleaseEvent( KoPointerEvent *event )
{
    KoGuidesData * guidesData = m_canvas->guidesData();
    if( d->mode == Private::AddGuide && guidesData )
    {
        // add the new guide line
        guidesData->addGuideLine( d->orientation, d->position );
    }

    emit done();
}

void KoGuidesTool::setNewGuideLine( Qt::Orientation orientation, qreal position )
{
    d->orientation = orientation;
    d->index = -1;
    d->position = position;
    d->mode = Private::AddGuide;
}

void KoGuidesTool::setEditGuideLine( Qt::Orientation orientation, uint index )
{
    d->orientation = orientation;
    d->index = index;
    d->mode = Private::EditGuide;
}
