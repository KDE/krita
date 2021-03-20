/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_alternate_invocation_action.h"

#include <QApplication>
#include <klocalizedstring.h>

#include <kis_tool_proxy.h>
#include <kis_canvas2.h>

#include "kis_input_manager.h"
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
    setDescription(i18n("The <i>Alternate Invocation</i> action performs an alternate action with the current tool. For example, using the brush tool it samples a color from the canvas."));
    QHash<QString, int> shortcuts;
    shortcuts.insert(i18n("Primary Mode"), PrimaryAlternateModeShortcut);
    shortcuts.insert(i18n("Secondary Mode"), SecondaryAlternateModeShortcut);
    shortcuts.insert(i18n("Tertiary Mode"), TertiaryAlternateModeShortcut);


    shortcuts.insert(i18n("Sample Foreground Color from Current Layer"), SampleColorFgLayerModeShortcut);
    shortcuts.insert(i18n("Sample Background Color from Current Layer"), SampleColorBgLayerModeShortcut);

    shortcuts.insert(i18n("Sample Foreground Color from Merged Image"), SampleColorFgImageModeShortcut);
    shortcuts.insert(i18n("Sample Background Color from Merged Image"), SampleColorBgImageModeShortcut);

    setShortcutIndexes(shortcuts);
}

KisAlternateInvocationAction::~KisAlternateInvocationAction()
{
}

KisTool::ToolAction KisAlternateInvocationAction::shortcutToToolAction(int shortcut)
{
    KisTool::ToolAction action = KisTool::Alternate_NONE;

    switch ((Shortcut)shortcut) {
    case SampleColorFgLayerModeShortcut:
        action = KisTool::AlternateSampleFgNode;
        break;
    case SampleColorBgLayerModeShortcut:
        action = KisTool::AlternateSampleBgNode;
        break;
    case SampleColorFgImageModeShortcut:
        action = KisTool::AlternateSampleFgImage;
        break;
    case SampleColorBgImageModeShortcut:
        action = KisTool::AlternateSampleBgImage;
        break;
    case PrimaryAlternateModeShortcut:
        action = KisTool::AlternateSecondary;
        break;
    case SecondaryAlternateModeShortcut:
        action = KisTool::AlternateThird;
        break;
    case TertiaryAlternateModeShortcut:
        action = KisTool::AlternateFourth;
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

    QMouseEvent targetEvent(QEvent::MouseButtonPress, eventPosF(event), Qt::LeftButton, Qt::LeftButton, Qt::ControlModifier); // There must be a better way

    m_d->savedAction = shortcutToToolAction(shortcut);

    inputManager()->toolProxy()->forwardEvent(KisToolProxy::BEGIN, m_d->savedAction, &targetEvent, event);
}

void KisAlternateInvocationAction::end(QEvent *event)
{
    if (!event) return;

    Qt::KeyboardModifiers modifiers;

    switch (m_d->savedAction) {
    case KisTool::AlternateSampleFgNode:
        modifiers = Qt::ControlModifier;
        break;
    case KisTool::AlternateThird:
        modifiers = Qt::ControlModifier | Qt::AltModifier;
        break;
    default:
        ;
    }

    QMouseEvent targetEvent = QMouseEvent(QEvent::MouseButtonRelease, eventPosF(event), Qt::LeftButton, Qt::LeftButton, modifiers);
    inputManager()->toolProxy()->forwardEvent(KisToolProxy::END, m_d->savedAction, &targetEvent, event);

    KisAbstractInputAction::end(event);
}

void KisAlternateInvocationAction::inputEvent(QEvent* event)
{
    if (event && ((event->type() == QEvent::MouseMove) || (event->type() == QEvent::TabletMove))) {
        Qt::KeyboardModifiers modifiers;
        switch (m_d->savedAction) {
        case KisTool::AlternateSampleFgNode:
            modifiers =  Qt::ControlModifier;
            break;
        case KisTool::AlternateThird:
            modifiers = Qt::ControlModifier | Qt::AltModifier;
            break;
        default:
            modifiers = Qt::ShiftModifier;
        }

        QMouseEvent targetEvent(QEvent::MouseMove, eventPosF(event), Qt::LeftButton, Qt::LeftButton, modifiers);
        inputManager()->toolProxy()->forwardEvent(KisToolProxy::CONTINUE, m_d->savedAction, &targetEvent, event);
    }
}
