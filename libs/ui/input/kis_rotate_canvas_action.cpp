/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_rotate_canvas_action.h"

#include <QApplication>
#include <QNativeGestureEvent>
#include <klocalizedstring.h>

#include "kis_cursor.h"
#include "kis_canvas_controller.h"
#include <kis_canvas2.h>
#include "kis_input_manager.h"
#include <KoViewTransformStillPoint.h>

#include <math.h>

#include "KisCanvasNavigationActionStrategyTouch.h"
#include "KisCanvasNavigationActionStrategyNativeGesture.h"

constexpr qreal DISCRETE_ANGLE_STEP = 15.0;  // discrete rotation snapping angle

class KisRotateCanvasAction::Private
{
public:
    Private() {}

    // Coverity requires sane defaults for all variables (CID 36429)
    Shortcut mode {RotateModeShortcut};

    qreal snapRotation {0.0};
    bool allowRotation {false};

    std::unique_ptr<KisCanvasNavigationActionStrategy> actionStrategy;
};


KisRotateCanvasAction::KisRotateCanvasAction()
    : KisAbstractInputAction("Rotate Canvas")
    , d(new Private())
{
    setName(i18n("Rotate Canvas"));
    setDescription(i18n("The <i>Rotate Canvas</i> action rotates the canvas."));

    QHash<QString, int> shortcuts;
    shortcuts.insert(i18n("Rotate Mode"), RotateModeShortcut);
    shortcuts.insert(i18n("Discrete Rotate Mode"), DiscreteRotateModeShortcut);
    shortcuts.insert(i18n("Rotate Left"), RotateLeftShortcut);
    shortcuts.insert(i18n("Rotate Right"), RotateRightShortcut);
    shortcuts.insert(i18n("Reset Rotation"), RotateResetShortcut);
    setShortcutIndexes(shortcuts);
}

KisRotateCanvasAction::~KisRotateCanvasAction()
{
    delete d;
}

int KisRotateCanvasAction::priority() const
{
    return 3;
}

void KisRotateCanvasAction::activate(int shortcut)
{
    if (shortcut == DiscreteRotateModeShortcut) {
        QApplication::setOverrideCursor(KisCursor::rotateCanvasDiscreteCursor());
    } else /* if (shortcut == SmoothRotateModeShortcut) */ {
        QApplication::setOverrideCursor(KisCursor::rotateCanvasSmoothCursor());
    }
}

void KisRotateCanvasAction::deactivate(int shortcut)
{
    Q_UNUSED(shortcut);
    QApplication::restoreOverrideCursor();
}

void KisRotateCanvasAction::begin(int shortcut, QEvent *event)
{
    KisAbstractInputAction::begin(shortcut, event);

    KisCanvasController *canvasController =
        dynamic_cast<KisCanvasController*>(inputManager()->canvas()->canvasController());
    KIS_SAFE_ASSERT_RECOVER_RETURN(canvasController);

    if (event
        && (event->type() == QEvent::NativeGesture || event->type() == QEvent::TouchBegin
            || event->type() == QEvent::TouchUpdate)
        && (shortcut == RotateModeShortcut || shortcut == DiscreteRotateModeShortcut)) {

        using Flag = KisCanvasNavigationActionStrategyNativeGesture::Flag;
        using Flags = KisCanvasNavigationActionStrategyNativeGesture::Flags;

        Flags flags;
        flags.setFlag(Flag::PanEnabled);
        flags.setFlag(Flag::RotationEnabled);
        flags.setFlag(Flag::RotationDescrete, shortcut == DiscreteRotateModeShortcut);

        if (event->type() == QEvent::NativeGesture) {
            d->actionStrategy.reset(new KisCanvasNavigationActionStrategyNativeGesture(flags, eventPosF(event), inputManager()->canvas()));
        } else {
            const QTouchEvent *tevent = static_cast<const QTouchEvent*>(event);
            d->actionStrategy.reset(new KisCanvasNavigationActionStrategyTouch(flags, tevent, inputManager()->canvas()));
        }

        // native gestures don't have cursor tracking by the OS, so they shouldn't show any cursor
        QApplication::restoreOverrideCursor();
        return;
    }

    d->allowRotation = false;
    d->snapRotation = 0;

    d->mode = (Shortcut)shortcut;

    switch(shortcut) {
        case RotateModeShortcut:
        case DiscreteRotateModeShortcut: {
            // If the canvas has been rotated to an angle that is not an exact multiple of DISCRETE_ANGLE_STEP,
            // we need to adjust the final discrete rotation by that angle difference.
            // trunc() is used to round the negative numbers towards zero.
            const qreal startRotation = inputManager()->canvas()->rotationAngle();
            d->snapRotation = startRotation - std::trunc(startRotation / DISCRETE_ANGLE_STEP) * DISCRETE_ANGLE_STEP;
            canvasController->beginCanvasRotation();
            break;
        }
        case RotateLeftShortcut:
            canvasController->rotateCanvasLeft15();
            break;
        case RotateRightShortcut:
            canvasController->rotateCanvasRight15();
            break;
        case RotateResetShortcut:
            canvasController->resetCanvasRotation();
            break;
    }
}

void KisRotateCanvasAction::end(QEvent *event)
{
    Q_UNUSED(event);

    if (d->actionStrategy) {
        d->actionStrategy.reset();
    } else {
        KisCanvasController *canvasController =
            dynamic_cast<KisCanvasController *>(inputManager()->canvas()->canvasController());
        KIS_SAFE_ASSERT_RECOVER_RETURN(canvasController);

        switch (d->mode) {
        case RotateModeShortcut:
        case DiscreteRotateModeShortcut:
            canvasController->endCanvasRotation();
            break;
        default:
            break;
        }
    }
}

void KisRotateCanvasAction::cursorMovedAbsolute(const QPointF &startPos, const QPointF &pos)
{
    if (d->mode == RotateResetShortcut) {
        return;
    }

    const KisCoordinatesConverter *converter = inputManager()->canvas()->coordinatesConverter();
    const QPointF centerPoint = converter->flakeToWidget(converter->flakeCenterPoint());
    const QPointF startPoint = startPos - centerPoint;
    const QPointF newPoint = pos - centerPoint;

    const qreal oldAngle = atan2(startPoint.y(), startPoint.x());
    const qreal newAngle = atan2(newPoint.y(), newPoint.x());

    qreal newRotation = (180 / M_PI) * (newAngle - oldAngle);

    if (d->mode == DiscreteRotateModeShortcut) {
        // Do not snap unless the user rotated half-way in the desired direction.
        if (qAbs(newRotation) > 0.5 * DISCRETE_ANGLE_STEP || d->allowRotation) {
            d->allowRotation = true;
            newRotation = qRound((newRotation + d->snapRotation) / DISCRETE_ANGLE_STEP) * DISCRETE_ANGLE_STEP - d->snapRotation;
        } else {
            newRotation = 0.0;
        }
    }

    KisCanvasController *canvasController =
        dynamic_cast<KisCanvasController*>(inputManager()->canvas()->canvasController());
    KIS_SAFE_ASSERT_RECOVER_RETURN(canvasController);
    canvasController->rotateCanvas(newRotation);
}

void KisRotateCanvasAction::inputEvent(QEvent* event)
{
    if(!event) {
        return;
    }

    if (d->actionStrategy && d->actionStrategy->supportsEvent(event)) {
        d->actionStrategy->inputEvent(event);
    } else {
        KisAbstractInputAction::inputEvent(event);
    }
}

KisInputActionGroup KisRotateCanvasAction::inputActionGroup(int shortcut) const
{
    Q_UNUSED(shortcut);
    return ViewTransformActionGroup;
}

bool KisRotateCanvasAction::supportsHiResInputEvents(int shortcut) const
{
    Q_UNUSED(shortcut);
    return true;
}
