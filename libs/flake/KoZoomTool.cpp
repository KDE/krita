/* This file is part of the KDE project
 *
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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

#include "KoZoomTool.h"
#include "KoZoomStrategy.h"
#include "KoPointerEvent.h"
#include "KoCanvasController.h"

//   #include <QMouseEvent>
//   #include <QPainter>
//
//   #include "KoInteractionStrategy.h"
//
//   #include <kcommand.h>

KoZoomTool::KoZoomTool(KoCanvasBase *canvas)
    : KoInteractionTool( canvas )
{
}

void KoZoomTool::paint( QPainter &painter, KoViewConverter &converter) {
    if ( m_currentStrategy )
        m_currentStrategy->paint( painter, converter);
}

void KoZoomTool::wheelEvent ( KoPointerEvent * event )
{
    if(event->modifiers() & Qt::ControlModifier)
    {
        if(event->delta() >0)
            m_controller->zoomIn(event->point);
        else
            m_controller->zoomOut(event->point);
    }
    else
        event->ignore();
}

void KoZoomTool::mouseReleaseEvent( KoPointerEvent *event ) {
    KoInteractionTool::mouseReleaseEvent(event);
    //emit KoTool::sigDone();
}

void KoZoomTool::mousePressEvent( KoPointerEvent *event ) {
    m_currentStrategy = new KoZoomStrategy(this, m_controller, event->point);
}
