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
#include <klocalizedstring.h>

#include <kis_tool_proxy.h>
#include <kis_canvas2.h>

#include "kis_input_manager.h"
#include "kis_tool.h"
#include "kis_cursor.h"

struct KisAlternateInvocationAction::Private
{
    KisTool::ToolAction savedAction;
};

KisAlternateInvocationAction::KisAlternateInvocationAction()
    : KisAbstractInputAction("Alternate Invocation")
    , m_d(new Private)
{
    setName(i18n("Alternate Invocation"));
    setDescription(i18n("The <i>Alternate Invocation</i> action performs an alternate action with the current tool. For example, using the brush tool it picks a color from the canvas."));
    QHash<QString, int> shortcuts;
    shortcuts.insert(i18n("Toggle Primary Mode"), PrimaryAlternateToggleShortcut);
    shortcuts.insert(i18n("Toggle Secondary Mode"), SecondaryAlternateToggleShortcut);

    shortcuts.insert(i18n("Pick Foreground Color from Current Node"), PickColorFgNodeToggleShortcut);
    shortcuts.insert(i18n("Pick Background Color from Current Node"), PickColorBgNodeToggleShortcut);

    shortcuts.insert(i18n("Pick Foreground Color from Merged Image"), PickColorFgImageToggleShortcut);
    shortcuts.insert(i18n("Pick Background Color from Merged Image"), PickColorBgImageToggleShortcut);

    setShortcutIndexes(shortcuts);
}

KisAlternateInvocationAction::~KisAlternateInvocationAction()
{
}

KisTool::ToolAction KisAlternateInvocationAction::shortcutToToolAction(int shortcut)
{
    KisTool::ToolAction action = KisTool::Alternate_NONE;

    switch ((Shortcut)shortcut) {
    case PickColorFgNodeToggleShortcut:
        action = KisTool::AlternatePickFgNode;
        break;
    case PickColorBgNodeToggleShortcut:
        action = KisTool::AlternatePickBgNode;
        break;
    case PickColorFgImageToggleShortcut:
        action = KisTool::AlternatePickFgImage;
        break;
    case PickColorBgImageToggleShortcut:
        action = KisTool::AlternatePickBgImage;
        break;
    case PrimaryAlternateToggleShortcut:
        action = KisTool::AlternateSecondary;
        break;
    case SecondaryAlternateToggleShortcut:
        action = KisTool::AlternateThird;
        break;
    }

    return action;
}

void KisAlternateInvocationAction::activate(int shortcut)
{
    KisTool::ToolAction action = shortcutToToolAction(shortcut);
    inputManager()->toolProxy()->activateToolAction(action);
}

void KisAlternateInvocationAction::deactivate(int shortcut)
{
    KisTool::ToolAction action = shortcutToToolAction(shortcut);
    inputManager()->toolProxy()->deactivateToolAction(action);
}

int KisAlternateInvocationAction::priority() const
{
    return 9;
}

void KisAlternateInvocationAction::begin(int shortcut, QEvent *event)
{
    if (!event) return;

    KisAbstractInputAction::begin(shortcut, event);

    QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(event);
    QMouseEvent targetEvent(QEvent::MouseButtonPress, mouseEvent->pos(), Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier);;

    m_d->savedAction = shortcutToToolAction(shortcut);

    inputManager()->toolProxy()->forwardEvent(
        KisToolProxy::BEGIN, m_d->savedAction, &targetEvent, event,
        inputManager()->lastTabletEvent(),
        inputManager()->canvas()->canvasWidget()->mapToGlobal(QPoint(0, 0)));
}

void KisAlternateInvocationAction::end(QEvent *event)
{
    if (!event) return;

    QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(event);

    QMouseEvent targetEvent(*mouseEvent);

    switch (m_d->savedAction) {
    case KisTool::AlternatePickFgNode:
        targetEvent = QMouseEvent(QEvent::MouseButtonRelease, mouseEvent->pos(), Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier);
        break;
    case KisTool::AlternateThird:
        targetEvent = QMouseEvent(QEvent::MouseButtonRelease, mouseEvent->pos(), Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier | Qt::AltModifier);
        break;
    }

    inputManager()->toolProxy()->forwardEvent(
        KisToolProxy::END, m_d->savedAction, &targetEvent, event,
        inputManager()->lastTabletEvent(),
        inputManager()->canvas()->canvasWidget()->mapToGlobal(QPoint(0, 0)));

    KisAbstractInputAction::end(event);
}

void KisAlternateInvocationAction::inputEvent(QEvent* event)
{
    if (event && event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        QMouseEvent targetEvent(QEvent::MouseMove, mouseEvent->pos(), Qt::NoButton, Qt::LeftButton, Qt::ShiftModifier);

        switch (m_d->savedAction) {
        case KisTool::AlternatePickFgNode:
            targetEvent = QMouseEvent(QEvent::MouseMove, mouseEvent->pos(), Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier);
            break;
        case KisTool::AlternateThird:
            targetEvent = QMouseEvent(QEvent::MouseMove, mouseEvent->pos(), Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier | Qt::AltModifier);
            break;
        }

        inputManager()->toolProxy()->forwardEvent(
            KisToolProxy::CONTINUE, m_d->savedAction, &targetEvent, event,
            inputManager()->lastTabletEvent(),
            inputManager()->canvas()->canvasWidget()->mapToGlobal(QPoint(0, 0)));
    }
}
