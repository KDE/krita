/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 Arjen Hiemstra <ahiemstra@heimr.nl>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
    Private(KisZoomAction *qq) : q(qq), lastDistance(0.f) {}

    QPointF centerPoint(QTouchEvent* event);

    KisZoomAction *q;
    Shortcuts mode;

    QPointF lastPosition;
    float lastDistance;

    qreal startZoom = 1.0;
    qreal lastDescreteZoomDistance = 0.0;

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
    shortcuts.insert(i18n("Fit to Height"), ZoomToHeightShortcut);
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
            d->startZoom = inputManager()->canvas()->viewManager()->zoomController()->zoomAction()->effectiveZoom();
            d->mode = (Shortcuts)shortcut;
            d->lastPosition = QPoint();
            break;
        }
        case DiscreteZoomModeShortcut:
        case RelativeDiscreteZoomModeShortcut:
            d->startZoom = inputManager()->canvas()->viewManager()->zoomController()->zoomAction()->effectiveZoom();
            d->lastDescreteZoomDistance = 0;
            d->mode = (Shortcuts)shortcut;
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
        case ZoomToHeightShortcut:
            inputManager()->canvas()->viewManager()->zoomController()->setZoom(KoZoomMode::ZOOM_HEIGHT, 1.0);
            break;
    }
}

void KisZoomAction::inputEvent( QEvent* event )
{
    switch (event->type()) {
        case QEvent::TouchUpdate: {
            QTouchEvent *tevent = static_cast<QTouchEvent*>(event);

            if (tevent->touchPoints().count() != 2) {
                // Sanity check. The input state machine should only invoke
                // this action if there are 2 TouchPoints in the event.
                return;
            }

            // First, let's determine if we want to handle this event. Sadly
            // the coordinates of TouchPoints reported by Qt are not always
            // dependable. TouchPoints that are just getting released can be
            // off by a significant amount. So we stop the zoom as soon as the
            // user lifts a finger.

            QTouchEvent::TouchPoint tp0 = tevent->touchPoints().at(0);
            QTouchEvent::TouchPoint tp1 = tevent->touchPoints().at(1);
            if (tp0.state() == Qt::TouchPointReleased ||
                    tp1.state() == Qt::TouchPointReleased) {
                // Force a recomputation of the position on the next event.
                d->lastPosition = QPoint();
                return;
            }

            QPointF p0 = tp0.pos();
            QPointF p1 = tp1.pos();

            // Make sure none of the TouchPoints are too close together, which
            // throws off the zoom calculations. This also addresses a glitch
            // where a newly pressed TouchPoint can incorrectly report another
            // existing TouchPoint's coordinates instead of its own.

            if ((p0-p1).manhattanLength() < 10) {
                d->lastPosition = QPointF();
                return;
            }

            // If this is the first valid set of points that we are getting,
            // then use that as the reference for the zoom.

            if (d->lastPosition.isNull()) {
                d->lastPosition = p0;
                d->lastDistance = 0;
                return;
            }

            float dist = QLineF(p0, p1).length();
            float delta = qFuzzyCompare(1.0f, 1.0f + d->lastDistance) ? 1.f : dist / d->lastDistance;

            // Workaround: only apply the zoom delta if it's not too
            // outlandish. As explained above, TouchPoint coordinates are not
            // always 100% reliable.

            if(qAbs(delta) < 0.8f || qAbs(delta) > 1.2f) {
                // TouchPoint coordinates tend to converge toward correct
                // values over time, so assume that the new position is
                // likelier to be correct than the last and use that as the new
                // reference.
                d->lastPosition = p0;
                return;
            }

            KisCanvasController *controller = static_cast<KisCanvasController *>(inputManager()->canvas()->canvasController());
            controller->zoomRelativeToPoint(p0.toPoint(), delta);
            d->lastDistance = dist;

            QPointF moveDelta = (p0 - d->lastPosition) * delta;
            controller->pan(-moveDelta.toPoint());
            d->lastPosition = p0;
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

void KisZoomAction::cursorMovedAbsolute(const QPointF &startPos, const QPointF &pos)
{
    QPointF diff = -(pos - startPos);

    const int stepCont = 100;
    const int stepDisc = 50;

    if (d->mode == ZoomModeShortcut ||
        d->mode == RelativeZoomModeShortcut) {

        const qreal zoom = inputManager()->canvas()->viewManager()->zoomController()->zoomAction()->effectiveZoom();
        const qreal logDistance = std::pow(2.0, qreal(diff.y()) / qreal(stepCont));

        KisConfig cfg(true);
        qreal newZoom = zoom;
        if (cfg.readEntry<bool>("InvertMiddleClickZoom", false)) {
            newZoom = d->startZoom / logDistance;
        } else {
            newZoom = d->startZoom * logDistance;
        }

        if (d->mode == ZoomModeShortcut) {
            inputManager()->canvas()->viewManager()->zoomController()->setZoom(KoZoomMode::ZOOM_CONSTANT, newZoom);
        } else {
            const qreal coeff = newZoom / zoom;

            KoCanvasControllerWidget *controller =
                dynamic_cast<KoCanvasControllerWidget*>(
                    inputManager()->canvas()->canvasController());

            controller->zoomRelativeToPoint(startPos.toPoint(), coeff);
        }

    } else if (d->mode == DiscreteZoomModeShortcut ||
               d->mode == RelativeDiscreteZoomModeShortcut) {

        QPoint stillPoint = d->mode == RelativeDiscreteZoomModeShortcut ?
            startPos.toPoint() : QPoint();

        qreal currentDiff = qreal(diff.y()) / stepDisc - d->lastDescreteZoomDistance;

        bool zoomIn = currentDiff > 0;
        while (qAbs(currentDiff) > 1.0) {
            d->zoomTo(zoomIn, stillPoint);
            d->lastDescreteZoomDistance += zoomIn ? 1.0 : -1.0;
            currentDiff = qreal(diff.y()) / stepDisc - d->lastDescreteZoomDistance;
        }
    }
}

bool KisZoomAction::isShortcutRequired(int shortcut) const
{
    return shortcut == ZoomModeShortcut;
}

bool KisZoomAction::supportsHiResInputEvents(int shortcut) const
{
    Q_UNUSED(shortcut);
    return true;
}

KisInputActionGroup KisZoomAction::inputActionGroup(int shortcut) const
{
    Q_UNUSED(shortcut);
    return ViewTransformActionGroup;
}

