/*
 * This file is part of the KDE project
 * SPDX-FileCopyrightText: 2019 Sharaf Zaman <sharafzaz121@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <QApplication>
#include <QTouchEvent>

#include <klocalizedstring.h>
#include <kis_config.h>
#include <kis_canvas_controller.h>
#include <kis_canvas2.h>
#include <KisViewManager.h>
#include <KoZoomController.h>

#include "kis_zoom_and_rotate_action.h"
#include "kis_input_manager.h"

class KisZoomAndRotateAction::Private {
public:
    Private() {}

    QPointF lastPosition;
    float lastDistance;
    qreal previousAngle;
};

KisZoomAndRotateAction::KisZoomAndRotateAction()
    : KisAbstractInputAction ("Zoom and Rotate Canvas")
    , d(new Private)
{
    setName(i18n("Zoom and Rotate Canvas"));
}

KisZoomAndRotateAction::~KisZoomAndRotateAction()
{
}

int KisZoomAndRotateAction::priority() const
{
    // if rotation is disabled, we set the lowest priority, so that, this action isn't triggered
    return KisConfig(true).disableTouchRotation() ? 0 : 5;
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
    Q_UNUSED(shortcut);
    QTouchEvent *touchEvent = dynamic_cast<QTouchEvent *>(event);

    if (touchEvent) {
        d->lastPosition = touchEvent->touchPoints().at(0).pos();
        d->lastDistance = 0;
        d->previousAngle = 0;
    }
}

void KisZoomAndRotateAction::cursorMovedAbsolute(const QPointF &, const QPointF &)
{
}

void KisZoomAndRotateAction::inputEvent(QEvent *event)
{
    switch (event->type()) {
    case QEvent::TouchUpdate: {
        QTouchEvent *tevent = static_cast<QTouchEvent *>(event);

        QTouchEvent::TouchPoint tp0 = tevent->touchPoints().at(0);
        QTouchEvent::TouchPoint tp1 = tevent->touchPoints().at(1);

        QPointF p0 = tp0.pos();
        QPointF p1 = tp1.pos();

        float dist = QLineF(p0, p1).length();
        float scaleDelta = qFuzzyCompare(1.0f, 1.0f + d->lastDistance) ? 1.f : dist / d->lastDistance;

        QPointF slope = p1 - p0;
        qreal newAngle = atan2(slope.y(), slope.x());
        qreal rotationAngle = (180 / M_PI) * (newAngle - d->previousAngle);
        if (!d->previousAngle) {
            d->previousAngle = newAngle;
            return;
        }

        KisCanvas2 *canvas = inputManager()->canvas();
        KisCanvasController *controller =
            static_cast<KisCanvasController *>(canvas->canvasController());

        QTransform transform;
        transform.rotate(rotationAngle);
        QPointF stationaryPointOffset = (p0 - d->lastPosition) * transform * scaleDelta;

        controller->zoomRelativeToPoint(p0.toPoint(), scaleDelta);
        controller->rotateCanvas(rotationAngle, p0);
        controller->pan(-stationaryPointOffset.toPoint());

        d->lastPosition = p0;
        d->lastDistance = dist;
        d->previousAngle = newAngle;

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
