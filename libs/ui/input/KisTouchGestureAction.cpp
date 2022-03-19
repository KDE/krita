/*
 * SPDX-FileCopyrightText: 2022 Sharaf Zaman <shzam@sdf.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisTouchGestureAction.h"
#include <KisMainWindow.h>
#include <KisPart.h>
#include <QAction>
#include <kactioncollection.h>
#include <kis_debug.h>

KisTouchGestureAction::KisTouchGestureAction()
    : KisAbstractInputAction("Touch Gestures")
{
    setName(i18n("Touch Gestures"));
    setDescription(i18n("The Touch Gestures actions launch a single action for the specified gesture"));

    QHash<QString, int> shortcuts;
    shortcuts.insert(i18n("Undo"), UndoActionShortcut);
    shortcuts.insert(i18n("Redo"), RedoActionShortcut);
    shortcuts.insert(i18n("Toggle Canvas Only Mode"), ToggleCanvasOnlyShortcut);
    shortcuts.insert(i18n("Toggle Eraser"), ToggleEraserMode);
    setShortcutIndexes(shortcuts);
}

void KisTouchGestureAction::begin(int shortcut, QEvent *event)
{
    Q_UNUSED(event)
    m_shortcut = shortcut;
}

void KisTouchGestureAction::end(QEvent *event)
{
    Q_UNUSED(event)
    KActionCollection *actionCollection = KisPart::instance()->currentMainwindow()->actionCollection();
    switch (m_shortcut) {
    case UndoActionShortcut: {
        QAction *action = actionCollection->action("edit_undo");
        if (action) {
            action->trigger();
        }
        break;
    }
    case RedoActionShortcut: {
        QAction *action = actionCollection->action("edit_redo");
        if (action) {
            action->trigger();
        }
        break;
    }
    case ToggleCanvasOnlyShortcut: {
        QAction *action = actionCollection->action("view_show_canvas_only");
        if (action) {
            action->trigger();
        }
        break;
    }
    case ToggleEraserMode: {
        QAction *action = actionCollection->action("erase_action");
        if (action) {
            action->trigger();
        }
        break;
    }
    default:
        break;
    }
}

int KisTouchGestureAction::priority() const
{
    return 6;
}
