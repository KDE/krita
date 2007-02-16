/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
 * Copyright (c) 2006 Boudewijn Rempt <boud@valdyas.org>
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
#include "KoToolProxy.h"

#include "KoTool.h"
#include "KoPointerEvent.h"

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


class KoToolProxy::Private {
public:
    Private() : activeTool(0) {}

    KoTool *activeTool;
};

KoToolProxy::KoToolProxy(KoCanvasBase *canvas)
    : d(new Private())
{
    // TODO register myself with the ToolManager
}

KoToolProxy::~KoToolProxy() {
    delete d;
}

void KoToolProxy::paint( QPainter &painter, KoViewConverter &converter ) {
    if (d->activeTool) d->activeTool->paint(painter, converter);
}

void KoToolProxy::repaintDecorations()
{
    if (d->activeTool) d->activeTool->repaintDecorations();
}

void KoToolProxy::tabletEvent( QTabletEvent *event, const QPointF &point )
{
    KoPointerEvent ev( event, point );
    switch( event->type() ) {
    case ( QEvent::TabletPress ):
        if (d->activeTool) d->activeTool->mousePressEvent( &ev );
        break;
    case ( QEvent::TabletRelease ):
        if (d->activeTool) d->activeTool->mouseReleaseEvent( &ev );
        break;
    //case ( QEvent::TabletMove ):
    default:
        if (d->activeTool) d->activeTool->mouseMoveEvent( &ev );
    }
}

void KoToolProxy::mousePressEvent( QMouseEvent *event, const QPointF &point )
{
    KoPointerEvent ev( event, point );
    if (d->activeTool) d->activeTool->mousePressEvent( &ev );
}

void KoToolProxy::mouseDoubleClickEvent( QMouseEvent *event, const QPointF &point )
{
    KoPointerEvent ev( event, point );
    if (d->activeTool) d->activeTool->mouseDoubleClickEvent( &ev );
}

void KoToolProxy::mouseMoveEvent( QMouseEvent *event, const QPointF &point )
{
    KoPointerEvent ev( event, point );
    if (d->activeTool) d->activeTool->mouseMoveEvent( &ev );
}

void KoToolProxy::mouseReleaseEvent( QMouseEvent *event, const QPointF &point )
{
    KoPointerEvent ev( event, point );
    if (d->activeTool) d->activeTool->mouseReleaseEvent( &ev );
}

void KoToolProxy::keyPressEvent(QKeyEvent *event)
{
    if (d->activeTool) d->activeTool->keyPressEvent( event );
}

void KoToolProxy::keyReleaseEvent(QKeyEvent *event)
{
    if (d->activeTool) d->activeTool->keyReleaseEvent( event );
}

void KoToolProxy::wheelEvent ( QWheelEvent * event, const QPointF &point )
{
    KoPointerEvent ev( event, point );
    if (d->activeTool) d->activeTool->wheelEvent( &ev );
}

KoToolSelection* KoToolProxy::selection() {
    if (d->activeTool)
        return d->activeTool->selection();
    return 0;
}

void KoToolProxy::setActiveTool(KoTool *tool) {
    d->activeTool = tool;
}

