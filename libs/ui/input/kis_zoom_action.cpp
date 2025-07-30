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

#include <kis_canvas2.h>
#include <kis_canvas_controller.h>
#include <KoViewTransformStillPoint.h>
#include "kis_cursor.h"
#include "KisViewManager.h"
#include "kis_input_manager.h"
#include "kis_config.h"


class KisZoomAction::Private
{
public:
    Private(KisZoomAction *qq) : q(qq), lastDistance(0.f) {}

    QPointF centerPoint(QTouchEvent* event);

    KisZoomAction *q {nullptr};
    // Coverity requires sane defaults for all variables (CID 36380)
    Shortcuts mode {ZoomModeShortcut};

    QPointF lastPosition;
    float lastDistance {0.0};

    KoViewTransformStillPoint actionStillPoint;

    qreal startZoom {1.0};
    qreal lastDiscreteZoomDistance {0.0};
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
    shortcuts.insert(i18n("Zoom In To Cursor"), ZoomInToCursorShortcut);
    shortcuts.insert(i18n("Zoom Out From Cursor"), ZoomOutFromCursorShortcut);
    shortcuts.insert(i18n("Zoom to 100%"), Zoom100PctShortcut);
    shortcuts.insert(i18n("Fit to View"), FitToViewShortcut);
    shortcuts.insert(i18n("Fit to View Width"), FitToWidthShortcut);
    shortcuts.insert(i18n("Fit to View Height"), FitToHeightShortcut);
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
            d->startZoom = inputManager()->canvas()->coordinatesConverter()->zoom();
            d->mode = (Shortcuts)shortcut;
            d->lastPosition = QPoint();
            d->actionStillPoint = inputManager()->canvas()->coordinatesConverter()->makeViewStillPoint(eventPosF(event));
            break;
        }
        case DiscreteZoomModeShortcut:
        case RelativeDiscreteZoomModeShortcut:
            d->startZoom = inputManager()->canvas()->coordinatesConverter()->zoom();
            d->lastDiscreteZoomDistance = 0;
            d->mode = (Shortcuts)shortcut;
            d->actionStillPoint = inputManager()->canvas()->coordinatesConverter()->makeViewStillPoint(eventPosF(event));
            break;
        case ZoomInShortcut:
        case ZoomOutShortcut:
        case ZoomInToCursorShortcut:
        case ZoomOutFromCursorShortcut: {
            KoCanvasControllerWidget *controller =
                dynamic_cast<KoCanvasControllerWidget *>(inputManager()->canvas()->canvasController());
            KIS_SAFE_ASSERT_RECOVER_RETURN(controller);

            QPoint pt;
            if (shortcut == ZoomInToCursorShortcut || shortcut == ZoomOutFromCursorShortcut) {
                pt = eventPos(event);
                if (pt.isNull()) {
                    pt = controller->mapFromGlobal(QCursor::pos());
                }
            }

            if (shortcut == ZoomInToCursorShortcut || shortcut == ZoomInShortcut) {
                if (pt.isNull()) {
                    controller->zoomIn();
                } else {
                    controller->zoomIn(inputManager()->canvas()->coordinatesConverter()->makeViewStillPoint(pt));
                }
            } else {
                if (pt.isNull()) {
                    controller->zoomOut();
                } else {
                    controller->zoomOut(inputManager()->canvas()->coordinatesConverter()->makeViewStillPoint(pt));
                }
            }
            break;
        }
        case Zoom100PctShortcut: {
            KoCanvasControllerWidget *controller =
                dynamic_cast<KoCanvasControllerWidget *>(inputManager()->canvas()->canvasController());
            controller->setZoom(KoZoomMode::ZOOM_CONSTANT, 1.0);
            break;
        }
        case FitToViewShortcut: {
            KoCanvasControllerWidget *controller =
                dynamic_cast<KoCanvasControllerWidget *>(inputManager()->canvas()->canvasController());
            controller->setZoom(KoZoomMode::ZOOM_PAGE, 1.0);
            break;
        }
        case FitToWidthShortcut: {
            KoCanvasControllerWidget *controller =
                dynamic_cast<KoCanvasControllerWidget *>(inputManager()->canvas()->canvasController());
            controller->setZoom(KoZoomMode::ZOOM_WIDTH, 1.0);
            break;
        }
        case FitToHeightShortcut: {
            KoCanvasControllerWidget *controller =
                dynamic_cast<KoCanvasControllerWidget *>(inputManager()->canvas()->canvasController());
            controller->setZoom(KoZoomMode::ZOOM_HEIGHT, 1.0);
            break;
        }
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
            const qreal newZoom = controller->canvas()->viewConverter()->zoom() * delta;
            KoViewTransformStillPoint adjustedStillPoint = d->actionStillPoint;
            adjustedStillPoint.second = p0;
            controller->setZoom(KoZoomMode::ZOOM_CONSTANT, newZoom, adjustedStillPoint);
            d->lastDistance = dist;
            d->lastPosition = p0;
            return;  // Don't try to update the cursor during a pinch-zoom
        }
        case QEvent::NativeGesture: {
            QNativeGestureEvent *gevent = static_cast<QNativeGestureEvent*>(event);
            if (gevent->gestureType() == Qt::ZoomNativeGesture) {
                KisCanvas2 *canvas = inputManager()->canvas();
                KisCanvasController *controller = static_cast<KisCanvasController*>(canvas->canvasController());
                const qreal delta = 1.0f + gevent->value();
                const qreal newZoom = controller->canvas()->viewConverter()->zoom() * delta;
                controller->setZoom(KoZoomMode::ZOOM_CONSTANT, newZoom, d->actionStillPoint);
            } else if (gevent->gestureType() == Qt::SmartZoomNativeGesture) {
                KisCanvas2 *canvas = inputManager()->canvas();
                KoCanvasController *controller = canvas->canvasController();

                if (controller->zoomState().mode != KoZoomMode::ZOOM_WIDTH) {
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

        KisConfig cfg(true);

        const qreal logDistance = std::pow(2.0, qreal(cfg.zoomHorizontal() ? -diff.x() : diff.y()) / qreal(stepCont));

        qreal newZoom = 1.0;
        if (cfg.readEntry<bool>("InvertMiddleClickZoom", false)) {
            newZoom = d->startZoom / logDistance;
        } else {
            newZoom = d->startZoom * logDistance;
        }

        KoCanvasControllerWidget *controller =
            dynamic_cast<KoCanvasControllerWidget *>(inputManager()->canvas()->canvasController());
        KIS_SAFE_ASSERT_RECOVER_RETURN(controller);

        if (d->mode == ZoomModeShortcut) {
            controller->setZoom(KoZoomMode::ZOOM_CONSTANT, newZoom);
        } else {
            controller->setZoom(KoZoomMode::ZOOM_CONSTANT, newZoom, d->actionStillPoint);
        }

    } else if (d->mode == DiscreteZoomModeShortcut ||
               d->mode == RelativeDiscreteZoomModeShortcut) {

        KoCanvasControllerWidget *controller =
            dynamic_cast<KoCanvasControllerWidget *>(inputManager()->canvas()->canvasController());
        KIS_SAFE_ASSERT_RECOVER_RETURN(controller);

        KisConfig cfg(true);

        qreal axisDiff = qreal(cfg.zoomHorizontal() ? -diff.x() : diff.y());
        qreal currentDiff = axisDiff / stepDisc - d->lastDiscreteZoomDistance;

        const bool zoomIn = currentDiff > 0;
        while (qAbs(currentDiff) > 1.0) {
            if (zoomIn) {
                if (d->mode == RelativeDiscreteZoomModeShortcut) {
                    controller->zoomIn(d->actionStillPoint);
                } else {
                    controller->zoomIn();
                }
            } else {
                if (d->mode == RelativeDiscreteZoomModeShortcut) {
                    controller->zoomOut(d->actionStillPoint);
                } else {
                    controller->zoomOut();
                }
            }
            d->lastDiscreteZoomDistance += zoomIn ? 1.0 : -1.0;
            currentDiff = axisDiff / stepDisc - d->lastDiscreteZoomDistance;
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

