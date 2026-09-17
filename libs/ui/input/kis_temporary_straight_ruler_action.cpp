/*
 *  SPDX-FileCopyrightText: 2026 Ayanami Kaine <personal@ayanamikaine.com>
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_temporary_straight_ruler_action.h"

#include <optional>

#include <QHash>

#include <klocalizedstring.h>

#include <kis_canvas2.h>
#include <kis_painting_assistants_decoration.h>
#include <kis_temporary_paint_constraint.h>

#include "kis_input_manager.h"
#include "kis_tool_invocation_action.h"

class KisTemporaryStraightRulerAction::Private
{
public:
    static KisTemporaryPaintConstraint::InteractionMode interactionMode(int shortcut)
    {
        switch (shortcut) {
        case SnapToAngleShortcut:
            return KisTemporaryPaintConstraint::InteractionMode::SnapToAngle;
        case NormalShortcut:
        default:
            return KisTemporaryPaintConstraint::InteractionMode::Normal;
        }
    }

    KisCanvas2 *canvas(KisInputManager *inputManager) const
    {
        if (!inputManager) return nullptr;
        return inputManager->canvas();
    }

    KisPaintingAssistantsDecorationSP decoration(KisInputManager *inputManager) const
    {
        KisCanvas2 *c = canvas(inputManager);
        if (!c) return KisPaintingAssistantsDecorationSP();
        return c->paintingAssistantsDecoration();
    }

    KisToolInvocationAction *defaultInputAction(KisInputManager *inputManager) const
    {
        return inputManager ? inputManager->defaultInputAction() : nullptr;
    }
};

KisTemporaryStraightRulerAction::KisTemporaryStraightRulerAction()
    : KisAbstractInputAction("Temporary Straight Ruler")
    , d(new Private)
{
    setName(i18n("Temporary Straight Ruler"));
    setDescription(i18n("The <i>Temporary Straight Ruler</i> action creates a transient ruler for constrained painting."));

    QHash<QString, int> shortcuts;
    shortcuts.insert(i18n("Normal"), NormalShortcut);
    shortcuts.insert(i18n("Snapped Angle"), SnapToAngleShortcut);
    setShortcutIndexes(shortcuts);
}

KisTemporaryStraightRulerAction::~KisTemporaryStraightRulerAction()
{
    delete d;
}

int KisTemporaryStraightRulerAction::priority() const
{
    return 100;
}

void KisTemporaryStraightRulerAction::activate(int shortcut)
{
    KisPaintingAssistantsDecorationSP decoration = d->decoration(inputManager());
    if (decoration) {
        KisTemporaryPaintConstraint constraint =
            decoration->temporaryConstraint().value_or(KisTemporaryPaintConstraint());
        constraint.setInteractionMode(Private::interactionMode(shortcut));
        decoration->setTemporaryConstraint(constraint);
    }
}

void KisTemporaryStraightRulerAction::deactivate(int shortcut)
{
    Q_UNUSED(shortcut);

    KisPaintingAssistantsDecorationSP decoration = d->decoration(inputManager());
    if (decoration) {
        decoration->setTemporaryConstraint(std::nullopt);
    }
}

bool KisTemporaryStraightRulerAction::trySwitchShortcut(int oldShortcut, int newShortcut)
{
    Q_UNUSED(oldShortcut);

    KisPaintingAssistantsDecorationSP decoration = d->decoration(inputManager());
    if (!decoration) {
        return false;
    }

    std::optional<KisTemporaryPaintConstraint> constraint = decoration->temporaryConstraint();
    if (!constraint) {
        return false;
    }

    constraint->setInteractionMode(Private::interactionMode(newShortcut));
    decoration->setTemporaryConstraint(constraint);
    return true;
}

void KisTemporaryStraightRulerAction::begin(int shortcut, QEvent *event)
{
    Q_UNUSED(shortcut);

    if (KisToolInvocationAction *action = d->defaultInputAction(inputManager())) {
        action->begin(KisToolInvocationAction::ActivateShortcut, event);
    }
}

void KisTemporaryStraightRulerAction::inputEvent(QEvent *event)
{
    if (KisToolInvocationAction *action = d->defaultInputAction(inputManager())) {
        action->inputEvent(event);
    }
}

void KisTemporaryStraightRulerAction::end(QEvent *event)
{
    if (KisToolInvocationAction *action = d->defaultInputAction(inputManager())) {
        action->end(event);
    }
}

bool KisTemporaryStraightRulerAction::supportsHiResInputEvents(int shortcut) const
{
    Q_UNUSED(shortcut);

    KisToolInvocationAction *action = d->defaultInputAction(inputManager());
    return action ? action->supportsHiResInputEvents(KisToolInvocationAction::ActivateShortcut) : true;
}

bool KisTemporaryStraightRulerAction::isAvailable() const
{
    return true;
}

KisInputActionGroup KisTemporaryStraightRulerAction::inputActionGroup(int shortcut) const
{
    Q_UNUSED(shortcut);
    return ToolInvoactionActionGroup;
}
