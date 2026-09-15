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

#include "KisCanvasNavigationActionStrategyTouch.h"
#include "KisCanvasNavigationActionStrategyNativeGesture.h"


class KisZoomAction::Private
{
public:
    Private(KisZoomAction *qq) : q(qq) {}

    KisZoomAction *q {nullptr};
    // Coverity requires sane defaults for all variables (CID 36380)
    Shortcuts mode {ZoomModeShortcut};

    KoViewTransformStillPoint actionStillPoint;

    qreal startZoom {1.0};
    qreal lastDiscreteZoomDistance {0.0};

    std::unique_ptr<KisCanvasNavigationActionStrategy> actionStrategy;
};

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

    /**
     * Firstly, try to handle native gestures and touch events
     */
    if (event
        && (event->type() == QEvent::NativeGesture || event->type() == QEvent::TouchBegin
            || event->type() == QEvent::TouchUpdate)
        && (shortcut == ZoomModeShortcut || shortcut == RelativeZoomModeShortcut || shortcut == DiscreteZoomModeShortcut
            || shortcut == RelativeDiscreteZoomModeShortcut)) {
        using Flag = KisCanvasNavigationActionStrategyNativeGesture::Flag;
        using Flags = KisCanvasNavigationActionStrategyNativeGesture::Flags;

        Flags flags;
        flags.setFlag(Flag::ZoomEnabled);
        flags.setFlag(Flag::ZoomDescrete,
                      shortcut == DiscreteZoomModeShortcut || shortcut == RelativeDiscreteZoomModeShortcut);

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

    switch(shortcut) {
        case ZoomModeShortcut:
        case RelativeZoomModeShortcut: {
            d->startZoom = inputManager()->canvas()->coordinatesConverter()->zoom();
            d->mode = (Shortcuts)shortcut;
            d->actionStillPoint = inputManager()->canvas()->coordinatesConverter()->makeWidgetStillPoint(eventPosF(event));
            break;
        }
        case DiscreteZoomModeShortcut:
        case RelativeDiscreteZoomModeShortcut:
            d->startZoom = inputManager()->canvas()->coordinatesConverter()->zoom();
            d->lastDiscreteZoomDistance = 0;
            d->mode = (Shortcuts)shortcut;
            d->actionStillPoint = inputManager()->canvas()->coordinatesConverter()->makeWidgetStillPoint(eventPosF(event));
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
                    controller->zoomIn(inputManager()->canvas()->coordinatesConverter()->makeWidgetStillPoint(pt));
                }
            } else {
                if (pt.isNull()) {
                    controller->zoomOut();
                } else {
                    controller->zoomOut(inputManager()->canvas()->coordinatesConverter()->makeWidgetStillPoint(pt));
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

void KisZoomAction::end(QEvent *event)
{
    d->actionStrategy.reset();
    KisAbstractInputAction::end(event);
}

void KisZoomAction::inputEvent( QEvent* event )
{
    if(!event) {
        return;
    }

    if (d->actionStrategy && d->actionStrategy->supportsEvent(event)) {
        d->actionStrategy->inputEvent(event);
        return;
    } else {
        KisAbstractInputAction::inputEvent(event);
    }
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

