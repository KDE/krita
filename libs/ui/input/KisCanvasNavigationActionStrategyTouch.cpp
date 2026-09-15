/*
 * SPDX-FileCopyrightText: 2026 Dmitry Kazakov <dimula73@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisCanvasNavigationActionStrategyTouch.h"

#include <QTouchEvent>

#include <kis_algebra_2d.h>
#include <kis_canvas2.h>
#include <kis_canvas_controller.h>

#include <input/kis_touch_shortcut.h>

namespace
{

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    using TouchPoint = QTouchEvent::TouchPoint;
#else
    using TouchPoint = QEventPoint;
#endif

bool testIfPointPressed(const TouchPoint &point) {
    auto state = static_cast<Qt::TouchPointState>(point.state());
    return KisTouchShortcut::pressedOnlyTouchStates().testFlag(state);
}

QPointF calculateBasePointFromEvent(const QTouchEvent *tevent)
{
    QPointF result;

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    auto points = tevent->touchPoints();
    auto pointFunctor = [] (const auto &point) {
        return point.pos();
    };
#else
    auto points = tevent->points();
    auto pointFunctor = [] (const auto &point) {
        return point.position();
    };
#endif

    auto [sum, count] =
        std::accumulate(points.begin(),
                        points.end(),
                        std::pair<QPointF, int>{},
                        [&](std::pair<QPointF, int> result, const TouchPoint &point) {
                            if (!testIfPointPressed(point)) {
                                return result;
                            }
                            result.first += pointFunctor(point);
                            result.second++;

                            return result;
                        });

    if (count > 0) {
        result = sum / qreal(count);
    }

    return result;
}

} // namespace

KisCanvasNavigationActionStrategyTouch::KisCanvasNavigationActionStrategyTouch(Flags flags,
                                                                               const QTouchEvent *startEvent,
                                                                               KisCanvas2 *canvas)
    : KisCanvasNavigationActionStrategy(flags)
    , m_canvas(canvas)
{
    m_actionStillPoint = canvas->coordinatesConverter()->makeWidgetStillPoint(
        calculateBasePointFromEvent(startEvent));
    m_nonRoundedZoom = canvas->viewConverter()->zoom();
}

bool KisCanvasNavigationActionStrategyTouch::supportsEvent(QEvent* event) const
{
    return event->type() == QEvent::TouchBegin || event->type() == QEvent::TouchUpdate
        || event->type() == QEvent::TouchEnd;
}

void KisCanvasNavigationActionStrategyTouch::inputEvent(QEvent* event)
{
    if (event->type() != QEvent::TouchUpdate) return;

    QTouchEvent *tevent = dynamic_cast<QTouchEvent *>(event);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    auto points = tevent->touchPoints();
#else
    auto points = tevent->points();
#endif

    auto point0_it = std::find_if(points.begin(), points.end(), &testIfPointPressed);
    if (point0_it == points.end())
        return;

    auto point1_it = std::find_if(std::next(point0_it), points.end(), &testIfPointPressed);
    if (point1_it == points.end()) {
        if (flags() & (ZoomEnabled | RotationEnabled)) {
            return;
        } else {
            // we are doing pan-only, this action can be connected to
            // a single-finger gesture
            point1_it = point0_it;
        }
    }

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const QPointF p0 = point0_it->pos();
    const QPointF p1 = point1_it->pos();
#else
    const QPointF p0 = point0_it->position();
    const QPointF p1 = point1_it->position();
#endif

    const QPointF slope = p1 - p0;

    KisCanvasController *controller = static_cast<KisCanvasController *>(m_canvas->canvasController());

    KoViewTransformStillPoint adjustedStillPoint = m_actionStillPoint;

    /**
     * Zoom and Rotate actions will perform pan as part of
     * the post-action recentering stage, so we should track
     * if we really need to perform a separate pan action.
     */
    bool needsSeparatePanAction = true;

    if (flags().testFlag(PanEnabled)) {
        adjustedStillPoint.second = calculateBasePointFromEvent(tevent);
    }

    if (flags().testFlag(ZoomEnabled)) {
        const qreal dist = KisAlgebra2D::norm(slope);

        qreal scaleDelta = qFuzzyIsNull(m_lastDistance) ? 1.0 : dist / m_lastDistance;

        // Make sure none of the TouchPoints are too close together, which
        // throws off the zoom calculations. This also addresses a glitch
        // where a newly pressed TouchPoint can incorrectly report another
        // existing TouchPoint's coordinates instead of its own.

        if (dist < 10) {
            scaleDelta = 1.0;
        }

        // Workaround: only apply the zoom delta if it's not too
        // outlandish. TouchPoint coordinates are not always 100% reliable.

        if (qAbs(scaleDelta) < 0.8 || qAbs(scaleDelta) > 1.2) {
            // just skip the current zoom step
            scaleDelta = 1.0;
        }

        const qreal newNonRoundedZoom = m_nonRoundedZoom * scaleDelta;

        if (flags().testFlag(ZoomDescrete)) {
            const KisCoordinatesConverter *converter = m_canvas->coordinatesConverter();
            if (scaleDelta > 1.0) {
                const qreal nextExpectedZoom =
                    converter->findNextZoom(converter->zoom(), converter->standardZoomLevels());
                if (newNonRoundedZoom >= nextExpectedZoom) {
                    controller->setZoom(KoZoomMode::ZOOM_CONSTANT, nextExpectedZoom, adjustedStillPoint);
                    needsSeparatePanAction = false;
                }
            } else if (scaleDelta < 1.0) {
                const qreal prevExpectedZoom =
                    converter->findPrevZoom(converter->zoom(), converter->standardZoomLevels());
                if (newNonRoundedZoom <= prevExpectedZoom) {
                    controller->setZoom(KoZoomMode::ZOOM_CONSTANT, prevExpectedZoom, adjustedStillPoint);
                    needsSeparatePanAction = false;
                }
            }

        } else {
            controller->setZoom(KoZoomMode::ZOOM_CONSTANT, newNonRoundedZoom, adjustedStillPoint);
            needsSeparatePanAction = false;
        }

        m_lastDistance = dist;
        m_nonRoundedZoom = newNonRoundedZoom;
    }

    if (flags().testFlag(RotationEnabled)) {
        const qreal currentAngle = std::atan2(slope.y(), slope.x());
        qreal rotationAngle = 0.0;

        if (flags().testFlag(RotationDescrete)) {
            rotationAngle = canvasRotationAngleDescrete(currentAngle, m_rotationData);
        } else {
            KisCanvasController *controller = static_cast<KisCanvasController *>(m_canvas->canvasController());
            rotationAngle = canvasRotationAngleContinuous(currentAngle, controller->rotation(), m_rotationData);
        }

        if (!qFuzzyIsNull(rotationAngle)) {
            controller->rotateCanvas(rotationAngle, adjustedStillPoint);
            needsSeparatePanAction = false;
        }
    }

    if (needsSeparatePanAction) {
        const KisCoordinatesConverter *converter = m_canvas->coordinatesConverter();
        const QPointF currentStillPointViewPosition = converter->documentToWidget(m_actionStillPoint.docPoint());
        const QPoint canvasOffset((adjustedStillPoint.viewPoint() - currentStillPointViewPosition).toPoint());
        controller->pan(-canvasOffset);
    }
}