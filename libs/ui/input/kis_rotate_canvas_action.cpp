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

constexpr qreal DISCRETE_ANGLE_STEP = 15.0;  // discrete rotation snapping angle

class KisRotateCanvasAction::Private
{
public:
    Private() {}

    // Coverity requires sane defaults for all variables (CID 36429)
    Shortcut mode {RotateModeShortcut};

    qreal previousAngle {0.0};
    qreal snapRotation {0.0};
    qreal touchRotation {0.0};
    bool allowRotation {false};
    KoViewTransformStillPoint actionStillPoint;
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
    d->allowRotation = false;
    d->previousAngle = 0;
    d->snapRotation = 0;
    d->touchRotation = 0;

    KisCanvasController *canvasController =
        dynamic_cast<KisCanvasController*>(inputManager()->canvas()->canvasController());
    KIS_SAFE_ASSERT_RECOVER_RETURN(canvasController);

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
            d->actionStillPoint = inputManager()->canvas()->coordinatesConverter()->makeWidgetStillPoint(eventPosF(event));
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

    KisCanvasController *canvasController =
        dynamic_cast<KisCanvasController*>(inputManager()->canvas()->canvasController());
    KIS_SAFE_ASSERT_RECOVER_RETURN(canvasController);

    switch(d->mode) {
    case RotateModeShortcut:
    case DiscreteRotateModeShortcut:
        canvasController->endCanvasRotation();
        break;
    default:
        break;
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
    switch (event->type()) {
        case QEvent::NativeGesture: {
            QNativeGestureEvent *gevent = static_cast<QNativeGestureEvent*>(event);
            KisCanvas2 *canvas = inputManager()->canvas();
            KisCanvasController *controller = static_cast<KisCanvasController*>(canvas->canvasController());

            const float angle = gevent->value();
            QPoint widgetPos = canvas->canvasWidget()->mapFromGlobal(gevent->globalPos());

            KoViewTransformStillPoint adjustedStillPoint = d->actionStillPoint;
            adjustedStillPoint.second = widgetPos;

            controller->rotateCanvas(angle, adjustedStillPoint, true);
            return;
        }
        case QEvent::TouchUpdate: {
            QTouchEvent *touchEvent = static_cast<QTouchEvent*>(event);

            if (touchEvent->touchPoints().count() != 2)
                break;

            QTouchEvent::TouchPoint tp0 = touchEvent->touchPoints().at(0);
            QTouchEvent::TouchPoint tp1 = touchEvent->touchPoints().at(1);

            if (tp0.state() == Qt::TouchPointReleased ||
                tp1.state() == Qt::TouchPointReleased)
            {
                // Workaround: on some devices, the coordinates of TouchPoints
                // in state TouchPointReleased are not reliable, and can
                // "jump" by a significant distance. So we just stop handling
                // the rotation as soon as the user's finger leaves the tablet.
                break;
            }

            QPointF p0 = tp0.pos();
            QPointF p1 = tp1.pos();

            if ((p0-p1).manhattanLength() < 10)
            {
                // The TouchPoints are too close together. Don't update the
                // rotation as the angle will likely be off. This also deals
                // with a glitch where a newly pressed TouchPoint incorrectly
                // reports the existing TouchPoint's coordinates instead of its
                // own.
                break;
            }

            // high school (y2 - y1) / (x2 - x1)
            QPointF slope = p1 - p0;
            qreal newAngle = atan2(slope.y(), slope.x());

            // We must have the previous angle measurement to calculate the delta.
            if (d->allowRotation)
            {
                qreal delta = (180 / M_PI) * (newAngle - d->previousAngle);

                // Rotate by the effective angle from the beginning of the action.
                d->touchRotation += delta;

                KisCanvas2 *canvas = inputManager()->canvas();
                KisCanvasController *controller = static_cast<KisCanvasController*>(canvas->canvasController());
                controller->rotateCanvas(d->touchRotation);
            }
            else
            {
                d->allowRotation = true;
            }

            d->previousAngle = newAngle;
            return;
        }
        default:
            break;
    }
    KisAbstractInputAction::inputEvent(event);
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
