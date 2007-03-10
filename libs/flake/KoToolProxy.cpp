/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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
#include "KoInputDevice.h"
#include "KoToolManager.h"
#include "KoCanvasBase.h"
#include "KoCanvasController.h"

#include <kdebug.h>
#include <QTimer>

/* Unused for now.
#if 0
namespace {

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
    Private() : activeTool(0), tabletPressed(false), controller(0) {
        scrollTimer.setInterval(100);
    }

    void timeout() { // Auto scroll the canvas
        Q_ASSERT(controller);
        int offsetX = controller->canvasOffsetX();
        int offsetY = controller->canvasOffsetY();
        // get the points version of 10 pixels offset.
        QPointF offset = activeTool->m_canvas->viewConverter()->viewToDocument(QPointF(10, 10));
        QRectF mouseArea(scrollEdgePoint, QSizeF(offset.x(), offset.y()));
        mouseArea.setTopLeft(mouseArea.center());

        activeTool->m_canvas->ensureVisible(mouseArea);

        QPoint moved(offsetX - controller->canvasOffsetX(), offsetY - controller->canvasOffsetY());
        if(moved.x() == 0 && moved.y() == 0)
            return;
        scrollEdgePoint += activeTool->m_canvas->viewConverter()->viewToDocument(moved);

        QMouseEvent event(QEvent::MouseMove, scrollEdgePoint.toPoint(), Qt::LeftButton, Qt::LeftButton, 0);
        KoPointerEvent ev( &event, scrollEdgePoint );
        activeTool->mouseMoveEvent( &ev );
    }

    void checkAutoScroll(const KoPointerEvent &event) {
        if(controller == 0) return;
        if(!activeTool->wantsAutoScroll()) return;
        if(!event.isAccepted()) return;
        if(event.buttons() != Qt::LeftButton) return;
        scrollEdgePoint = event.point;
        if(! scrollTimer.isActive())
            scrollTimer.start();
    }

    KoTool *activeTool;
    bool tabletPressed;
    QTimer scrollTimer;
    QPointF scrollEdgePoint;
    KoCanvasController *controller;
};

KoToolProxy::KoToolProxy(KoCanvasBase *canvas, QObject *parent)
    : QObject(parent),
    d(new Private())
{
    KoToolManager::instance()->registerToolProxy(this, canvas);

    connect(&d->scrollTimer, SIGNAL(timeout()), this, SLOT(timeout()));
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
    KoInputDevice id(event->device(), event->pointerType(), event->uniqueId());
    KoToolManager::instance()->switchInputDevice(id);

    KoPointerEvent ev( event, point );
    switch( event->type() ) {
    case QEvent::TabletPress:
        ev.setTabletButton(Qt::LeftButton);
        if(! d->tabletPressed && d->activeTool)
            d->activeTool->mousePressEvent( &ev );
        d->tabletPressed = true;
        break;
    case QEvent::TabletRelease:
        d->tabletPressed = false;
        if (d->activeTool) d->activeTool->mouseReleaseEvent( &ev );
        break;
    case QEvent::TabletMove:
        if(d->tabletPressed) ev.setTabletButton(Qt::LeftButton);
        if (d->activeTool) d->activeTool->mouseMoveEvent( &ev );
        d->checkAutoScroll(ev);
    default:
        ; // ignore the rest.
    }

    // Always accept tablet events as they are useless to parent widgets and they will
    // get re-send as mouseevents if we don't accept them.
    event->accept();
}

void KoToolProxy::mousePressEvent( QMouseEvent *event, const QPointF &point )
{
    KoInputDevice id;
    KoToolManager::instance()->switchInputDevice(id);

    if(d->tabletPressed) // refuse to send a press unless there was a release first.
        return;

    KoPointerEvent ev( event, point );
    if (d->activeTool) d->activeTool->mousePressEvent( &ev );
}

void KoToolProxy::mouseDoubleClickEvent( QMouseEvent *event, const QPointF &point )
{
    KoInputDevice id;
    KoToolManager::instance()->switchInputDevice(id);

    KoPointerEvent ev( event, point );
    if (d->activeTool) d->activeTool->mouseDoubleClickEvent( &ev );
}

void KoToolProxy::mouseMoveEvent( QMouseEvent *event, const QPointF &point )
{
    KoInputDevice id;
    KoToolManager::instance()->switchInputDevice(id);
    if(d->activeTool == 0)
        return;

    KoPointerEvent ev( event, point );
    d->activeTool->mouseMoveEvent( &ev );

    d->checkAutoScroll(ev);
}

void KoToolProxy::mouseReleaseEvent( QMouseEvent *event, const QPointF &point )
{
    KoInputDevice id;
    KoToolManager::instance()->switchInputDevice(id);
    d->scrollTimer.stop();

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

void KoToolProxy::setCanvasController(KoCanvasController *controller) {
    d->controller = controller;
}

#include <KoToolProxy.moc>
