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

#include "kis_alternate_invocation_action.h"

#include <KLocalizedString>

#include <KoToolProxy.h>

#include <kis_canvas2.h>

#include "kis_input_manager.h"

class KisAlternateInvocationAction::Private
{
public:
    QPointF mousePosition;
};

KisAlternateInvocationAction::KisAlternateInvocationAction(KisInputManager *manager)
    : KisAbstractInputAction(manager), d(new Private)
{
    setName(i18n("Alternate Invocation"));
    setDescription(i18n("Alternate Invocation performs an alternate action with the current tool. For example, using the brush tool it picks a color from the canvas."));
}

KisAlternateInvocationAction::~KisAlternateInvocationAction()
{
}

void KisAlternateInvocationAction::begin(int /*shortcut*/)
{
    QMouseEvent *mevent = new QMouseEvent(QEvent::MouseButtonPress, inputManager()->mousePosition().toPoint(), Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier);
    inputManager()->toolProxy()->mousePressEvent(mevent, inputManager()->mousePosition());
}

void KisAlternateInvocationAction::end()
{
    QMouseEvent *mevent = new QMouseEvent(QEvent::MouseButtonRelease, d->mousePosition.toPoint(), Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier);
    inputManager()->toolProxy()->mouseReleaseEvent(mevent, d->mousePosition);
}

void KisAlternateInvocationAction::inputEvent(QEvent* event)
{
    if(event->type() == QEvent::MouseMove) {
        QMouseEvent *mevent = static_cast<QMouseEvent*>(event);
        d->mousePosition = inputManager()->widgetToPixel(mevent->posF());
        inputManager()->toolProxy()->mouseMoveEvent(mevent, d->mousePosition);
    }
}

bool KisAlternateInvocationAction::isBlockingAutoRepeat() const
{
    return true;
}
