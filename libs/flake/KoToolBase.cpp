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

#include "KoToolBase.h"
#include "KoToolBase_p.h"
#include "KoCanvasBase.h"
#include "KoPointerEvent.h"
#include "KoResourceManager.h"
#include "KoViewConverter.h"

#include <klocale.h>
#include <kactioncollection.h>
#include <QWidget>

KoToolBase::KoToolBase(KoCanvasBase *canvas)
    : d_ptr(new KoToolBasePrivate(this, canvas))
{
    Q_D(KoToolBase);
    if (d->canvas) { // in the case of KoToolManagers dummytool it can be zero :(
        KoResourceManager * crp = d->canvas->resourceManager();
        Q_ASSERT_X(crp, "KoToolBase::KoToolBase", "No KoResourceManager");
        if (crp)
            connect(d->canvas->resourceManager(), SIGNAL(resourceChanged(int, const QVariant &)),
                    this, SLOT(resourceChanged(int, const QVariant &)));
    }
}

KoToolBase::KoToolBase(KoToolBasePrivate &dd)
    : d_ptr(&dd)
{
}

KoToolBase::~KoToolBase()
{
    delete d_ptr;
}

void KoToolBase::deactivate()
{
}

void KoToolBase::resourceChanged(int key, const QVariant & res)
{
    Q_UNUSED(key);
    Q_UNUSED(res);
}

bool KoToolBase::wantsAutoScroll() const
{
    return true;
}

void KoToolBase::mouseDoubleClickEvent(KoPointerEvent *event)
{
    event->ignore();
}

void KoToolBase::keyPressEvent(QKeyEvent *e)
{
    e->ignore();
}

void KoToolBase::keyReleaseEvent(QKeyEvent *e)
{
    e->ignore();
}

void KoToolBase::wheelEvent(KoPointerEvent * e)
{
    e->ignore();
}

QVariant KoToolBase::inputMethodQuery(Qt::InputMethodQuery query, const KoViewConverter &) const
{
    Q_D(const KoToolBase);
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

void KoToolBase::inputMethodEvent(QInputMethodEvent * event)
{
    if (! event->commitString().isEmpty()) {
        QKeyEvent ke(QEvent::KeyPress, -1, 0, event->commitString());
        keyPressEvent(&ke);
    }
    event->accept();
}

void KoToolBase::customPressEvent(KoPointerEvent * event)
{
    event->ignore();
}

void KoToolBase::customReleaseEvent(KoPointerEvent * event)
{
    event->ignore();
}

void KoToolBase::customMoveEvent(KoPointerEvent * event)
{
    event->ignore();
}

void KoToolBase::useCursor(const QCursor &cursor)
{
    Q_D(KoToolBase);
    d->currentCursor = cursor;
    emit cursorChanged(d->currentCursor);
}

QMap<QString, QWidget *> KoToolBase::optionWidgets()
{
    Q_D(KoToolBase);
    if (d->optionWidgets.empty()) {
        d->optionWidgets = createOptionWidgets();
    }
    return d->optionWidgets;
}

void KoToolBase::addAction(const QString &name, KAction *action, ReadWrite readWrite)
{
    Q_D(KoToolBase);
    d->actionCollection.insert(name, action);
    if (readWrite == ReadOnlyAction)
        d->readOnlyActions.insert(action);
}

QHash<QString, KAction*> KoToolBase::actions(ReadWrite readWrite) const
{
    Q_D(const KoToolBase);
    QHash<QString, KAction*> answer = d->actionCollection;
    if (readWrite == ReadOnlyAction) {
        QHash<QString, KAction*>::Iterator iter = answer.begin();
        while (iter != answer.end()) {
            if (d->readOnlyActions.contains(iter.value()))
                iter = answer.erase(iter);
            else
                ++iter;
        }
    }
    return answer;
}

KAction *KoToolBase::action(const QString &name) const
{
    Q_D(const KoToolBase);
    return d->actionCollection.value(name);
}

QWidget * KoToolBase::createOptionWidget()
{
    return 0;
}

QMap<QString, QWidget *>  KoToolBase::createOptionWidgets()
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

void KoToolBase::setToolId(const QString &id)
{
    Q_D(KoToolBase);
    d->toolId = id;
}

QString KoToolBase::toolId() const
{
    Q_D(const KoToolBase);
    return d->toolId;
}

QCursor KoToolBase::cursor() const
{
    Q_D(const KoToolBase);
    return d->currentCursor;
}

void KoToolBase::deleteSelection()
{
}

void KoToolBase::cut()
{
    copy();
    deleteSelection();
}

QList<QAction*> KoToolBase::popupActionList() const
{
    Q_D(const KoToolBase);
    return d->popupActionList;
}

void KoToolBase::setPopupActionList(const QList<QAction*> &list)
{
    Q_D(KoToolBase);
    d->popupActionList = list;
}

KoCanvasBase * KoToolBase::canvas() const
{
    Q_D(const KoToolBase);
    return d->canvas;
}

void KoToolBase::setStatusText(const QString &statusText)
{
    emit statusTextChanged(statusText);
}

QRectF KoToolBase::handleGrabRect(const QPointF &position) const
{
    Q_D(const KoToolBase);
    const KoViewConverter * converter = d->canvas->viewConverter();
    uint handleSize = 2*d->canvas->resourceManager()->grabSensitivity();
    QRectF r = converter->viewToDocument(QRectF(0, 0, handleSize, handleSize));
    r.moveCenter(position);
    return r;
}

QRectF KoToolBase::handlePaintRect(const QPointF &position) const
{
    Q_D(const KoToolBase);
    const KoViewConverter * converter = d->canvas->viewConverter();
    uint handleSize = 2*d->canvas->resourceManager()->handleRadius();
    QRectF r = converter->viewToDocument(QRectF(0, 0, handleSize, handleSize));
    r.moveCenter(position);
    return r;
}

QStringList KoToolBase::supportedPasteMimeTypes() const
{
    return QStringList();
}

bool KoToolBase::paste()
{
    return false;
}

void KoToolBase::copy() const
{
}

KoToolSelection *KoToolBase::selection()
{
    return 0;
}

void KoToolBase::repaintDecorations()
{
}

void KoToolBase::setReadWrite(bool readWrite)
{
    Q_D(KoToolBase);
    d->readWrite = readWrite;
}

bool KoToolBase::isReadWrite() const
{
    Q_D(const KoToolBase);
    return d->readWrite;
}

#include <KoToolBase.moc>
