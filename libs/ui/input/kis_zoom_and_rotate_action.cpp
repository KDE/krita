/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2019 Sharaf Zaman <sharafzaz121@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QApplication>
#include <QTouchEvent>

#include <klocalizedstring.h>
#include <kis_canvas_controller.h>
#include <kis_canvas2.h>
#include <KisViewManager.h>
#include <KoZoomController.h>
#include <kis_algebra_2d.h>

#include "kis_zoom_and_rotate_action.h"
#include "kis_input_manager.h"

class KisZoomAndRotateAction::Private {
public:
    Private() {}

    int shortcutIndex {0};
    QPointF lastPosition {0, 0};
    float lastDistance {0.0};
    qreal previousAngle {0.0};
    qreal initialReferenceAngle {0.0};
    qreal accumRotationAngle {0.0};
};

KisZoomAndRotateAction::KisZoomAndRotateAction()
    : KisAbstractInputAction ("Zoom and Rotate Canvas")
    , d(new Private)
{
    setName(i18n("Zoom and Rotate Canvas"));
    QHash<QString, int> shortcuts;
    shortcuts.insert(i18n("Rotate Mode"), ContinuousRotateMode);
    shortcuts.insert(i18n("Discrete Rotate Mode"), DiscreteRotateMode);
    setShortcutIndexes(shortcuts);
}

KisZoomAndRotateAction::~KisZoomAndRotateAction()
{
}

int KisZoomAndRotateAction::priority() const
{
    return 5;
}

void KisZoomAndRotateAction::activate(int shortcut)
{
    Q_UNUSED(shortcut);
}

void KisZoomAndRotateAction::deactivate(int shortcut)
{
    Q_UNUSED(shortcut);
}

void KisZoomAndRotateAction::begin(int shortcut, QEvent *event)
{
    QTouchEvent *touchEvent = dynamic_cast<QTouchEvent *>(event);

    if (touchEvent) {
        d->shortcutIndex = shortcut;
        d->lastPosition = touchEvent->touchPoints().at(0).pos();
        d->lastDistance = 0;
        d->previousAngle = 0;
        d->initialReferenceAngle = 0;
        d->accumRotationAngle = 0;
    }
}

void KisZoomAndRotateAction::cursorMovedAbsolute(const QPointF &, const QPointF &)
{
}

qreal angleForSnapping(qreal angle)
{
    if (angle < 0) {
        return std::fmod(angle - 2, 45) + 2;
    } else {
        return std::fmod(angle + 2, 45) - 2;
    }
}

void KisZoomAndRotateAction::inputEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::TouchUpdate: {
        QTouchEvent *tevent = static_cast<QTouchEvent *>(event);

        const QPointF p0 = tevent->touchPoints().at(0).pos();
        const QPointF p1 = tevent->touchPoints().at(1).pos();

        const qreal rotationAngle = canvasRotationAngle(p0, p1);
        const float dist = QLineF(p0, p1).length();
        const float scaleDelta = qFuzzyCompare(1.0f, 1.0f + d->lastDistance) ? 1.f : dist / d->lastDistance;

        QTransform transform;
        transform.rotate(rotationAngle);
        const QPointF stationaryPointOffset = (p0 - d->lastPosition) * transform * scaleDelta;

        KisCanvas2 *canvas = inputManager()->canvas();
        KisCanvasController *controller = static_cast<KisCanvasController *>(canvas->canvasController());
        controller->zoomRelativeToPoint(p0.toPoint(), scaleDelta);
        controller->rotateCanvas(rotationAngle, p0);
        controller->pan(-stationaryPointOffset.toPoint());

        d->lastPosition = p0;
        d->lastDistance = dist;

        return;
    }
    default:
        break;
    }
    KisAbstractInputAction::inputEvent(event);
}

KisInputActionGroup KisZoomAndRotateAction::inputActionGroup(int shortcut) const
{
    Q_UNUSED(shortcut);
    return ViewTransformActionGroup;
}

qreal KisZoomAndRotateAction::canvasRotationAngle(QPointF p0, QPointF p1)
{
    const QPointF slope = p1 - p0;
    const qreal currentAngle = std::atan2(slope.y(), slope.x());

    switch (d->shortcutIndex) {
    case ContinuousRotateMode: {
        if (!d->previousAngle) {
            d->previousAngle = currentAngle;
            return 0;
        }
        qreal rotationAngle = (180 / M_PI) * (currentAngle - d->previousAngle);
        d->previousAngle = currentAngle;

        KisCanvas2 *canvas = inputManager()->canvas();
        KisCanvasController *controller = static_cast<KisCanvasController *>(canvas->canvasController());
        const qreal canvasAnglePostRotation = controller->rotation() + rotationAngle;
        const qreal snapDelta = angleForSnapping(canvasAnglePostRotation);
        // we snap the canvas to an angle that is a multiple of 45
        if (abs(snapDelta) <= 2 && abs(d->accumRotationAngle) <= 2) {
            // accumulate the relative angle of finger from the point when we started snapping
            d->accumRotationAngle += rotationAngle;
            rotationAngle = rotationAngle - snapDelta;
        } else {
            // snap the canvas out using the accumulated angle
            rotationAngle += d->accumRotationAngle;
            d->accumRotationAngle = 0;
        }

        return rotationAngle;
    }
    case DiscreteRotateMode: {
        if (!d->initialReferenceAngle) {
            d->initialReferenceAngle = currentAngle;
            return 0;
        }
        qreal rotationAngle = 0;
        const qreal relativeAngle = (180 / M_PI) * (currentAngle - d->initialReferenceAngle);
        const qreal rotationThreshold = 15;

        // if the canvas is moved in either direction with an angle greater than the threshold, we rotate the canvas in
        // that direction by 15°.
        if (std::abs(relativeAngle) >= rotationThreshold && std::abs(relativeAngle) <= (360 - rotationThreshold)) {
            // set reference as currentAngle to check if we go beyond the threshold next time
            d->initialReferenceAngle = currentAngle;

            if (std::abs(relativeAngle) <= 180) {
                rotationAngle = KisAlgebra2D::copysign(15.0, relativeAngle);
            } else {
                // if we're over 180, it means the canvas has to be rotated in the opposite direction of the current
                // angle. E.g if the relative angle is +341° then we move the canvas by -15° (because the actual effect
                // is 341 - 360 = -19°  on the original theta).
                rotationAngle = KisAlgebra2D::copysign(15.0, -relativeAngle);
            }
        }
        return rotationAngle;
    }
    default:
        qWarning() << "KisZoomAndRotateAction: Unrecognized shortcut" << d->shortcutIndex;
        return 0;
    }
}
