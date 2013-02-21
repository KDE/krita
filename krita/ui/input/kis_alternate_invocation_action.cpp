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

#include <QApplication>
#include <KLocalizedString>

#include <KoToolProxy.h>
#include <kis_canvas2.h>

#include "kis_input_manager.h"

KisAlternateInvocationAction::KisAlternateInvocationAction(KisInputManager *manager)
    : KisAbstractInputAction(manager)
{
    setName(i18n("Alternate Invocation"));
    setDescription(i18n("Alternate Invocation performs an alternate action with the current tool. For example, using the brush tool it picks a color from the canvas."));
    QHash<QString, int> shortcuts;
    shortcuts.insert(i18n("Toggle Primary Mode"), PrimaryAlternateToggleShortcut);
    shortcuts.insert(i18n("Toggle Secondary Mode"), SecondaryAlternateToggleShortcut);
    setShortcutIndexes(shortcuts);
}

KisAlternateInvocationAction::~KisAlternateInvocationAction()
{
}

void KisAlternateInvocationAction::begin(int shortcut, QEvent *event)
{
    KisAbstractInputAction::begin(shortcut, event);

    QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(event);
    QMouseEvent targetEvent(*mouseEvent);

    m_savedShortcut = (Shortcut)shortcut;

    switch (m_savedShortcut) {
    case PrimaryAlternateToggleShortcut:
        targetEvent = QMouseEvent(QEvent::MouseButtonPress, mouseEvent->pos(), Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier);
        break;
    case SecondaryAlternateToggleShortcut:
        targetEvent = QMouseEvent(QEvent::MouseButtonPress, mouseEvent->pos(), Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier | Qt::AltModifier);
        break;
    }

    inputManager()->toolProxy()->mousePressEvent(&targetEvent, inputManager()->widgetToPixel(mouseEvent->posF()));
}

void KisAlternateInvocationAction::end(QEvent *event)
{
    QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(event);

    QMouseEvent targetEvent(*mouseEvent);

    switch (m_savedShortcut) {
    case PrimaryAlternateToggleShortcut:
        targetEvent = QMouseEvent(QEvent::MouseButtonRelease, mouseEvent->pos(), Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier);
        break;
    case SecondaryAlternateToggleShortcut:
        targetEvent = QMouseEvent(QEvent::MouseButtonRelease, mouseEvent->pos(), Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier | Qt::AltModifier);
        break;
    }

    inputManager()->toolProxy()->mouseReleaseEvent(&targetEvent, inputManager()->widgetToPixel(mouseEvent->posF()));

    KisAbstractInputAction::end(event);
}

void KisAlternateInvocationAction::mouseMoved(const QPointF &lastPos, const QPointF &pos)
{
    Q_UNUSED(lastPos);

    QMouseEvent targetEvent(QEvent::MouseMove, pos.toPoint(), Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier);

    switch (m_savedShortcut) {
    case PrimaryAlternateToggleShortcut:
        targetEvent = QMouseEvent(QEvent::MouseMove, pos.toPoint(), Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier);
        break;
    case SecondaryAlternateToggleShortcut:
        targetEvent = QMouseEvent(QEvent::MouseMove, pos.toPoint(), Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier | Qt::AltModifier);
        break;
    }

    inputManager()->toolProxy()->mouseMoveEvent(&targetEvent, inputManager()->widgetToPixel(pos));
}
