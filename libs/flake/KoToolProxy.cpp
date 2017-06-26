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

#include <QMimeData>
#include <QUrl>
#include <QTimer>
#include <QClipboard>
#include <QApplication>

#include <kundo2command.h>
#include <KoProperties.h>

#include <FlakeDebug.h>
#include <klocalizedstring.h>

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
#include "KoShapePaste.h"
#include "KoShapeRegistry.h"
#include "KoShapeController.h"
#include "KoOdf.h"
#include "KoViewConverter.h"
#include "KoShapeFactoryBase.h"


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
    QPointF viewPoint = widgetPoint.toPoint() - QPointF(origin - offset);

    return d->controller->canvas()->viewConverter()->viewToDocument(viewPoint);
}

KoCanvasBase* KoToolProxy::canvas() const
{
    return d->controller->canvas();
}

void KoToolProxy::tabletEvent(QTabletEvent *event, const QPointF &point)
{
    // We get these events exclusively from KisToolProxy - accept them
    event->accept();

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

    if (d->tabletPressed) { // refuse to send a press unless there was a release first.
        return;
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
    if (!event->isAccepted() && d->activeTool) {
        d->activeTool->canvas()->shapeManager()->suggestChangeTool(event);
    }
    d->activeTool->mouseDoubleClickEvent(event);
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
            if (manager->selection() && manager->selection()->count() <= 1) {
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
            if (manager->selection() && manager->selection()->count() <= 1) {
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

QHash<QString, QAction *> KoToolProxy::actions() const
{
    return d->activeTool ? d->activeTool->actions() : QHash<QString, QAction *>();
}

bool KoToolProxy::hasSelection() const
{
    return d->activeTool ? d->activeTool->hasSelection() : false;
}

void KoToolProxy::cut()
{
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
    bool success = false;
    KoCanvasBase *canvas = d->controller->canvas();

    if (d->activeTool && d->isActiveLayerEditable())
        success = d->activeTool->paste();

    if (!success) {
        const QMimeData *data = QApplication::clipboard()->mimeData();

        if (data->hasFormat(KoOdf::mimeType(KoOdf::Text))) {
            KoShapeManager *shapeManager = canvas->shapeManager();
            KoShapePaste paste(canvas, shapeManager->selection()->activeLayer());
            success = paste.paste(KoOdf::Text, data);
            if (success) {
                shapeManager->selection()->deselectAll();
                Q_FOREACH (KoShape *shape, paste.pastedShapes()) {
                    shapeManager->selection()->select(shape);
                }
            }
        }
    }

    if (!success) {
        const QMimeData *data = QApplication::clipboard()->mimeData();

        QList<QImage> imageList;

        QImage image = QApplication::clipboard()->image();

        if (!image.isNull()) {
            imageList << image;
        }
        // QT5TODO: figure out how to download data synchronously, which is deprecated in frameworks.
        else if (data->hasUrls()) {
            QList<QUrl> urls = QApplication::clipboard()->mimeData()->urls();
            foreach (const QUrl &url, urls) {
                QImage image;
                image.load(url.toLocalFile());
                if (!image.isNull()) {
                    imageList << image;
                }
            }
        }

        KoShapeFactoryBase *factory = KoShapeRegistry::instance()->value("PictureShape");
        QWidget *canvasWidget = canvas->canvasWidget();
        const KoViewConverter *converter = canvas->viewConverter();
        if (imageList.length() > 0 && factory && canvasWidget) {
            KUndo2Command *cmd = new KUndo2Command(kundo2_i18n("Paste Image"));
            Q_FOREACH (const QImage &image, imageList) {
                if (!image.isNull()) {
                    QPointF p = converter->viewToDocument(canvasWidget->mapFromGlobal(QCursor::pos()) + canvas->canvasController()->documentOffset()- canvasWidget->pos());
                    KoProperties params;
                    params.setProperty("qimage", image);

                    KoShape *shape = factory->createShape(&params, canvas->shapeController()->resourceManager());
                    shape->setPosition(p);

                    // add shape to the document
                    canvas->shapeController()->addShapeDirect(shape, cmd);

                    success = true;
                }
            }
            canvas->addCommand(cmd);
        }
    }
    return success;
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
            && (static_cast<QKeyEvent*>(e)->modifiers()==Qt::NoModifier ||
                static_cast<QKeyEvent*>(e)->modifiers()==Qt::ShiftModifier)) {
        e->accept();
    }
}

KoToolProxyPrivate *KoToolProxy::priv()
{
    return d;
}

//have to include this because of Q_PRIVATE_SLOT
#include "moc_KoToolProxy.cpp"
