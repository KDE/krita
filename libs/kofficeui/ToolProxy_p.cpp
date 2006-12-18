/* This file is part of the KDE project
 *
 * Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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
//   // local lib
#include "KoToolManager.h"
#include "ToolProxy_p.h"

//   // koffice
#include <KoTool.h>
#include <KoPointerEvent.h>
#include <KoInputDevice.h>

//   // Qt + kde
#include <QEvent>

/* Unused for now.
#if 0
namespace {

    // Time in ms that must pass after a tablet event before a mouse event is allowed to
    // change the input device to the mouse. This is needed because mouse events are always
    // sent to a receiver if it does not accept the tablet event.
    static const int MOUSE_CHANGE_EVENT_DELAY = 100;


    // Helper class to determine when the user might switch between
    // tablet and mouse.
    class TabletProximityFilter {
    public:

        TabletProximityFilter( KoToolManager * manager )
            : m_manager( manager )
            {
            }

        virtual ~TabletProximityFilter();

        bool eventFilter( QObject * object,  QEvent * event ) {

            if ( object == qApp ) {
                switch( event->type() ) {
                    case QEvent::TabletEnterProximity:
                        break;
                    case QEvent::TabletLeaveProximity:
                        break;
                    default:
                        break;
                }
            }
            return false;
        }

    private:

        KoToolManager * m_manager;
    };

}
#endif */

// ******** ToolProxy **********
ToolProxy::ToolProxy(KoCanvasBase *canvas)
    : m_canvas(canvas),
    m_activeTool(0)
{
}

void ToolProxy::paint( QPainter &painter, KoViewConverter &converter )
{
    if (m_activeTool) m_activeTool->paint(painter, converter);
}

void ToolProxy::repaintDecorations()
{
    if (m_activeTool) m_activeTool->repaintDecorations();
}

void ToolProxy::tabletEvent( QTabletEvent *event, const QPointF &point )
{
    KoPointerEvent ev( event, point );
    switch( event->type() ) {
    case ( QEvent::TabletPress ):
        if (m_activeTool) m_activeTool->mousePressEvent( &ev );
        break;
    case ( QEvent::TabletRelease ):
        if (m_activeTool) m_activeTool->mouseReleaseEvent( &ev );
        break;
    case ( QEvent::TabletMove ):
    default:
        if (m_activeTool) m_activeTool->mouseMoveEvent( &ev );
    }
}

void ToolProxy::mousePressEvent( QMouseEvent *event, const QPointF &point )
{
    KoPointerEvent ev( event, point );
    if (m_activeTool) m_activeTool->mousePressEvent( &ev );
}

void ToolProxy::mouseDoubleClickEvent( QMouseEvent *event, const QPointF &point )
{
    KoPointerEvent ev( event, point );
    if (m_activeTool) m_activeTool->mouseDoubleClickEvent( &ev );
}

void ToolProxy::mouseMoveEvent( QMouseEvent *event, const QPointF &point )
{
    KoPointerEvent ev( event, point );
    if (m_activeTool) m_activeTool->mouseMoveEvent( &ev );
}

void ToolProxy::mouseReleaseEvent( QMouseEvent *event, const QPointF &point )
{
    KoPointerEvent ev( event, point );
    if (m_activeTool) m_activeTool->mouseReleaseEvent( &ev );
}

void ToolProxy::keyPressEvent(QKeyEvent *event)
{
    if (m_activeTool) m_activeTool->keyPressEvent( event );
}

void ToolProxy::keyReleaseEvent(QKeyEvent *event)
{
    if (m_activeTool) m_activeTool->keyReleaseEvent( event );
}

void ToolProxy::wheelEvent ( QWheelEvent * event, const QPointF &point )
{
    KoPointerEvent ev( event, point );
    if (m_activeTool) m_activeTool->wheelEvent( &ev );
}

KoToolSelection* ToolProxy::selection() {
    if (m_activeTool)
        return m_activeTool->selection();
    return 0;
}
