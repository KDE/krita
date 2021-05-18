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

#include <math.h>

class KisRotateCanvasAction::Private
{
public:
    Private() : previousAngle(0) {}

    Shortcut mode;

    qreal previousAngle;
    qreal startRotation;
    qreal previousRotation;
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
    d->previousAngle = 0;

    KisCanvasController *canvasController =
        dynamic_cast<KisCanvasController*>(inputManager()->canvas()->canvasController());

    switch(shortcut) {
        case RotateModeShortcut:
        case DiscreteRotateModeShortcut:
            d->mode = (Shortcut)shortcut;
            d->startRotation = inputManager()->canvas()->rotationAngle();
            d->previousRotation = 0;
            break;
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

void KisRotateCanvasAction::cursorMovedAbsolute(const QPointF &startPos, const QPointF &pos)
{
    const KisCoordinatesConverter *converter = inputManager()->canvas()->coordinatesConverter();
    const QPointF centerPoint = converter->flakeToWidget(converter->flakeCenterPoint());
    const QPointF startPoint = startPos - centerPoint;
    const QPointF newPoint = pos - centerPoint;

    const qreal oldAngle = atan2(startPoint.y(), startPoint.x());
    const qreal newAngle = atan2(newPoint.y(), newPoint.x());

    qreal newRotation = (180 / M_PI) * (newAngle - oldAngle);

    if (d->mode == DiscreteRotateModeShortcut) {
        const qreal angleStep = 15;

        // avoid jumps at the beginning of the rotation action
        if (qAbs(newRotation) > 0.5 * angleStep) {
            const qreal currentCanvasRotation = converter->rotationAngle();
            const qreal desiredOffset = newRotation - d->previousRotation;
            newRotation = qRound((currentCanvasRotation + desiredOffset) / angleStep) * angleStep - currentCanvasRotation + d->previousRotation;
        } else {
            newRotation = d->previousRotation;
        }
    }

    KisCanvasController *canvasController =
        dynamic_cast<KisCanvasController*>(inputManager()->canvas()->canvasController());
    canvasController->rotateCanvas(newRotation - d->previousRotation);
    d->previousRotation = newRotation;
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
            controller->rotateCanvas(angle, widgetPos);
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
            qreal delta = (180 / M_PI) * (newAngle - d->previousAngle);

            // Workaround: So, TouchPoint coordinates are not 100% reliable.
            // Freshly pressed TouchPoints are sometimes not accurate for
            // multiple events. So if the computed rotation angle is out of
            // whack, we just ignore it: it probably means the previous
            // position was off.
            if (qAbs(delta) > 15) {
                // TouchPoint coordinates tend to converge toward correct
                // values, so assume the previous angle was off but this one is
                // better and should be the new basis for the next event.
                d->previousAngle = newAngle;
                break;
            }

            KisCanvas2 *canvas = inputManager()->canvas();
            KisCanvasController *controller = static_cast<KisCanvasController*>(canvas->canvasController());
            controller->rotateCanvas(delta);

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
