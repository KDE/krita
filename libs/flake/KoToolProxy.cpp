/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (c) 2006-2011 Boudewijn Rempt <boud@valdyas.org>
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
#include "KoToolProxy_p.h"

#include "KoToolBase.h"
#include "KoPointerEvent.h"
#include "KoInputDevice.h"
#include "KoToolManager_p.h"
#include "KoToolSelection.h"
#include "KoCanvasBase.h"
#include "KoCanvasController.h"
#include "KoShapeManager.h"
#include "KoSelection.h"
#include "KoShapeLayer.h"

#include <kdebug.h>
#include <QTimer>
#include <QApplication>

KoToolProxyPrivate::KoToolProxyPrivate(KoToolProxy *p)
    : activeTool(0),
    tabletPressed(false),
    hasSelection(false),
    controller(0),
    parent(p)
{
    scrollTimer.setInterval(100);
    mouseLeaveWorkaround = false;
    multiClickCount = 0;
}

void KoToolProxyPrivate::timeout() // Auto scroll the canvas
{
    Q_ASSERT(controller);

    QPoint offset = QPoint(controller->canvasOffsetX(), controller->canvasOffsetY());
    QPoint origin = controller->canvas()->documentOrigin();
    QPoint viewPoint = widgetScrollPoint - origin - offset;

    QRectF mouseArea(viewPoint, QSizeF(10, 10));
    mouseArea.setTopLeft(mouseArea.center());

    controller->ensureVisible(mouseArea, true);

    QPoint newOffset = QPoint(controller->canvasOffsetX(), controller->canvasOffsetY());

    QPoint moved = offset - newOffset;
    if (moved.isNull())
        return;

    widgetScrollPoint += moved;

    QPointF documentPoint = parent->widgetToDocument(widgetScrollPoint);
    QMouseEvent event(QEvent::MouseMove, widgetScrollPoint, Qt::LeftButton, Qt::LeftButton, 0);
    KoPointerEvent ev(&event, documentPoint);
    activeTool->mouseMoveEvent(&ev);
}

void KoToolProxyPrivate::checkAutoScroll(const KoPointerEvent &event)
{
    if (controller == 0) return;
    if (!activeTool) return;
    if (!activeTool->wantsAutoScroll()) return;
    if (!event.isAccepted()) return;
    if (event.buttons() != Qt::LeftButton) return;

    widgetScrollPoint = event.pos();

    if (! scrollTimer.isActive())
        scrollTimer.start();
}

void KoToolProxyPrivate::selectionChanged(bool newSelection)
{
    if (hasSelection == newSelection)
        return;
    hasSelection = newSelection;
    emit parent->selectionChanged(hasSelection);
}

bool KoToolProxyPrivate::isActiveLayerEditable()
{
    if (!activeTool)
        return false;

    KoShapeManager * shapeManager = activeTool->canvas()->shapeManager();
    KoShapeLayer * activeLayer = shapeManager->selection()->activeLayer();
    if (activeLayer && !activeLayer->isEditable())
        return false;
    return true;
}

KoToolProxy::KoToolProxy(KoCanvasBase *canvas, QObject *parent)
        : QObject(parent),
        d(new KoToolProxyPrivate(this))
{
    KoToolManager::instance()->priv()->registerToolProxy(this, canvas);

    connect(&d->scrollTimer, SIGNAL(timeout()), this, SLOT(timeout()));
}

KoToolProxy::~KoToolProxy()
{
    delete d;
}

void KoToolProxy::paint(QPainter &painter, const KoViewConverter &converter)
{
    if (d->activeTool) d->activeTool->paint(painter, converter);
}

void KoToolProxy::repaintDecorations()
{
    if (d->activeTool) d->activeTool->repaintDecorations();
}

QPointF KoToolProxy::widgetToDocument(const QPointF &widgetPoint) const
{
    QPoint offset = QPoint(d->controller->canvasOffsetX(), d->controller->canvasOffsetY());
    QPoint origin = d->controller->canvas()->documentOrigin();
    QPoint viewPoint = widgetPoint.toPoint() - origin - offset;

    return d->controller->canvas()->viewConverter()->viewToDocument(viewPoint);
}

KoCanvasBase* KoToolProxy::canvas() const
{
    return d->controller->canvas();
}

#include <KDebug>
void KoToolProxy::tabletEvent(QTabletEvent *event, const QPointF &point)
{
    // don't process tablet events for stylus middle and right mouse button
    // they will be re-send as mouse events with the correct button. there is no possibility to get the button from the QTabletEvent.
    if(qFuzzyIsNull(event->pressure()) && d->tabletPressed==false && event->type()!=QEvent::TabletMove) {
        //kDebug()<<"don't accept tablet event: "<< point;
        return;
    }
    else {
        // Accept the tablet events as they are useless to parent widgets and they will
        // get re-send as mouseevents if we don't accept them.
        //kDebug()<<"accept tablet event: "<< point;
        event->accept();
    }

    KoInputDevice id(event->device(), event->pointerType(), event->uniqueId());
    KoToolManager::instance()->priv()->switchInputDevice(id);

    KoPointerEvent ev(event, point);
    switch (event->type()) {
    case QEvent::TabletPress:
        ev.setTabletButton(Qt::LeftButton);
        if (!d->tabletPressed && d->activeTool)
            d->activeTool->mousePressEvent(&ev);
        d->tabletPressed = true;
        break;
    case QEvent::TabletRelease:
        ev.setTabletButton(Qt::LeftButton);
        d->tabletPressed = false;
        d->scrollTimer.stop();
        if (d->activeTool)
            d->activeTool->mouseReleaseEvent(&ev);
        break;
    case QEvent::TabletMove:
        if (d->tabletPressed)
            ev.setTabletButton(Qt::LeftButton);
        if (d->activeTool)
            d->activeTool->mouseMoveEvent(&ev);
        d->checkAutoScroll(ev);
    default:
        ; // ignore the rest.
    }

    d->mouseLeaveWorkaround = true;
}

void KoToolProxy::mousePressEvent(KoPointerEvent *ev)
{
    d->mouseLeaveWorkaround = false;
    KoInputDevice id;
    KoToolManager::instance()->priv()->switchInputDevice(id);
    d->mouseDownPoint = ev->pos();

    if (d->tabletPressed) // refuse to send a press unless there was a release first.
        return;

    QPointF globalPoint = ev->globalPos();
    if (d->multiClickGlobalPoint != globalPoint) {
        if (qAbs(globalPoint.x() - d->multiClickGlobalPoint.x()) > 5||
            qAbs(globalPoint.y() - d->multiClickGlobalPoint.y()) > 5) {
            d->multiClickCount = 0;
        }
        d->multiClickGlobalPoint = globalPoint;
    }

    if (d->multiClickCount && d->multiClickTimeStamp.elapsed() < QApplication::doubleClickInterval()) {
        // One more multiclick;
        d->multiClickCount++;
    } else {
        d->multiClickTimeStamp.start();
        d->multiClickCount = 1;
    }

    if (d->activeTool) {
        switch (d->multiClickCount) {
        case 0:
        case 1:
            d->activeTool->mousePressEvent(ev);
            break;
        case 2:
            d->activeTool->mouseDoubleClickEvent(ev);
            break;
        case 3:
        default:
            d->activeTool->mouseTripleClickEvent(ev);
            break;
        }
    } else {
        d->multiClickCount = 0;
        ev->ignore();
    }
}

void KoToolProxy::mousePressEvent(QMouseEvent *event, const QPointF &point)
{
    KoPointerEvent ev(event, point);
    mousePressEvent(&ev);
}

void KoToolProxy::mouseDoubleClickEvent(QMouseEvent *event, const QPointF &point)
{
    KoPointerEvent ev(event, point);
    mouseDoubleClickEvent(&ev);
}

void KoToolProxy::mouseDoubleClickEvent(KoPointerEvent *event)
{
     // let us handle it as any other mousepress (where we then detect multi clicks
    mousePressEvent(event);
    if (!event->isAccepted() && d->activeTool)
        d->activeTool->canvas()->shapeManager()->suggestChangeTool(event);
}

void KoToolProxy::mouseMoveEvent(QMouseEvent *event, const QPointF &point)
{
    if (d->mouseLeaveWorkaround) {
        d->mouseLeaveWorkaround = false;
        return;
    }
    KoInputDevice id;
    KoToolManager::instance()->priv()->switchInputDevice(id);
    if (d->activeTool == 0) {
        event->ignore();
        return;
    }

    KoPointerEvent ev(event, point);
    d->activeTool->mouseMoveEvent(&ev);

    d->checkAutoScroll(ev);
}

void KoToolProxy::mouseMoveEvent(KoPointerEvent *event)
{
    if (d->mouseLeaveWorkaround) {
        d->mouseLeaveWorkaround = false;
        return;
    }
    KoInputDevice id;
    KoToolManager::instance()->priv()->switchInputDevice(id);
    if (d->activeTool == 0) {
        event->ignore();
        return;
    }

    d->activeTool->mouseMoveEvent(event);

    d->checkAutoScroll(*event);
}

void KoToolProxy::mouseReleaseEvent(QMouseEvent *event, const QPointF &point)
{
    d->mouseLeaveWorkaround = false;
    KoInputDevice id;
    KoToolManager::instance()->priv()->switchInputDevice(id);
    d->scrollTimer.stop();

    KoPointerEvent ev(event, point);
    if (d->activeTool) {
        d->activeTool->mouseReleaseEvent(&ev);

        if (! event->isAccepted() && event->button() == Qt::LeftButton && event->modifiers() == 0
                && qAbs(d->mouseDownPoint.x() - event->x()) < 5
                && qAbs(d->mouseDownPoint.y() - event->y()) < 5) {
            // we potentially will change the selection
            Q_ASSERT(d->activeTool->canvas());
            KoShapeManager *manager = d->activeTool->canvas()->shapeManager();
            Q_ASSERT(manager);
            // only change the selection if that will not lead to losing a complex selection
            if (manager->selection()->count() <= 1) {
                KoShape *shape = manager->shapeAt(point);
                if (shape && !manager->selection()->isSelected(shape)) { // make the clicked shape the active one
                    manager->selection()->deselectAll();
                    manager->selection()->select(shape);
                    QList<KoShape*> shapes;
                    shapes << shape;
                    QString tool = KoToolManager::instance()->preferredToolForSelection(shapes);
                    KoToolManager::instance()->switchToolRequested(tool);
                }
            }
        }
    } else {
        event->ignore();
    }
}

void KoToolProxy::mouseReleaseEvent(KoPointerEvent* event)
{
    d->mouseLeaveWorkaround = false;
    KoInputDevice id;
    KoToolManager::instance()->priv()->switchInputDevice(id);
    d->scrollTimer.stop();

    if (d->activeTool) {
        d->activeTool->mouseReleaseEvent(event);

        if (!event->isAccepted() && event->button() == Qt::LeftButton && event->modifiers() == 0
                && qAbs(d->mouseDownPoint.x() - event->x()) < 5
                && qAbs(d->mouseDownPoint.y() - event->y()) < 5) {
            // we potentially will change the selection
            Q_ASSERT(d->activeTool->canvas());
            KoShapeManager *manager = d->activeTool->canvas()->shapeManager();
            Q_ASSERT(manager);
            // only change the selection if that will not lead to losing a complex selection
            if (manager->selection()->count() <= 1) {
                KoShape *shape = manager->shapeAt(event->point);
                if (shape && !manager->selection()->isSelected(shape)) { // make the clicked shape the active one
                    manager->selection()->deselectAll();
                    manager->selection()->select(shape);
                    QList<KoShape*> shapes;
                    shapes << shape;
                    QString tool = KoToolManager::instance()->preferredToolForSelection(shapes);
                    KoToolManager::instance()->switchToolRequested(tool);
                }
            }
        }
    } else {
        event->ignore();
    }
}

void KoToolProxy::keyPressEvent(QKeyEvent *event)
{
    if (d->activeTool)
        d->activeTool->keyPressEvent(event);
    else
        event->ignore();
}

void KoToolProxy::keyReleaseEvent(QKeyEvent *event)
{
    if (d->activeTool)
        d->activeTool->keyReleaseEvent(event);
    else
        event->ignore();
}

void KoToolProxy::wheelEvent(QWheelEvent *event, const QPointF &point)
{
    KoPointerEvent ev(event, point);
    if (d->activeTool)
        d->activeTool->wheelEvent(&ev);
    else
        event->ignore();
}

void KoToolProxy::wheelEvent(KoPointerEvent *event)
{
    if (d->activeTool)
        d->activeTool->wheelEvent(event);
    else
        event->ignore();
}

QVariant KoToolProxy::inputMethodQuery(Qt::InputMethodQuery query, const KoViewConverter &converter) const
{
    if (d->activeTool)
        return d->activeTool->inputMethodQuery(query, converter);
    return QVariant();
}

void KoToolProxy::inputMethodEvent(QInputMethodEvent *event)
{
    if (d->activeTool) d->activeTool->inputMethodEvent(event);
}

void KoToolProxy::setActiveTool(KoToolBase *tool)
{
    if (d->activeTool)
        disconnect(d->activeTool, SIGNAL(selectionChanged(bool)), this, SLOT(selectionChanged(bool)));
    d->activeTool = tool;
    if (tool) {
        connect(d->activeTool, SIGNAL(selectionChanged(bool)), this, SLOT(selectionChanged(bool)));
        d->selectionChanged(hasSelection());
        emit toolChanged(tool->toolId());
    }
}

void KoToolProxyPrivate::setCanvasController(KoCanvasController *c)
{
    controller = c;
}

QHash<QString, KAction*> KoToolProxy::actions() const
{
    return d->activeTool ? d->activeTool->actions() : QHash<QString, KAction*>();
}

bool KoToolProxy::hasSelection() const
{
    return d->activeTool ? d->activeTool->hasSelection() : false;
}

void KoToolProxy::cut()
{
    // TODO maybe move checking the active layer to KoPasteController ?
    if (d->activeTool && d->isActiveLayerEditable())
        d->activeTool->cut();
}

void KoToolProxy::copy() const
{
    if (d->activeTool)
        d->activeTool->copy();
}

bool KoToolProxy::paste()
{
    // TODO maybe move checking the active layer to KoPasteController ?
    if (d->activeTool && d->isActiveLayerEditable())
        return d->activeTool->paste();
    return false;
}

void KoToolProxy::dragMoveEvent(QDragMoveEvent *event, const QPointF &point)
{
    if (d->activeTool)
        d->activeTool->dragMoveEvent(event, point);
}

void KoToolProxy::dragLeaveEvent(QDragLeaveEvent *event)
{
    if (d->activeTool)
        d->activeTool->dragLeaveEvent(event);
}

void KoToolProxy::dropEvent(QDropEvent *event, const QPointF &point)
{
    if (d->activeTool)
        d->activeTool->dropEvent(event, point);
}

QStringList KoToolProxy::supportedPasteMimeTypes() const
{
    if (d->activeTool)
        return d->activeTool->supportedPasteMimeTypes();

    return QStringList();
}

QList<QAction*> KoToolProxy::popupActionList() const
{
    if (d->activeTool)
        return d->activeTool->popupActionList();
    return QList<QAction*>();
}

void KoToolProxy::deleteSelection()
{
    if (d->activeTool)
        return d->activeTool->deleteSelection();
}

void KoToolProxy::processEvent(QEvent *e) const
{
    if(e->type()==QEvent::ShortcutOverride
       && d->activeTool
       && d->activeTool->isInTextMode()
       && static_cast<QKeyEvent*>(e)->modifiers()==Qt::NoModifier) {
        e->accept();
    }
}

KoToolProxyPrivate *KoToolProxy::priv()
{
    return d;
}

#include <KoToolProxy.moc>
