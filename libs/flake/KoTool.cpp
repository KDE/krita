/* This file is part of the KDE project
 * Copyright (C) 2006, 2010 Thomas Zander <zander@kde.org>
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

#include "KoTool.h"
#include "KoTool_p.h"
#include "KoCanvasBase.h"
#include "KoPointerEvent.h"
#include "KoCanvasResourceProvider.h"
#include "KoViewConverter.h"

#include <klocale.h>
#include <kactioncollection.h>
#include <QWidget>

KoTool::KoTool(KoCanvasBase *canvas)
    : d_ptr(new KoToolPrivate(this, canvas))
{
    Q_D(KoTool);
    if (d->canvas) { // in the case of KoToolManagers dummytool it can be zero :(
        KoCanvasResourceProvider * crp = d->canvas->resourceProvider();
        Q_ASSERT_X(crp, "KoTool::KoTool", "No KoCanvasResourceProvider");
        if (crp)
            connect(d->canvas->resourceProvider(), SIGNAL(resourceChanged(int, const QVariant &)),
                    this, SLOT(resourceChanged(int, const QVariant &)));
    }
}

KoTool::KoTool(KoToolPrivate &dd)
    : d_ptr(&dd)
{
}

KoTool::~KoTool()
{
    delete d_ptr;
}

void KoTool::activate(bool temporary)
{
    Q_UNUSED(temporary);
}

void KoTool::deactivate()
{
}

void KoTool::resourceChanged(int key, const QVariant & res)
{
    Q_UNUSED(key);
    Q_UNUSED(res);
}

bool KoTool::wantsAutoScroll()
{
    return true;
}

void KoTool::mouseDoubleClickEvent(KoPointerEvent *event)
{
    event->ignore();
}

void KoTool::keyPressEvent(QKeyEvent *e)
{
    e->ignore();
}

void KoTool::keyReleaseEvent(QKeyEvent *e)
{
    e->ignore();
}

void KoTool::wheelEvent(KoPointerEvent * e)
{
    e->ignore();
}

QVariant KoTool::inputMethodQuery(Qt::InputMethodQuery query, const KoViewConverter &) const
{
    Q_D(const KoTool);
    if (d->canvas->canvasWidget() == 0)
        return QVariant();

    switch (query) {
    case Qt::ImMicroFocus:
        return QRect(d->canvas->canvasWidget()->width() / 2, 0, 1, d->canvas->canvasWidget()->height());
    case Qt::ImFont:
        return d->canvas->canvasWidget()->font();
    default:
        return QVariant();
    }
}

void KoTool::inputMethodEvent(QInputMethodEvent * event)
{
    if (! event->commitString().isEmpty()) {
        QKeyEvent ke(QEvent::KeyPress, -1, 0, event->commitString());
        keyPressEvent(&ke);
    }
    event->accept();
}

void KoTool::customPressEvent(KoPointerEvent * event)
{
    event->ignore();
}

void KoTool::customReleaseEvent(KoPointerEvent * event)
{
    event->ignore();
}

void KoTool::customMoveEvent(KoPointerEvent * event)
{
    event->ignore();
}

void KoTool::useCursor(const QCursor &cursor)
{
    Q_D(KoTool);
    d->currentCursor = cursor;
    emit cursorChanged(d->currentCursor);
}

QMap<QString, QWidget *> KoTool::optionWidgets()
{
    Q_D(KoTool);
    if (d->optionWidgets.empty()) {
        d->optionWidgets = createOptionWidgets();
    }
    return d->optionWidgets;
}

void KoTool::addAction(const QString &name, KAction *action)
{
    Q_D(KoTool);
    d->actionCollection.insert(name, action);
}

QHash<QString, KAction*> KoTool::actions() const
{
    Q_D(const KoTool);
    return d->actionCollection;
}

KAction *KoTool::action(const QString &name) const
{
    Q_D(const KoTool);
    return d->actionCollection.value(name);
}

QWidget * KoTool::createOptionWidget()
{
    return 0;
}

QMap<QString, QWidget *>  KoTool::createOptionWidgets()
{
    QMap<QString, QWidget *> ow;
    if (QWidget *widget = createOptionWidget()) {
        if (widget->objectName().isEmpty()) {
            widget->setObjectName(toolId());
        }
        ow.insert(i18n("Tool Options"), widget);
    }
    return ow;
}

void KoTool::setToolId(const QString &id)
{
    Q_D(KoTool);
    d->toolId = id;
}

QString KoTool::toolId() const
{
    Q_D(const KoTool);
    return d->toolId;
}

QCursor KoTool::cursor() const
{
    Q_D(const KoTool);
    return d->currentCursor;
}

void KoTool::deleteSelection()
{
}

void KoTool::cut()
{
    copy();
    deleteSelection();
}

QList<QAction*> KoTool::popupActionList() const
{
    Q_D(const KoTool);
    return d->popupActionList;
}

void KoTool::setPopupActionList(const QList<QAction*> &list)
{
    Q_D(KoTool);
    d->popupActionList = list;
}

KoCanvasBase * KoTool::canvas() const
{
    Q_D(const KoTool);
    return d->canvas;
}

void KoTool::setStatusText(const QString &statusText)
{
    emit statusTextChanged(statusText);
}

QRectF KoTool::handleGrabRect(const QPointF &position)
{
    Q_D(KoTool);
    const KoViewConverter * converter = d->canvas->viewConverter();
    uint handleSize = 2*d->canvas->resourceProvider()->grabSensitivity();
    QRectF r = converter->viewToDocument(QRectF(0, 0, handleSize, handleSize));
    r.moveCenter(position);
    return r;
}

QRectF KoTool::handlePaintRect(const QPointF &position)
{
    Q_D(KoTool);
    const KoViewConverter * converter = d->canvas->viewConverter();
    uint handleSize = 2*d->canvas->resourceProvider()->handleRadius();
    QRectF r = converter->viewToDocument(QRectF(0, 0, handleSize, handleSize));
    r.moveCenter(position);
    return r;
}

QStringList KoTool::supportedPasteMimeTypes() const
{
    return QStringList();
}

bool KoTool::paste()
{
    return false;
}

void KoTool::copy() const
{
}

KoToolSelection *KoTool::selection()
{
    return 0;
}

void KoTool::repaintDecorations()
{
}

#include <KoTool.moc>
