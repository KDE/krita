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
    shortcuts.insert(i18n("Toggle Eraser Preset"), ToggleEraserPreset);
    shortcuts.insert(i18n("Reset Display"), ResetDisplay);
    shortcuts.insert(i18n("Toggle Previous Brush Preset"), PreviousPresetShortcut);
    shortcuts.insert(i18n("Color Sampler"), ColorSampler);
    shortcuts.insert(i18n("Deselect"), Deselect);
    shortcuts.insert(i18n("Activate Next Layer"), NextLayer);
    shortcuts.insert(i18n("Activate Previous Layer"), PreviousLayer);
    shortcuts.insert(i18n("Activate Freehand Brush Tool"), FreehandBrush);
    shortcuts.insert(i18n("Freehand Selection Tool"), KisToolSelectContiguous);
    shortcuts.insert(i18n("Activate Move Tool"), KisToolMove);
    shortcuts.insert(i18n("Activate Transform Tool"), KisToolTransform);
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
    KisKActionCollection *actionCollection = KisPart::instance()->currentMainwindow()->actionCollection();
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
     case ResetDisplay: {
        QAction *action = actionCollection->action("reset_display");
        if (action) {
            action->trigger();
        }
        break;
    }
    case PreviousPresetShortcut: {
        QAction *action = actionCollection->action("previous_preset");
        if (action) {
            action->trigger();
        }
        break;
    }
    case ColorSampler: {
        QAction *action = actionCollection->action("KisToolColorSampler");
        if (action) {
            action->trigger();
        }
        break;
    }
    case Deselect: {
        QAction *action = actionCollection->action("deselect");
        if (action) {
            action->trigger();
        }
        break;
    }
    case NextLayer: {
        QAction *action = actionCollection->action("activateNextLayer");
        if (action) {
            action->trigger();
        }
        break;
    }
    case PreviousLayer: {
        QAction *action = actionCollection->action("activatePreviousLayer");
        if (action) {
            action->trigger();
        }
        break;
    }
    case FreehandBrush: {
        QAction *action = actionCollection->action("FreehandBrush");
        if (action) {
            action->trigger();
        }
        break;
    }
    case KisToolSelectContiguous: {
        QAction *action = actionCollection->action("KisToolSelectContiguous");
        if (action) {
            action->trigger();
        }
        break;
    }
    case KisToolMove: {
        QAction *action = actionCollection->action("KisToolMove");
        if (action) {
            action->trigger();
        }
        break;
    }
    case KisToolTransform: {
        QAction *action = actionCollection->action("KisToolTransform");
        if (action) {
            action->trigger();
        }
        break;
    }
    case ToggleEraserPreset: {
        QAction *action = actionCollection->action("eraser_preset_action");
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
