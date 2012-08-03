/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_tool_invocation_action.h"

#include <QDebug>

#include <KLocalizedString>

#include <KoToolProxy.h>
#include <kis_canvas2.h>
#include <kis_coordinates_converter.h>

#include "kis_input_manager.h"

class KisToolInvocationAction::Private
{
public:
    Private(KisToolInvocationAction *qq) : q(qq), useTablet(false) { }
    QPointF tabletToPixel(const QPointF& globalPos);

    KisToolInvocationAction *q;

    bool useTablet;
    QTabletEvent::TabletDevice tabletDevice;
    QTabletEvent::PointerType pointerType;
    int tabletZ;
    qint64 tabletID;

    Qt::KeyboardModifiers modifiers;
};

KisToolInvocationAction::KisToolInvocationAction(KisInputManager *manager)
    : KisAbstractInputAction(manager), d(new Private(this))
{
    setName(i18n("Tool Invocation"));
    setDescription(i18n("Tool Invocation invokes the current tool, for example, using the brush tool, it will start painting."));
}

KisToolInvocationAction::~KisToolInvocationAction()
{
    delete d;
}

void KisToolInvocationAction::begin(int shortcut)
{
    if (shortcut == ActivateShortcut) {
        if (inputManager()->tabletPressEvent()) {
            QTabletEvent *pressEvent = inputManager()->tabletPressEvent();
            inputManager()->toolProxy()->tabletEvent(pressEvent, d->tabletToPixel(pressEvent->hiResGlobalPos()));
            d->useTablet = true;
            d->pointerType = pressEvent->pointerType();
            d->tabletDevice = pressEvent->device();
            d->tabletZ = pressEvent->z();
            d->tabletID = pressEvent->uniqueId();
            setMousePosition(d->tabletToPixel(pressEvent->hiResGlobalPos()));

        } else {
            QMouseEvent pressEvent(QEvent::MouseButtonPress, inputManager()->mousePosition().toPoint(), Qt::LeftButton, Qt::LeftButton, 0);
            inputManager()->toolProxy()->mousePressEvent(&pressEvent, inputManager()->mousePosition());
            setMousePosition(inputManager()->mousePosition());
        }
    } else {
        QKeyEvent pressEvent(QEvent::KeyPress, Qt::Key_Return, 0);
        inputManager()->toolProxy()->keyPressEvent(&pressEvent);
        QKeyEvent releaseEvent(QEvent::KeyRelease, Qt::Key_Return, 0);
        inputManager()->toolProxy()->keyReleaseEvent(&releaseEvent);
    }
}

void KisToolInvocationAction::end()
{
    if(d->useTablet) {
        QTabletEvent releaseEvent(QEvent::TabletRelease, mousePosition().toPoint(), mousePosition().toPoint(), mousePosition(), d->tabletDevice, d->pointerType, 0.f, 0, 0, 0.f, 0.f, d->tabletZ, d->modifiers, d->tabletID);
        inputManager()->toolProxy()->tabletEvent(&releaseEvent, mousePosition());
        d->useTablet = false;
    } else {
        QMouseEvent releaseEvent(QEvent::MouseButtonRelease, mousePosition().toPoint(), Qt::LeftButton, Qt::LeftButton, d->modifiers);
        inputManager()->toolProxy()->mouseReleaseEvent(&releaseEvent, mousePosition());
    }
}

void KisToolInvocationAction::inputEvent(QEvent* event)
{
    if(event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mevent = static_cast<QMouseEvent*>(event);
        setMousePosition(inputManager()->widgetToPixel(mevent->posF()));
        d->modifiers = mevent->modifiers();
        inputManager()->toolProxy()->mousePressEvent(mevent, mousePosition());
    } else if(event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent* mevent = static_cast<QMouseEvent*>(event);
        setMousePosition(inputManager()->widgetToPixel(mevent->posF()));
        d->modifiers = mevent->modifiers();
        inputManager()->toolProxy()->mouseReleaseEvent(mevent, mousePosition());
    } else if(event->type() == QEvent::MouseMove) {
        QMouseEvent* mevent = static_cast<QMouseEvent*>(event);
        setMousePosition(inputManager()->widgetToPixel(mevent->posF()));
        d->modifiers = mevent->modifiers();
        inputManager()->toolProxy()->mouseMoveEvent(mevent, mousePosition());
    } else if(event->type() == QEvent::TabletMove) {
        QTabletEvent* tevent = static_cast<QTabletEvent*>(event);
        setMousePosition(d->tabletToPixel(tevent->hiResGlobalPos()));
        d->modifiers = tevent->modifiers();
        inputManager()->toolProxy()->tabletEvent(tevent, mousePosition());
    } else if(event->type() == QEvent::KeyPress) {
        QKeyEvent* kevent = static_cast<QKeyEvent*>(event);
        inputManager()->toolProxy()->keyPressEvent(kevent);
    } else if(event->type() == QEvent::KeyRelease) {
        QKeyEvent* kevent = static_cast<QKeyEvent*>(event);
        inputManager()->toolProxy()->keyReleaseEvent(kevent);
    }
}

bool KisToolInvocationAction::handleTablet() const
{
    return true;
}

QPointF KisToolInvocationAction::Private::tabletToPixel(const QPointF &globalPos)
{
    const QPointF pos = globalPos - q->inputManager()->canvas()->canvasWidget()->mapToGlobal(QPoint(0, 0));
    return q->inputManager()->widgetToPixel(pos);
}
