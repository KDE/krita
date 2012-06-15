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

KisToolInvocationAction::KisToolInvocationAction(KisInputManager *manager)
    : KisAbstractInputAction(manager), m_tablet(false)
{
    setName(i18n("Tool Invocation"));
    setDescription(i18n("Tool Invocation invokes the current tool, for example, using the brush tool, it will start painting."));
}

KisToolInvocationAction::~KisToolInvocationAction()
{
}

void KisToolInvocationAction::begin(int /*shortcut*/)
{
    if(inputManager()->tabletPressEvent()) {
        inputManager()->tabletPressEvent()->accept();
        inputManager()->toolProxy()->tabletEvent(inputManager()->tabletPressEvent(), inputManager()->canvas()->coordinatesConverter()->widgetToDocument(inputManager()->tabletPressEvent()->pos()));
        m_tablet = true;
    } else {
        QMouseEvent *pressEvent = new QMouseEvent(QEvent::MouseButtonPress, inputManager()->mousePosition().toPoint(), Qt::LeftButton, Qt::LeftButton, 0);
        inputManager()->toolProxy()->mousePressEvent(pressEvent, pressEvent->pos());
    }
}

void KisToolInvocationAction::end()
{
    if(m_tablet) {
        QTabletEvent* pressEvent = inputManager()->tabletPressEvent();
        QTabletEvent *releaseEvent = new QTabletEvent(QEvent::TabletRelease, inputManager()->mousePosition().toPoint(), inputManager()->mousePosition().toPoint(), inputManager()->mousePosition(), pressEvent->device(), pressEvent->pointerType(), 0.f, 0, 0, 0.f, 0.f, pressEvent->z(), 0, pressEvent->uniqueId());
        inputManager()->toolProxy()->tabletEvent(releaseEvent, releaseEvent->pos());
    } else {
        QMouseEvent *releaseEvent = new QMouseEvent(QEvent::MouseButtonRelease, inputManager()->mousePosition().toPoint(), Qt::LeftButton, Qt::LeftButton, 0);
        inputManager()->toolProxy()->mouseReleaseEvent(releaseEvent, releaseEvent->pos());
    }
}

void KisToolInvocationAction::inputEvent(QEvent* event)
{
    if(event->type() == QEvent::MouseMove) {
        QMouseEvent* mevent = static_cast<QMouseEvent*>(event);
        inputManager()->toolProxy()->mouseMoveEvent(mevent, inputManager()->canvas()->coordinatesConverter()->widgetToDocument(mevent->posF()));
    } else if(event->type() == QEvent::TabletMove) {
        QTabletEvent* tevent = static_cast<QTabletEvent*>(event);
        inputManager()->toolProxy()->tabletEvent(tevent, inputManager()->canvas()->coordinatesConverter()->widgetToDocument(tevent->pos()));
    }
}

bool KisToolInvocationAction::handleTablet() const
{
    return true;
}
