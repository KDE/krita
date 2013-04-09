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
#include "kis_image.h"

class KisToolInvocationAction::Private
{
public:
    Private(KisToolInvocationAction *qq) : q(qq), active(false) { }
    QPointF tabletToPixel(const QPointF& globalPos);

    KisToolInvocationAction *q;
    bool active;
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

void KisToolInvocationAction::begin(int shortcut, QEvent *event)
{
    if (shortcut == ActivateShortcut) {
        QTabletEvent *tabletEvent = inputManager()->lastTabletEvent();
        QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(event);

        if (tabletEvent) {
            inputManager()->toolProxy()->tabletEvent(tabletEvent, d->tabletToPixel(tabletEvent->hiResGlobalPos()));
        } else if (mouseEvent) {
            inputManager()->toolProxy()->mousePressEvent(mouseEvent, inputManager()->widgetToPixel(mouseEvent->posF()));
        }

        d->active = true;
    } else if (shortcut == ConfirmShortcut) {
        QKeyEvent pressEvent(QEvent::KeyPress, Qt::Key_Return, 0);
        inputManager()->toolProxy()->keyPressEvent(&pressEvent);
        QKeyEvent releaseEvent(QEvent::KeyRelease, Qt::Key_Return, 0);
        inputManager()->toolProxy()->keyReleaseEvent(&releaseEvent);

        /**
         * All the tools now have a KisTool::requestStrokeEnd() method,
         * so they should use this instead of direct filtering Enter key
         * press. Until all the tools support it, we just duplicate the
         * key event and the method call
         */
        inputManager()->canvas()->image()->requestStrokeEnd();
    } else if (shortcut == CancelShortcut) {
        /**
         * The tools now have a KisTool::requestStrokeCancellation() method,
         * so just request it.
         */

        inputManager()->canvas()->image()->requestStrokeCancellation();
    }
}

void KisToolInvocationAction::end(QEvent *event)
{
    if (d->active) {
        QTabletEvent *tabletEvent = inputManager()->lastTabletEvent();
        QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(event);

        if (tabletEvent) {
            inputManager()->toolProxy()->tabletEvent(tabletEvent, d->tabletToPixel(tabletEvent->hiResGlobalPos()));
        } else {
            inputManager()->toolProxy()->mouseReleaseEvent(mouseEvent, inputManager()->widgetToPixel(mouseEvent->posF()));
        }

        d->active = false;
    }

    KisAbstractInputAction::end(event);
}

void KisToolInvocationAction::inputEvent(QEvent* event)
{
    if(event->type() == QEvent::MouseButtonPress) {
        QMouseEvent* mevent = static_cast<QMouseEvent*>(event);
        inputManager()->toolProxy()->mousePressEvent(mevent, inputManager()->widgetToPixel(mevent->posF()));
    } else if(event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent* mevent = static_cast<QMouseEvent*>(event);
        inputManager()->toolProxy()->mouseReleaseEvent(mevent, inputManager()->widgetToPixel(mevent->posF()));
    } else if(event->type() == QEvent::MouseMove) {
        QTabletEvent* tevent = inputManager()->lastTabletEvent();
        QMouseEvent* mevent = static_cast<QMouseEvent*>(event);
        if (tevent && tevent->type() == QEvent::TabletMove) {
            inputManager()->toolProxy()->tabletEvent(tevent, d->tabletToPixel(tevent->hiResGlobalPos()));
        } else {
            inputManager()->toolProxy()->mouseMoveEvent(mevent, inputManager()->widgetToPixel(mevent->posF()));
        }
    } else if(event->type() == QEvent::KeyPress) {
        QKeyEvent* kevent = static_cast<QKeyEvent*>(event);
        inputManager()->toolProxy()->keyPressEvent(kevent);
    } else if(event->type() == QEvent::KeyRelease) {
        QKeyEvent* kevent = static_cast<QKeyEvent*>(event);
        inputManager()->toolProxy()->keyReleaseEvent(kevent);
    }
}

bool KisToolInvocationAction::supportsHiResInputEvents() const
{
    return true;
}

QPointF KisToolInvocationAction::Private::tabletToPixel(const QPointF &globalPos)
{
    const QPointF pos = globalPos - q->inputManager()->canvas()->canvasWidget()->mapToGlobal(QPoint(0, 0));
    return q->inputManager()->widgetToPixel(pos);
}

