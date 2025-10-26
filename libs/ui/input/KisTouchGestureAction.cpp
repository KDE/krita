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

    QString actionName;
    switch (m_shortcut) {
    case UndoActionShortcut:
        actionName = QStringLiteral("edit_undo");
        break;
    case RedoActionShortcut:
        actionName = QStringLiteral("edit_redo");
        break;
    case ToggleCanvasOnlyShortcut:
        actionName = QStringLiteral("view_show_canvas_only");
        break;
    case ToggleEraserMode:
        actionName = QStringLiteral("erase_action");
        break;
    case ResetDisplay:
        actionName = QStringLiteral("reset_display");
        break;
    case PreviousPresetShortcut:
        actionName = QStringLiteral("previous_preset");
        break;
    case ColorSampler:
        actionName = QStringLiteral("KritaSelected/KisToolColorSampler");
        break;
    case Deselect:
        actionName = QStringLiteral("deselect");
        break;
    case NextLayer:
        actionName = QStringLiteral("activateNextLayer");
        break;
    case PreviousLayer:
        actionName = QStringLiteral("activatePreviousLayer");
        break;
    case FreehandBrush:
        actionName = QStringLiteral("KritaShape/KisToolBrush");
        break;
    case KisToolSelectContiguous:
        actionName = QStringLiteral("KisToolSelectOutline");
        break;
    case KisToolMove:
        actionName = QStringLiteral("KritaTransform/KisToolMove");
        break;
    case KisToolTransform:
        actionName = QStringLiteral("KisToolTransform");
        break;
    case ToggleEraserPreset:
        actionName = QStringLiteral("eraser_preset_action");
        break;
    }

    if (actionName.isEmpty()) {
        qWarning("KisTouchGestureAction: Unhandled shortcut %d", m_shortcut);
    } else {
        KisKActionCollection *actionCollection = KisPart::instance()->currentMainwindow()->actionCollection();
        QAction *action = actionCollection->action(actionName);
        if (action) {
            action->trigger();
        } else {
            qWarning("KisTouchGestureAction: unable to find action '%s' for shortcut %d",
                     qUtf8Printable(actionName),
                     m_shortcut);
        }
    }
}

int KisTouchGestureAction::priority() const
{
    return 6;
}
