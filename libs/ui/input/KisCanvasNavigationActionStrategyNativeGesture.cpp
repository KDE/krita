/*
 * SPDX-FileCopyrightText: 2026 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisCanvasNavigationActionStrategyNativeGesture.h"

#include <QNativeGestureEvent>

#include <kis_canvas2.h>
#include <kis_canvas_controller.h>

KisCanvasNavigationActionStrategyNativeGesture::KisCanvasNavigationActionStrategyNativeGesture(Flags flags, const QPointF &startViewPos, KisCanvas2 *canvas)
    : m_canvas(canvas)
    , m_flags(flags)
{
    m_actionStillPoint = canvas->coordinatesConverter()->makeWidgetStillPoint(startViewPos);
    m_nonRoundedZoom = canvas->viewConverter()->zoom();
}

bool KisCanvasNavigationActionStrategyNativeGesture::supportsEvent(QEvent* event) const
{
    return event->type() == QEvent::NativeGesture;
}

void KisCanvasNavigationActionStrategyNativeGesture::inputEvent(QEvent *event)
{
    QNativeGestureEvent *gevent = static_cast<QNativeGestureEvent *>(event);
    if (gevent->gestureType() == Qt::SmartZoomNativeGesture) {
        KoCanvasController *controller = m_canvas->canvasController();

        if (controller->zoomState().mode != KoZoomMode::ZOOM_WIDTH) {
            controller->setZoom(KoZoomMode::ZOOM_WIDTH, 1.0);
        } else {
            controller->setZoom(KoZoomMode::ZOOM_CONSTANT, 1.0);
        }
    } else if (gevent->gestureType() == Qt::ZoomNativeGesture && m_flags.testFlag(ZoomEnabled)) {
        KisCanvasController *controller = static_cast<KisCanvasController *>(m_canvas->canvasController());
        const KisCoordinatesConverter *converter = m_canvas->coordinatesConverter();

        const qreal delta = 1.0 + gevent->value();

        // Workaround: only apply the zoom delta if it's not too
        // outlandish. TouchPoint coordinates are not always 100% reliable.
        if (qAbs(delta) > 1.2 || qAbs(delta) < 0.8) {
            // just skip the current zoom step
            return;
        }

        const qreal newZoom = m_nonRoundedZoom * delta;

        if (m_flags.testFlag(ZoomDescrete)) {
            if (delta > 1.0) {
                const qreal nextExpectedZoom =
                    converter->findNextZoom(converter->zoom(), converter->standardZoomLevels());
                if (newZoom >= nextExpectedZoom) {
                    controller->setZoom(KoZoomMode::ZOOM_CONSTANT, nextExpectedZoom, m_actionStillPoint);
                }
            } else if (delta < 1.0) {
                const qreal prevExpectedZoom =
                    converter->findPrevZoom(converter->zoom(), converter->standardZoomLevels());
                if (newZoom <= prevExpectedZoom) {
                    controller->setZoom(KoZoomMode::ZOOM_CONSTANT, prevExpectedZoom, m_actionStillPoint);
                }
            }

        } else {
            controller->setZoom(KoZoomMode::ZOOM_CONSTANT, newZoom, m_actionStillPoint);
        }

        m_nonRoundedZoom = converter->clampZoom(newZoom);

    } else if (gevent->gestureType() == Qt::RotateNativeGesture && m_flags.testFlag(RotationEnabled)) {
        KisCanvasController *controller = static_cast<KisCanvasController *>(m_canvas->canvasController());

        m_nonRoundedRelativeRotation += gevent->value();

        const qreal rotationAngle = [&]() {
            if (m_flags.testFlag(RotationDescrete)) {
                return canvasRotationAngleDescrete(kisDegreesToRadians(m_nonRoundedRelativeRotation),
                                                   m_rotationData);
            } else {
                return canvasRotationAngleContinuous(kisDegreesToRadians(m_nonRoundedRelativeRotation),
                                                     controller->rotation(),
                                                     m_rotationData);
            }
        }();

        if (!qFuzzyIsNull(rotationAngle)) {
            controller->rotateCanvas(rotationAngle, m_actionStillPoint, true);
        }
    }
    if (gevent->gestureType() == Qt::PanNativeGesture && m_flags.testFlag(PanEnabled)) {
        KisCanvasController *controller = static_cast<KisCanvasController *>(m_canvas->canvasController());

        m_accumulatedPan += gevent->delta();

        // we take the integral part of the accumulated offset and pan
        // using it; the fractional part is left in the accumulator for
        // the following pan actions
        const QPoint canvasOffset(std::trunc(m_accumulatedPan.x()),
                                  std::trunc(m_accumulatedPan.y()));
        m_accumulatedPan -= canvasOffset;

        // do the pan and adjust the still point so that zoom and rotate actions
        // could also take that into account
        m_actionStillPoint.second += canvasOffset;
        controller->pan(-canvasOffset);
    }
}