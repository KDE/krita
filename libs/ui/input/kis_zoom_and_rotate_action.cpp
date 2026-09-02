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
#include <kis_algebra_2d.h>

#include "kis_zoom_and_rotate_action.h"
#include "kis_input_manager.h"
#include <KoViewTransformStillPoint.h>

#include "KisCanvasNavigationActionStrategyTouch.h"
#include "KisCanvasNavigationActionStrategyNativeGesture.h"

class KisZoomAndRotateAction::Private {
public:
    Private() {}

    int shortcutIndex {0};
    float lastDistance {0.0};

    KisCanvasNavigationActionStrategy::SnappedRotationData rotationData;

    KoViewTransformStillPoint actionStillPoint;

    std::unique_ptr<KisCanvasNavigationActionStrategy> actionStrategy;

    static bool zoomIsDiscrete(int shortcutIndex) {
        return shortcutIndex == PanAndDiscreteZoomAndRotateMode ||
            shortcutIndex == PanAndDiscreteZoomAndDiscreteRotateMode ||
            shortcutIndex == DiscreteZoomAndRotateMode ||
            shortcutIndex == DiscreteZoomAndDiscreteRotateMode;
    }

    static bool rotationIsDiscrete(int shortcutIndex) {
        return shortcutIndex == PanAndZoomAndDiscreteRotateMode ||
            shortcutIndex == PanAndDiscreteZoomAndDiscreteRotateMode ||
            shortcutIndex == ZoomAndDiscreteRotateMode ||
            shortcutIndex == DiscreteZoomAndDiscreteRotateMode;
    }

    static bool hasPanAction(int shortcutIndex) {
        return shortcutIndex == PanAndZoomAndRotateMode ||
            shortcutIndex == PanAndZoomAndDiscreteRotateMode ||
            shortcutIndex == PanAndDiscreteZoomAndRotateMode ||
            shortcutIndex == PanAndDiscreteZoomAndDiscreteRotateMode;
    }
};

KisZoomAndRotateAction::KisZoomAndRotateAction()
    : KisAbstractInputAction ("Zoom and Rotate Canvas")
    , d(new Private)
{
    setName(i18n("Zoom and Rotate Canvas"));
    QHash<QString, int> shortcuts;
    shortcuts.insert(i18n("Pan, Zoom, Rotate Mode"), PanAndZoomAndRotateMode);
    shortcuts.insert(i18n("Pan, Zoom, Discrete Rotate Mode"), PanAndZoomAndDiscreteRotateMode);
    shortcuts.insert(i18n("Pan, Discrete Zoom, Rotate Mode"), PanAndDiscreteZoomAndRotateMode);
    shortcuts.insert(i18n("Pan, Discrete Zoom, Discrete Rotate Mode"), PanAndDiscreteZoomAndDiscreteRotateMode);
    shortcuts.insert(i18n("Zoom, Rotate Mode"), ZoomAndRotateMode);
    shortcuts.insert(i18n("Zoom, Discrete Rotate Mode"), ZoomAndDiscreteRotateMode);
    shortcuts.insert(i18n("Discrete Zoom, Rotate Mode"), DiscreteZoomAndRotateMode);
    shortcuts.insert(i18n("Discrete Zoom, Discrete Rotate Mode"), DiscreteZoomAndDiscreteRotateMode);
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
    if (!event) return;

    //QTouchEvent *touchEvent = dynamic_cast<QTouchEvent *>(event);

    /*if (touchEvent && touchEvent->touchPoints().size() > 0) {
        d->shortcutIndex = shortcut;
        const QPointF lastPosition = touchEvent->touchPoints().at(0).pos();

        d->lastDistance = 0;

        d->rotationData = {};

        d->actionStillPoint = inputManager()->canvas()->coordinatesConverter()->makeWidgetStillPoint(lastPosition);
    } else*/ if (event->type() == QEvent::NativeGesture ||
                 event->type() == QEvent::TouchBegin ||
                 event->type() == QEvent::TouchUpdate) {

        using Flag = KisCanvasNavigationActionStrategy::Flag;
        using Flags = KisCanvasNavigationActionStrategy::Flags;

        Flags flags;
        flags.setFlag(Flag::PanEnabled, d->hasPanAction(shortcut));
        flags.setFlag(Flag::RotationEnabled);
        flags.setFlag(Flag::RotationDescrete, d->rotationIsDiscrete(shortcut));
        flags.setFlag(Flag::ZoomEnabled);
        flags.setFlag(Flag::ZoomDescrete, d->zoomIsDiscrete(shortcut));

        if (event->type() == QEvent::NativeGesture) {
            d->actionStrategy.reset(new KisCanvasNavigationActionStrategyNativeGesture(flags, eventPosF(event), inputManager()->canvas()));
        } else {
            d->actionStrategy.reset(new KisCanvasNavigationActionStrategyTouch(flags, eventPosF(event), inputManager()->canvas()));
        }

        d->shortcutIndex = shortcut;
    }
}

void KisZoomAndRotateAction::end(QEvent *event)
{
    d->actionStrategy.reset();
    KisAbstractInputAction::end(event);
}

void KisZoomAndRotateAction::cursorMovedAbsolute(const QPointF &, const QPointF &)
{
}


void KisZoomAndRotateAction::inputEvent(QEvent *event)
{
    if (d->actionStrategy && d->actionStrategy->supportsEvent(event)) {
        d->actionStrategy->inputEvent(event);
        return;
    }

    switch (event->type()) {
    case QEvent::TouchUpdate: {
        QTouchEvent *tevent = dynamic_cast<QTouchEvent *>(event);
        if (tevent && tevent->touchPoints().size() > 1) {

            const QPointF p0 = tevent->touchPoints().at(0).pos();
            const QPointF p1 = tevent->touchPoints().at(1).pos();

            const QPointF slope = p1 - p0;
            const qreal currentAngle = std::atan2(slope.y(), slope.x());

            const qreal rotationAngle = canvasRotationAngle(currentAngle);
            const qreal dist = QLineF(p0, p1).length();
            qreal scaleDelta = qFuzzyCompare(1.0, 1.0 + d->lastDistance) ? 1.0 : dist / d->lastDistance;

            // Workaround: only apply the zoom delta if it's not too
            // outlandish. TouchPoint coordinates are not always 100% reliable.

            if(qAbs(scaleDelta) < 0.8 || qAbs(scaleDelta) > 1.2) {
                // just skip the current zoom step
                d->lastDistance = dist;
                scaleDelta = 1.0;
            }

            KisCanvas2 *canvas = inputManager()->canvas();
            KisCanvasController *controller = static_cast<KisCanvasController *>(canvas->canvasController());
            const qreal newZoom = canvas->viewConverter()->zoom() * scaleDelta;
            KoViewTransformStillPoint adjustedStillPoint = d->actionStillPoint;
            adjustedStillPoint.second = p0;
            controller->setZoom(KoZoomMode::ZOOM_CONSTANT, newZoom, adjustedStillPoint);
            controller->rotateCanvas(rotationAngle, adjustedStillPoint);

            return;
        }
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

qreal KisZoomAndRotateAction::canvasRotationAngle(qreal currentAngle)
{
    if (d->rotationIsDiscrete(d->shortcutIndex)) {
        return KisCanvasNavigationActionStrategy::canvasRotationAngleDescrete(currentAngle, d->rotationData);
    } else {
        KisCanvas2 *canvas = inputManager()->canvas();
        KisCanvasController *controller = static_cast<KisCanvasController*>(canvas->canvasController());
        return KisCanvasNavigationActionStrategy::canvasRotationAngleContinuous(currentAngle, controller->rotation(), d->rotationData);
    }
}

