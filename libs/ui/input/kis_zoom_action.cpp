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

#include "kis_zoom_action.h"

#include <QApplication>
#include <QNativeGestureEvent>

#include <klocalizedstring.h>

#include <KoCanvasControllerWidget.h>
#include <KoZoomController.h>

#include <kis_canvas2.h>
#include <kis_canvas_controller.h>
#include "kis_cursor.h"
#include "KisViewManager.h"
#include "kis_input_manager.h"
#include "kis_config.h"


inline QPoint pointFromEvent(QEvent *event) {
    if (!event) {
        return QPoint();
    } else if (QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent*>(event)) {
        return mouseEvent->pos();
    } else if (QTabletEvent *tabletEvent = dynamic_cast<QTabletEvent*>(event)) {
        return tabletEvent->pos();
    } else if (QWheelEvent *wheelEvent = dynamic_cast<QWheelEvent*>(event)) {
        return wheelEvent->pos();
    }

    return QPoint();
}


class KisZoomAction::Private
{
public:
    Private(KisZoomAction *qq) : q(qq), distance(0), lastDistance(0.f) {}

    QPointF centerPoint(QTouchEvent* event);

    KisZoomAction *q;
    int distance;
    Shortcuts mode;

    QPointF lastPosition;
    float lastDistance;

    QPoint startPoint;

    void zoomTo(bool zoomIn, const QPoint &pos);
};

QPointF KisZoomAction::Private::centerPoint(QTouchEvent* event)
{
    QPointF result;
    int count = 0;

    Q_FOREACH (QTouchEvent::TouchPoint point, event->touchPoints()) {
        if (point.state() != Qt::TouchPointReleased) {
            result += point.pos();
            count++;
        }
    }

    if (count > 0) {
        return result / count;
    } else {
        return QPointF();
    }
}

void KisZoomAction::Private::zoomTo(bool zoomIn, const QPoint &point)
{
    KoZoomAction *zoomAction = q->inputManager()->canvas()->viewManager()->zoomController()->zoomAction();

    if (!point.isNull()) {
        float oldZoom = zoomAction->effectiveZoom();
        float newZoom = zoomIn ?
            zoomAction->nextZoomLevel() : zoomAction->prevZoomLevel();

        KoCanvasControllerWidget *controller =
            dynamic_cast<KoCanvasControllerWidget*>(
                q->inputManager()->canvas()->canvasController());

        controller->zoomRelativeToPoint(point, newZoom / oldZoom);
    } else {
        if (zoomIn) {
            zoomAction->zoomIn();
        } else {
            zoomAction->zoomOut();
        }
    }
}

KisZoomAction::KisZoomAction()
    : KisAbstractInputAction("Zoom Canvas")
    , d(new Private(this))
{
    setName(i18n("Zoom Canvas"));
    setDescription(i18n("The <i>Zoom Canvas</i> action zooms the canvas."));

    QHash< QString, int > shortcuts;
    shortcuts.insert(i18n("Zoom Mode"), ZoomModeShortcut);
    shortcuts.insert(i18n("Discrete Zoom Mode"), DiscreteZoomModeShortcut);
    shortcuts.insert(i18n("Relative Zoom Mode"), RelativeZoomModeShortcut);
    shortcuts.insert(i18n("Relative Discrete Zoom Mode"), RelativeDiscreteZoomModeShortcut);
    shortcuts.insert(i18n("Zoom In"), ZoomInShortcut);
    shortcuts.insert(i18n("Zoom Out"), ZoomOutShortcut);
    shortcuts.insert(i18n("Reset Zoom to 100%"), ZoomResetShortcut);
    shortcuts.insert(i18n("Fit to Page"), ZoomToPageShortcut);
    shortcuts.insert(i18n("Fit to Width"), ZoomToWidthShortcut);
    setShortcutIndexes(shortcuts);
}

KisZoomAction::~KisZoomAction()
{
    delete d;
}

int KisZoomAction::priority() const
{
    return 4;
}

void KisZoomAction::activate(int shortcut)
{
    if (shortcut == DiscreteZoomModeShortcut ||
        shortcut == RelativeDiscreteZoomModeShortcut) {
        QApplication::setOverrideCursor(KisCursor::zoomDiscreteCursor());
    } else /* if (shortcut == SmoothZoomModeShortcut) */ {
        QApplication::setOverrideCursor(KisCursor::zoomSmoothCursor());
    }
}

void KisZoomAction::deactivate(int shortcut)
{
    Q_UNUSED(shortcut);
    QApplication::restoreOverrideCursor();
}

void KisZoomAction::begin(int shortcut, QEvent *event)
{
    KisAbstractInputAction::begin(shortcut, event);

    d->lastDistance = 0.f;

    switch(shortcut) {
        case ZoomModeShortcut:
        case RelativeZoomModeShortcut: {
            d->startPoint = pointFromEvent(event);
            d->mode = (Shortcuts)shortcut;
            QTouchEvent *tevent = dynamic_cast<QTouchEvent*>(event);
            if(tevent)
                d->lastPosition = d->centerPoint(tevent);
            break;
        }
        case DiscreteZoomModeShortcut:
        case RelativeDiscreteZoomModeShortcut:
            d->startPoint = pointFromEvent(event);
            d->mode = (Shortcuts)shortcut;
            d->distance = 0;
            break;
        case ZoomInShortcut:
            d->zoomTo(true, pointFromEvent(event));
            break;
        case ZoomOutShortcut:
            d->zoomTo(false, pointFromEvent(event));
            break;
        case ZoomResetShortcut:
            inputManager()->canvas()->viewManager()->zoomController()->setZoom(KoZoomMode::ZOOM_CONSTANT, 1.0);
            break;
        case ZoomToPageShortcut:
            inputManager()->canvas()->viewManager()->zoomController()->setZoom(KoZoomMode::ZOOM_PAGE, 1.0);
            break;
        case ZoomToWidthShortcut:
            inputManager()->canvas()->viewManager()->zoomController()->setZoom(KoZoomMode::ZOOM_WIDTH, 1.0);
            break;
    }
}

void KisZoomAction::inputEvent( QEvent* event )
{
    switch (event->type()) {
        case QEvent::TouchUpdate: {
            QTouchEvent *tevent = static_cast<QTouchEvent*>(event);
            QPointF center = d->centerPoint(tevent);

            int count = 0;
            float dist = 0.0f;
            Q_FOREACH (const QTouchEvent::TouchPoint &point, tevent->touchPoints()) {
                if (point.state() != Qt::TouchPointReleased) {
                    count++;

                    dist += (point.pos() - center).manhattanLength();
                }
            }

            dist /= count;
            float delta = qFuzzyCompare(1.0f, 1.0f + d->lastDistance) ? 1.f : dist / d->lastDistance;

            if(qAbs(delta) > 0.1f) {
                qreal zoom = inputManager()->canvas()->viewManager()->zoomController()->zoomAction()->effectiveZoom();
                Q_UNUSED(zoom);
                static_cast<KisCanvasController*>(inputManager()->canvas()->canvasController())->zoomRelativeToPoint(center.toPoint(), delta);
                d->lastDistance = dist;
                // Also do panning here, as doing it later requires a further check for validity
                QPointF moveDelta = center - d->lastPosition;
                inputManager()->canvas()->canvasController()->pan(-moveDelta.toPoint());
                d->lastPosition = center;
            }
            return;  // Don't try to update the cursor during a pinch-zoom
        }
        case QEvent::NativeGesture: {
            QNativeGestureEvent *gevent = static_cast<QNativeGestureEvent*>(event);
            if (gevent->gestureType() == Qt::ZoomNativeGesture) {
                KisCanvas2 *canvas = inputManager()->canvas();
                KisCanvasController *controller = static_cast<KisCanvasController*>(canvas->canvasController());
                const float delta = 1.0f + gevent->value();
                controller->zoomRelativeToPoint(canvas->canvasWidget()->mapFromGlobal(gevent->globalPos()), delta);
            } else if (gevent->gestureType() == Qt::SmartZoomNativeGesture) {
                KisCanvas2 *canvas = inputManager()->canvas();
                KoZoomController *controller = canvas->viewManager()->zoomController();

                if (controller->zoomMode() != KoZoomMode::ZOOM_WIDTH) {
                    controller->setZoom(KoZoomMode::ZOOM_WIDTH, 1.0);
                } else {
                    controller->setZoom(KoZoomMode::ZOOM_CONSTANT, 1.0);
                }
            }
            return;
        }
        default:
            break;
    }
    KisAbstractInputAction::inputEvent(event);
}

void KisZoomAction::cursorMoved(const QPointF &lastPos, const QPointF &pos)
{
    QPointF diff = -(pos - lastPos);

    const int stepCont = 100;
    const int stepDisc = 20;

    if (d->mode == ZoomModeShortcut ||
        d->mode == RelativeZoomModeShortcut) {
        KisConfig cfg(true);
        float coeff;
        if (cfg.readEntry<bool>("InvertMiddleClickZoom", false)) {
            coeff = 1.0 - qreal(diff.y()) / stepCont;
        }
        else {
            coeff = 1.0 + qreal(diff.y()) / stepCont;
        }

        if (d->mode == ZoomModeShortcut) {
            float zoom = coeff * inputManager()->canvas()->viewManager()->zoomController()->zoomAction()->effectiveZoom();
            inputManager()->canvas()->viewManager()->zoomController()->setZoom(KoZoomMode::ZOOM_CONSTANT, zoom);
        } else {
            KoCanvasControllerWidget *controller =
                dynamic_cast<KoCanvasControllerWidget*>(
                    inputManager()->canvas()->canvasController());

            controller->zoomRelativeToPoint(d->startPoint, coeff);
        }

    } else if (d->mode == DiscreteZoomModeShortcut ||
               d->mode == RelativeDiscreteZoomModeShortcut) {
        d->distance += diff.y();

        QPoint stillPoint = d->mode == RelativeDiscreteZoomModeShortcut ?
            d->startPoint : QPoint();

        bool zoomIn = d->distance > 0;
        while (qAbs(d->distance) > stepDisc) {
            d->zoomTo(zoomIn, stillPoint);
            d->distance += zoomIn ? -stepDisc : stepDisc;
        }
    }
}

bool KisZoomAction::isShortcutRequired(int shortcut) const
{
    return shortcut == ZoomModeShortcut;
}

bool KisZoomAction::supportsHiResInputEvents() const
{
    return true;
}

KisInputActionGroup KisZoomAction::inputActionGroup(int shortcut) const
{
    Q_UNUSED(shortcut);
    return ViewTransformActionGroup;
}

