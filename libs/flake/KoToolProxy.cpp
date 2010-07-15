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

KoToolProxyPrivate::KoToolProxyPrivate(KoToolProxy *p)
    : activeTool(0),
    tabletPressed(false),
    hasSelection(false),
    controller(0),
    parent(p)
{
    scrollTimer.setInterval(100);
    mouseLeaveWorkaround = false;
}

void KoToolProxyPrivate::timeout() // Auto scroll the canvas
{
    Q_ASSERT(controller);
    int offsetX = controller->canvasOffsetX();
    int offsetY = controller->canvasOffsetY();
    QRectF mouseArea(scrollEdgePoint, QSizeF(10, 10));
    mouseArea.setTopLeft(mouseArea.center());

    controller->ensureVisible(mouseArea, true);

    QPoint moved(offsetX - controller->canvasOffsetX(), offsetY - controller->canvasOffsetY());
    if (moved.x() == 0 && moved.y() == 0)
        return;
    scrollEdgePoint += moved;

    QMouseEvent event(QEvent::MouseMove, scrollEdgePoint, Qt::LeftButton, Qt::LeftButton, 0);
    KoPointerEvent ev(&event, scrollEdgePoint);
    activeTool->mouseMoveEvent(&ev);
}

void KoToolProxyPrivate::checkAutoScroll(const KoPointerEvent &event)
{
    if (controller == 0) return;
    if (!activeTool) return;
    if (!activeTool->wantsAutoScroll()) return;
    if (!event.isAccepted()) return;
    if (event.buttons() != Qt::LeftButton) return;
    scrollEdgePoint = event.point.toPoint();
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

void KoToolProxy::tabletEvent(QTabletEvent *event, const QPointF &point)
{
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

    // Always accept tablet events as they are useless to parent widgets and they will
    // get re-send as mouseevents if we don't accept them.
    event->accept();
    d->mouseLeaveWorkaround = true;
}

void KoToolProxy::mousePressEvent(QMouseEvent *event, const QPointF &point)
{
    d->mouseLeaveWorkaround = false;
    KoInputDevice id;
    KoToolManager::instance()->priv()->switchInputDevice(id);
    d->mouseDownPoint = event->pos();

    if (d->tabletPressed) // refuse to send a press unless there was a release first.
        return;

    KoPointerEvent ev(event, point);
    if (d->activeTool)
        d->activeTool->mousePressEvent(&ev);
    else
        event->ignore();
}

void KoToolProxy::mouseDoubleClickEvent(QMouseEvent *event, const QPointF &point)
{
    d->mouseLeaveWorkaround = false;
    KoInputDevice id;
    KoToolManager::instance()->priv()->switchInputDevice(id);
    if (d->activeTool == 0) {
        event->ignore();
        return;
    }

    KoPointerEvent ev(event, point);
    d->activeTool->mouseDoubleClickEvent(&ev);
    if (! event->isAccepted())
        d->activeTool->canvas()->shapeManager()->suggestChangeTool(&ev);
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

KoToolSelection* KoToolProxy::selection()
{
    if (d->activeTool)
        return d->activeTool->selection();
    return 0;
}

void KoToolProxy::setActiveTool(KoToolBase *tool)
{
    if (d->activeTool)
        disconnect(d->activeTool, SIGNAL(selectionChanged(bool)), this, SLOT(selectionChanged(bool)));
    d->activeTool = tool;
    if (tool) {
        connect(d->activeTool, SIGNAL(selectionChanged(bool)), this, SLOT(selectionChanged(bool)));
        d->selectionChanged(d->activeTool->selection() && d->activeTool->selection()->hasSelection());
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

KoToolProxyPrivate *KoToolProxy::priv()
{
    return d;
}

#include <KoToolProxy.moc>
