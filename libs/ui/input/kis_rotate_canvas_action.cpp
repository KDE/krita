/* This file is part of the KDE project
 * Copyright (C) 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
    Private() : angleDrift(0), previousAngle(0) {}

    Shortcut mode;
    qreal angleDrift;

    qreal previousAngle;
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
            d->mode = (Shortcut)shortcut;
            break;
        case DiscreteRotateModeShortcut:
            d->mode = (Shortcut)shortcut;
            d->angleDrift = 0;
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

void KisRotateCanvasAction::cursorMoved(const QPointF &lastPos, const QPointF &pos)
{
    const KisCoordinatesConverter *converter = inputManager()->canvas()->coordinatesConverter();
    QPointF centerPoint = converter->flakeToWidget(converter->flakeCenterPoint());
    QPointF oldPoint = lastPos - centerPoint;
    QPointF newPoint = pos - centerPoint;

    qreal oldAngle = atan2(oldPoint.y(), oldPoint.x());
    qreal newAngle = atan2(newPoint.y(), newPoint.x());

    qreal angle = (180 / M_PI) * (newAngle - oldAngle);

    if (d->mode == DiscreteRotateModeShortcut) {
        const qreal angleStep = 15;
        qreal initialAngle = inputManager()->canvas()->rotationAngle();
        qreal roundedAngle = qRound((initialAngle + angle + d->angleDrift) / angleStep) * angleStep - initialAngle;
        d->angleDrift += angle - roundedAngle;
        angle = roundedAngle;
    }

    KisCanvasController *canvasController =
        dynamic_cast<KisCanvasController*>(inputManager()->canvas()->canvasController());
    canvasController->rotateCanvas(angle);
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

            QPointF p0 = touchEvent->touchPoints().at(0).pos();
            QPointF p1 = touchEvent->touchPoints().at(1).pos();

            // high school (y2 - y1) / (x2 - x1)
            QPointF slope = p1 - p0;
            qreal newAngle = atan2(slope.y(), slope.x());

            if (!d->previousAngle)
            {
                d->previousAngle = newAngle;
                return;
            }

            qreal delta = (180 / M_PI) * (newAngle - d->previousAngle);

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
