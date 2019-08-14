/*
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_tool_freehand_helper.h"

#include <QTimer>
#include <QQueue>

#include <klocalizedstring.h>

#include <KoPointerEvent.h>
#include <KoCanvasResourceProvider.h>

#include "kis_algebra_2d.h"
#include "kis_distance_information.h"
#include "kis_painting_information_builder.h"
#include "kis_image.h"
#include "kis_painter.h"
#include <brushengine/kis_paintop_preset.h>
#include <brushengine/kis_paintop_utils.h>

#include "kis_update_time_monitor.h"
#include "kis_stabilized_events_sampler.h"
#include "KisStabilizerDelayedPaintHelper.h"
#include "kis_config.h"

#include "kis_random_source.h"
#include "KisPerStrokeRandomSource.h"

#include "strokes/freehand_stroke.h"
#include "strokes/KisFreehandStrokeInfo.h"
#include "KisAsyncronousStrokeUpdateHelper.h"

#include <math.h>

//#define DEBUG_BEZIER_CURVES

// Factor by which to scale the airbrush timer's interval, relative to the actual airbrushing rate.
// Setting this less than 1 makes the timer-generated pseudo-events happen faster than the desired
// airbrush rate, which can improve responsiveness.
const qreal AIRBRUSH_INTERVAL_FACTOR = 0.5;

// The amount of time, in milliseconds, to allow between updates of the spacing information. Only
// used when spacing updates between dabs are enabled.
const qreal SPACING_UPDATE_INTERVAL = 50.0;

// The amount of time, in milliseconds, to allow between updates of the timing information. Only
// used when airbrushing.
const qreal TIMING_UPDATE_INTERVAL = 50.0;

struct KisToolFreehandHelper::Private
{
    KisPaintingInformationBuilder *infoBuilder;
    KisStrokesFacade *strokesFacade;
    KisAsyncronousStrokeUpdateHelper asyncUpdateHelper;

    KUndo2MagicString transactionText;

    bool haveTangent;
    QPointF previousTangent;

    bool hasPaintAtLeastOnce;

    QTime strokeTime;
    QTimer strokeTimeoutTimer;

    QVector<KisFreehandStrokeInfo*> strokeInfos;
    KisResourcesSnapshotSP resources;
    KisStrokeId strokeId;

    KisPaintInformation previousPaintInformation;
    KisPaintInformation olderPaintInformation;

    KisSmoothingOptionsSP smoothingOptions;

    // fake random sources for hovering outline *only*
    KisRandomSourceSP fakeDabRandomSource;
    KisPerStrokeRandomSourceSP fakeStrokeRandomSource;

    // Timer used to generate paint updates periodically even without input events. This is only
    // used for paintops that depend on timely updates even when the cursor is not moving, e.g. for
    // airbrushing effects.
    QTimer airbrushingTimer;

    QList<KisPaintInformation> history;
    QList<qreal> distanceHistory;

    // Keeps track of past cursor positions. This is used to determine the drawing angle when
    // drawing the brush outline or starting a stroke.
    KisPaintOpUtils::PositionHistory lastCursorPos;

    // Stabilizer data
    bool usingStabilizer;
    QQueue<KisPaintInformation> stabilizerDeque;
    QTimer stabilizerPollTimer;
    KisStabilizedEventsSampler stabilizedSampler;
    KisStabilizerDelayedPaintHelper stabilizerDelayedPaintHelper;

    qreal effectiveSmoothnessDistance() const;
};


KisToolFreehandHelper::KisToolFreehandHelper(KisPaintingInformationBuilder *infoBuilder,
                                             const KUndo2MagicString &transactionText,
                                             KisSmoothingOptions *smoothingOptions)
    : m_d(new Private())
{
    m_d->infoBuilder = infoBuilder;
    m_d->transactionText = transactionText;
    m_d->smoothingOptions = KisSmoothingOptionsSP(
                smoothingOptions ? smoothingOptions : new KisSmoothingOptions());

    m_d->fakeDabRandomSource = new KisRandomSource();
    m_d->fakeStrokeRandomSource = new KisPerStrokeRandomSource();

    m_d->strokeTimeoutTimer.setSingleShot(true);
    connect(&m_d->strokeTimeoutTimer, SIGNAL(timeout()), SLOT(finishStroke()));
    connect(&m_d->airbrushingTimer, SIGNAL(timeout()), SLOT(doAirbrushing()));
    connect(&m_d->stabilizerPollTimer, SIGNAL(timeout()), SLOT(stabilizerPollAndPaint()));
    connect(m_d->smoothingOptions.data(), SIGNAL(sigSmoothingTypeChanged()), SLOT(slotSmoothingTypeChanged()));

    m_d->stabilizerDelayedPaintHelper.setPaintLineCallback(
                [this](const KisPaintInformation &pi1, const KisPaintInformation &pi2) {
                    paintLine(pi1, pi2);
                });
    m_d->stabilizerDelayedPaintHelper.setUpdateOutlineCallback(
                [this]() {
                    emit requestExplicitUpdateOutline();
                });
}

KisToolFreehandHelper::~KisToolFreehandHelper()
{
    delete m_d;
}

void KisToolFreehandHelper::setSmoothness(KisSmoothingOptionsSP smoothingOptions)
{
    m_d->smoothingOptions = smoothingOptions;
}

KisSmoothingOptionsSP KisToolFreehandHelper::smoothingOptions() const
{
    return m_d->smoothingOptions;
}

QPainterPath KisToolFreehandHelper::paintOpOutline(const QPointF &savedCursorPos,
                                                   const KoPointerEvent *event,
                                                   const KisPaintOpSettingsSP globalSettings,
                                                   KisPaintOpSettings::OutlineMode mode) const
{
    KisPaintOpSettingsSP settings = globalSettings;
    KisPaintInformation info = m_d->infoBuilder->hover(savedCursorPos, event);
    QPointF prevPoint = m_d->lastCursorPos.pushThroughHistory(savedCursorPos);
    qreal startAngle = KisAlgebra2D::directionBetweenPoints(prevPoint, savedCursorPos, 0);
    KisDistanceInformation distanceInfo(prevPoint, startAngle);

    if (!m_d->strokeInfos.isEmpty()) {
        settings = m_d->resources->currentPaintOpPreset()->settings();
        if (m_d->stabilizerDelayedPaintHelper.running() &&
                m_d->stabilizerDelayedPaintHelper.hasLastPaintInformation()) {
            info = m_d->stabilizerDelayedPaintHelper.lastPaintInformation();
        } else {
            info = m_d->previousPaintInformation;
        }

        /**
         * When LoD mode is active it may happen that the helper has
         * already started a stroke, but it painted noting, because
         * all the work is being calculated by the scaled-down LodN
         * stroke. So at first we try to fetch the data from the lodN
         * stroke ("buddy") and then check if there is at least
         * something has been painted with this distance information
         * object.
         */
        KisDistanceInformation *buddyDistance =
            m_d->strokeInfos.first()->buddyDragDistance();

        if (buddyDistance) {
            /**
             * Tiny hack alert: here we fetch the distance information
             * directly from the LodN stroke. Ideally, we should
             * upscale its data, but here we just override it with our
             * local copy of the coordinates.
             */
            distanceInfo = *buddyDistance;
            distanceInfo.overrideLastValues(prevPoint, startAngle);

        } else if (m_d->strokeInfos.first()->dragDistance->isStarted()) {
            distanceInfo = *m_d->strokeInfos.first()->dragDistance;
        }
    }

    KisPaintInformation::DistanceInformationRegistrar registrar =
        info.registerDistanceInformation(&distanceInfo);

    info.setRandomSource(m_d->fakeDabRandomSource);
    info.setPerStrokeRandomSource(m_d->fakeStrokeRandomSource);

    QPainterPath outline = settings->brushOutline(info, mode);

    if (m_d->resources &&
        m_d->smoothingOptions->smoothingType() == KisSmoothingOptions::STABILIZER &&
        m_d->smoothingOptions->useDelayDistance()) {

        const qreal R = m_d->smoothingOptions->delayDistance() /
            m_d->resources->effectiveZoom();

        outline.addEllipse(info.pos(), R, R);
    }

    return outline;
}

void KisToolFreehandHelper::cursorMoved(const QPointF &cursorPos)
{
    m_d->lastCursorPos.pushThroughHistory(cursorPos);
}

void KisToolFreehandHelper::initPaint(KoPointerEvent *event,
                                      const QPointF &pixelCoords,
                                      KoCanvasResourceProvider *resourceManager,
                                      KisImageWSP image, KisNodeSP currentNode,
                                      KisStrokesFacade *strokesFacade,
                                      KisNodeSP overrideNode,
                                      KisDefaultBoundsBaseSP bounds)
{
    QPointF prevPoint = m_d->lastCursorPos.pushThroughHistory(pixelCoords);
    m_d->strokeTime.start();
    KisPaintInformation pi =
        m_d->infoBuilder->startStroke(event, elapsedStrokeTime(), resourceManager);
    qreal startAngle = KisAlgebra2D::directionBetweenPoints(prevPoint, pixelCoords, 0.0);

    initPaintImpl(startAngle,
                  pi,
                  resourceManager,
                  image,
                  currentNode,
                  strokesFacade,
                  overrideNode,
                  bounds);
}

bool KisToolFreehandHelper::isRunning() const
{
    return m_d->strokeId;
}

void KisToolFreehandHelper::initPaintImpl(qreal startAngle,
                                          const KisPaintInformation &pi,
                                          KoCanvasResourceProvider *resourceManager,
                                          KisImageWSP image,
                                          KisNodeSP currentNode,
                                          KisStrokesFacade *strokesFacade,
                                          KisNodeSP overrideNode,
                                          KisDefaultBoundsBaseSP bounds)
{
    m_d->strokesFacade = strokesFacade;

    m_d->haveTangent = false;
    m_d->previousTangent = QPointF();

    m_d->hasPaintAtLeastOnce = false;

    m_d->previousPaintInformation = pi;

    m_d->resources = new KisResourcesSnapshot(image,
                                              currentNode,
                                              resourceManager,
                                              bounds);
    if(overrideNode) {
        m_d->resources->setCurrentNode(overrideNode);
    }

    const bool airbrushing = m_d->resources->needsAirbrushing();
    const bool useSpacingUpdates = m_d->resources->needsSpacingUpdates();

    KisDistanceInitInfo startDistInfo(m_d->previousPaintInformation.pos(),
                                      startAngle,
                                      useSpacingUpdates ? SPACING_UPDATE_INTERVAL : LONG_TIME,
                                      airbrushing ? TIMING_UPDATE_INTERVAL : LONG_TIME,
                                      0);
    KisDistanceInformation startDist = startDistInfo.makeDistInfo();

    createPainters(m_d->strokeInfos,
                   startDist);

    KisStrokeStrategy *stroke =
        new FreehandStrokeStrategy(m_d->resources, m_d->strokeInfos, m_d->transactionText);

    m_d->strokeId = m_d->strokesFacade->startStroke(stroke);

    m_d->history.clear();
    m_d->distanceHistory.clear();

    if (airbrushing) {
        m_d->airbrushingTimer.setInterval(computeAirbrushTimerInterval());
        m_d->airbrushingTimer.start();
    } else if (m_d->resources->presetNeedsAsynchronousUpdates()) {
        m_d->asyncUpdateHelper.startUpdateStream(m_d->strokesFacade, m_d->strokeId);
    }

    if (m_d->smoothingOptions->smoothingType() == KisSmoothingOptions::STABILIZER) {
        stabilizerStart(m_d->previousPaintInformation);
    }

    // If airbrushing, paint an initial dab immediately. This is a workaround for an issue where
    // some paintops (Dyna, Particle, Sketch) might never initialize their spacing/timing
    // information until paintAt is called.
    if (airbrushing) {
        paintAt(pi);
    }
}

void KisToolFreehandHelper::paintBezierSegment(KisPaintInformation pi1, KisPaintInformation pi2,
                                               QPointF tangent1, QPointF tangent2)
{
    if (tangent1.isNull() || tangent2.isNull()) return;

    const qreal maxSanePoint = 1e6;

    QPointF controlTarget1;
    QPointF controlTarget2;

    // Shows the direction in which control points go
    QPointF controlDirection1 = pi1.pos() + tangent1;
    QPointF controlDirection2 = pi2.pos() - tangent2;

    // Lines in the direction of the control points
    QLineF line1(pi1.pos(), controlDirection1);
    QLineF line2(pi2.pos(), controlDirection2);

    // Lines to check whether the control points lay on the opposite
    // side of the line
    QLineF line3(controlDirection1, controlDirection2);
    QLineF line4(pi1.pos(), pi2.pos());

    QPointF intersection;
    if (line3.intersect(line4, &intersection) == QLineF::BoundedIntersection) {
        qreal controlLength = line4.length() / 2;

        line1.setLength(controlLength);
        line2.setLength(controlLength);

        controlTarget1 = line1.p2();
        controlTarget2 = line2.p2();
    } else {
        QLineF::IntersectType type = line1.intersect(line2, &intersection);

        if (type == QLineF::NoIntersection ||
            intersection.manhattanLength() > maxSanePoint) {

            intersection = 0.5 * (pi1.pos() + pi2.pos());
//            dbgKrita << "WARNING: there is no intersection point "
//                     << "in the basic smoothing algorithms";
        }

        controlTarget1 = intersection;
        controlTarget2 = intersection;
    }

    // shows how near to the controlTarget the value raises
    qreal coeff = 0.8;

    qreal velocity1 = QLineF(QPointF(), tangent1).length();
    qreal velocity2 = QLineF(QPointF(), tangent2).length();

    if (velocity1 == 0.0 || velocity2 == 0.0) {
        velocity1 = 1e-6;
        velocity2 = 1e-6;
        warnKrita << "WARNING: Basic Smoothing: Velocity is Zero! Please report a bug:" << ppVar(velocity1) << ppVar(velocity2);
    }

    qreal similarity = qMin(velocity1/velocity2, velocity2/velocity1);

    // the controls should not differ more than 50%
    similarity = qMax(similarity, qreal(0.5));

    // when the controls are symmetric, their size should be smaller
    // to avoid corner-like curves
    coeff *= 1 - qMax(qreal(0.0), similarity - qreal(0.8));

    Q_ASSERT(coeff > 0);


    QPointF control1;
    QPointF control2;

    if (velocity1 > velocity2) {
        control1 = pi1.pos() * (1.0 - coeff) + coeff * controlTarget1;
        coeff *= similarity;
        control2 = pi2.pos() * (1.0 - coeff) + coeff * controlTarget2;
    } else {
        control2 = pi2.pos() * (1.0 - coeff) + coeff * controlTarget2;
        coeff *= similarity;
        control1 = pi1.pos() * (1.0 - coeff) + coeff * controlTarget1;
    }

    paintBezierCurve(pi1,
                     control1,
                     control2,
                     pi2);
}

qreal KisToolFreehandHelper::Private::effectiveSmoothnessDistance() const
{
    const qreal effectiveSmoothnessDistance =
        !smoothingOptions->useScalableDistance() ?
        smoothingOptions->smoothnessDistance() :
        smoothingOptions->smoothnessDistance() / resources->effectiveZoom();

    return effectiveSmoothnessDistance;
}

void KisToolFreehandHelper::paintEvent(KoPointerEvent *event)
{
    KisPaintInformation info =
            m_d->infoBuilder->continueStroke(event,
                                             elapsedStrokeTime());
    KisUpdateTimeMonitor::instance()->reportMouseMove(info.pos());

    paint(info);
}

void KisToolFreehandHelper::paint(KisPaintInformation &info)
{
    /**
     * Smooth the coordinates out using the history and the
     * distance. This is a heavily modified version of an algo used in
     * Gimp and described in https://bugs.kde.org/show_bug.cgi?id=281267 and
     * http://www24.atwiki.jp/sigetch_2007/pages/17.html.  The main
     * differences are:
     *
     * 1) It uses 'distance' instead of 'velocity', since time
     *    measurements are too unstable in realworld environment
     *
     * 2) There is no 'Quality' parameter, since the number of samples
     *    is calculated automatically
     *
     * 3) 'Tail Aggressiveness' is used for controlling the end of the
     *    stroke
     *
     * 4) The formila is a little bit different: 'Distance' parameter
     *    stands for $3 \Sigma$
     */
    if (m_d->smoothingOptions->smoothingType() == KisSmoothingOptions::WEIGHTED_SMOOTHING
        && m_d->smoothingOptions->smoothnessDistance() > 0.0) {

        { // initialize current distance
            QPointF prevPos;

            if (!m_d->history.isEmpty()) {
                const KisPaintInformation &prevPi = m_d->history.last();
                prevPos = prevPi.pos();
            } else {
                prevPos = m_d->previousPaintInformation.pos();
            }

            qreal currentDistance = QVector2D(info.pos() - prevPos).length();
            m_d->distanceHistory.append(currentDistance);
        }

        m_d->history.append(info);

        qreal x = 0.0;
        qreal y = 0.0;

        if (m_d->history.size() > 3) {
            const qreal sigma = m_d->effectiveSmoothnessDistance() / 3.0; // '3.0' for (3 * sigma) range

            qreal gaussianWeight = 1 / (sqrt(2 * M_PI) * sigma);
            qreal gaussianWeight2 = sigma * sigma;
            qreal distanceSum = 0.0;
            qreal scaleSum = 0.0;
            qreal pressure = 0.0;
            qreal baseRate = 0.0;

            Q_ASSERT(m_d->history.size() == m_d->distanceHistory.size());

            for (int i = m_d->history.size() - 1; i >= 0; i--) {
                qreal rate = 0.0;

                const KisPaintInformation nextInfo = m_d->history.at(i);
                double distance = m_d->distanceHistory.at(i);
                Q_ASSERT(distance >= 0.0);

                qreal pressureGrad = 0.0;
                if (i < m_d->history.size() - 1) {
                    pressureGrad = nextInfo.pressure() - m_d->history.at(i + 1).pressure();

                    const qreal tailAgressiveness = 40.0 * m_d->smoothingOptions->tailAggressiveness();

                    if (pressureGrad > 0.0 ) {
                        pressureGrad *= tailAgressiveness * (1.0 - nextInfo.pressure());
                        distance += pressureGrad * 3.0 * sigma; // (3 * sigma) --- holds > 90% of the region
                    }
                }

                if (gaussianWeight2 != 0.0) {
                    distanceSum += distance;
                    rate = gaussianWeight * exp(-distanceSum * distanceSum / (2 * gaussianWeight2));
                }

                if (m_d->history.size() - i == 1) {
                    baseRate = rate;
                } else if (baseRate / rate > 100) {
                    break;
                }

                scaleSum += rate;
                x += rate * nextInfo.pos().x();
                y += rate * nextInfo.pos().y();

                if (m_d->smoothingOptions->smoothPressure()) {
                    pressure += rate * nextInfo.pressure();
                }
            }

            if (scaleSum != 0.0) {
                x /= scaleSum;
                y /= scaleSum;

                if (m_d->smoothingOptions->smoothPressure()) {
                    pressure /= scaleSum;
                }
            }

            if ((x != 0.0 && y != 0.0) || (x == info.pos().x() && y == info.pos().y())) {
                info.setPos(QPointF(x, y));
                if (m_d->smoothingOptions->smoothPressure()) {
                    info.setPressure(pressure);
                }
                m_d->history.last() = info;
            }
        }
    }

    if (m_d->smoothingOptions->smoothingType() == KisSmoothingOptions::SIMPLE_SMOOTHING
        || m_d->smoothingOptions->smoothingType() == KisSmoothingOptions::WEIGHTED_SMOOTHING)
    {
        // Now paint between the coordinates, using the bezier curve interpolation
        if (!m_d->haveTangent) {
            m_d->haveTangent = true;
            m_d->previousTangent =
                (info.pos() - m_d->previousPaintInformation.pos()) /
                qMax(qreal(1.0), info.currentTime() - m_d->previousPaintInformation.currentTime());
        } else {
            QPointF newTangent = (info.pos() - m_d->olderPaintInformation.pos()) /
                qMax(qreal(1.0), info.currentTime() - m_d->olderPaintInformation.currentTime());

            if (newTangent.isNull() || m_d->previousTangent.isNull())
            {
                paintLine(m_d->previousPaintInformation, info);
            } else {
                paintBezierSegment(m_d->olderPaintInformation, m_d->previousPaintInformation,
                                   m_d->previousTangent, newTangent);
            }

            m_d->previousTangent = newTangent;
        }
        m_d->olderPaintInformation = m_d->previousPaintInformation;

        // Enable stroke timeout only when not airbrushing.
        if (!m_d->airbrushingTimer.isActive()) {
            m_d->strokeTimeoutTimer.start(100);
        }
    }
    else if (m_d->smoothingOptions->smoothingType() == KisSmoothingOptions::NO_SMOOTHING){
        paintLine(m_d->previousPaintInformation, info);
    }

    if (m_d->smoothingOptions->smoothingType() == KisSmoothingOptions::STABILIZER) {
        m_d->stabilizedSampler.addEvent(info);
        if (m_d->stabilizerDelayedPaintHelper.running()) {
            // Paint here so we don't have to rely on the timer
            // This is just a tricky source for a relatively stable 7ms "timer"
            m_d->stabilizerDelayedPaintHelper.paintSome();
        }
    } else {
        m_d->previousPaintInformation = info;
    }

    if(m_d->airbrushingTimer.isActive()) {
        m_d->airbrushingTimer.start();
    }
}

void KisToolFreehandHelper::endPaint()
{
    if (!m_d->hasPaintAtLeastOnce) {
        paintAt(m_d->previousPaintInformation);
    } else if (m_d->smoothingOptions->smoothingType() != KisSmoothingOptions::NO_SMOOTHING) {
        finishStroke();
    }
    m_d->strokeTimeoutTimer.stop();

    if(m_d->airbrushingTimer.isActive()) {
        m_d->airbrushingTimer.stop();
    }

    if (m_d->smoothingOptions->smoothingType() == KisSmoothingOptions::STABILIZER) {
        stabilizerEnd();
    }

    if (m_d->asyncUpdateHelper.isActive()) {
        m_d->asyncUpdateHelper.endUpdateStream();
    }

    /**
     * There might be some timer events still pending, so
     * we should cancel them. Use this flag for the purpose.
     * Please note that we are not in MT here, so no mutex
     * is needed
     */
    m_d->strokeInfos.clear();

    m_d->strokesFacade->endStroke(m_d->strokeId);
    m_d->strokeId.clear();
}

void KisToolFreehandHelper::cancelPaint()
{
    if (!m_d->strokeId) return;

    m_d->strokeTimeoutTimer.stop();

    if (m_d->airbrushingTimer.isActive()) {
        m_d->airbrushingTimer.stop();
    }

    if (m_d->asyncUpdateHelper.isActive()) {
        m_d->asyncUpdateHelper.cancelUpdateStream();
    }

    if (m_d->stabilizerPollTimer.isActive()) {
        m_d->stabilizerPollTimer.stop();
    }

    if (m_d->stabilizerDelayedPaintHelper.running()) {
        m_d->stabilizerDelayedPaintHelper.cancel();
    }

    // see a comment in endPaint()
    m_d->strokeInfos.clear();

    m_d->strokesFacade->cancelStroke(m_d->strokeId);
    m_d->strokeId.clear();

}

int KisToolFreehandHelper::elapsedStrokeTime() const
{
    return m_d->strokeTime.elapsed();
}

void KisToolFreehandHelper::stabilizerStart(KisPaintInformation firstPaintInfo)
{
    m_d->usingStabilizer = true;
    // FIXME: Ugly hack, this is no a "distance" in any way
    int sampleSize = qRound(m_d->effectiveSmoothnessDistance());
    sampleSize = qMax(3, sampleSize);

    // Fill the deque with the current value repeated until filling the sample
    m_d->stabilizerDeque.clear();
    for (int i = sampleSize; i > 0; i--) {
        m_d->stabilizerDeque.enqueue(firstPaintInfo);
    }

    // Poll and draw regularly
    KisConfig cfg(true);
    int stabilizerSampleSize = cfg.stabilizerSampleSize();
    m_d->stabilizerPollTimer.setInterval(stabilizerSampleSize);
    m_d->stabilizerPollTimer.start();

    bool delayedPaintEnabled = cfg.stabilizerDelayedPaint();
    if (delayedPaintEnabled) {
        m_d->stabilizerDelayedPaintHelper.start(firstPaintInfo);
    }

    m_d->stabilizedSampler.clear();
    m_d->stabilizedSampler.addEvent(firstPaintInfo);
}

KisPaintInformation
KisToolFreehandHelper::getStabilizedPaintInfo(const QQueue<KisPaintInformation> &queue,
                                              const KisPaintInformation &lastPaintInfo)
{
    KisPaintInformation result(lastPaintInfo.pos(),
                               lastPaintInfo.pressure(),
                               lastPaintInfo.xTilt(),
                               lastPaintInfo.yTilt(),
                               lastPaintInfo.rotation(),
                               lastPaintInfo.tangentialPressure(),
                               lastPaintInfo.perspective(),
                               elapsedStrokeTime(),
                               lastPaintInfo.drawingSpeed());

    if (queue.size() > 1) {
        QQueue<KisPaintInformation>::const_iterator it = queue.constBegin();
        QQueue<KisPaintInformation>::const_iterator end = queue.constEnd();

        /**
         * The first point is going to be overridden by lastPaintInfo, skip it.
         */
        it++;
        int i = 2;

        if (m_d->smoothingOptions->stabilizeSensors()) {
            while (it != end) {
                qreal k = qreal(i - 1) / i; // coeff for uniform averaging
                result.KisPaintInformation::mixOtherWithoutTime(k, *it);
                it++;
                i++;
            }
        } else{
            while (it != end) {
                qreal k = qreal(i - 1) / i; // coeff for uniform averaging
                result.KisPaintInformation::mixOtherOnlyPosition(k, *it);
                it++;
                i++;
            }
        }
    }

    return result;
}

void KisToolFreehandHelper::stabilizerPollAndPaint()
{
    KisStabilizedEventsSampler::iterator it;
    KisStabilizedEventsSampler::iterator end;
    std::tie(it, end) = m_d->stabilizedSampler.range();
    QVector<KisPaintInformation> delayedPaintTodoItems;

    for (; it != end; ++it) {
        KisPaintInformation sampledInfo = *it;

        bool canPaint = true;

        if (m_d->smoothingOptions->useDelayDistance()) {
            const qreal R = m_d->smoothingOptions->delayDistance() /
                    m_d->resources->effectiveZoom();

            QPointF diff = sampledInfo.pos() - m_d->previousPaintInformation.pos();
            qreal dx = sqrt(pow2(diff.x()) + pow2(diff.y()));

            if (!(dx > R)) {
                if (m_d->resources->needsAirbrushing()) {
                    sampledInfo.setPos(m_d->previousPaintInformation.pos());
                }
                else {
                    canPaint = false;
                }
            }
        }

        if (canPaint) {
            KisPaintInformation newInfo = getStabilizedPaintInfo(m_d->stabilizerDeque, sampledInfo);

            if (m_d->stabilizerDelayedPaintHelper.running()) {
                delayedPaintTodoItems.append(newInfo);
            } else {
                paintLine(m_d->previousPaintInformation, newInfo);
            }
            m_d->previousPaintInformation = newInfo;

            // Push the new entry through the queue
            m_d->stabilizerDeque.dequeue();
            m_d->stabilizerDeque.enqueue(sampledInfo);
        } else if (m_d->stabilizerDeque.head().pos() != m_d->previousPaintInformation.pos()) {

            QQueue<KisPaintInformation>::iterator it = m_d->stabilizerDeque.begin();
            QQueue<KisPaintInformation>::iterator end = m_d->stabilizerDeque.end();

            while (it != end) {
                *it = m_d->previousPaintInformation;
                ++it;
            }
        }
    }

    m_d->stabilizedSampler.clear();

    if (m_d->stabilizerDelayedPaintHelper.running()) {
        m_d->stabilizerDelayedPaintHelper.update(delayedPaintTodoItems);
    } else {
        emit requestExplicitUpdateOutline();
    }
}

void KisToolFreehandHelper::stabilizerEnd()
{
    // Stop the timer
    m_d->stabilizerPollTimer.stop();

    // Finish the line
    if (m_d->smoothingOptions->finishStabilizedCurve()) {
        // Process all the existing events first
        stabilizerPollAndPaint();

        // Draw the finish line with pending events and a time override
        m_d->stabilizedSampler.addFinishingEvent(m_d->stabilizerDeque.size());
        stabilizerPollAndPaint();
    }

    if (m_d->stabilizerDelayedPaintHelper.running()) {
        m_d->stabilizerDelayedPaintHelper.end();
    }
    m_d->usingStabilizer = false;
}

void KisToolFreehandHelper::slotSmoothingTypeChanged()
{
    if (!isRunning()) {
        return;
    }
    KisSmoothingOptions::SmoothingType currentSmoothingType =
            m_d->smoothingOptions->smoothingType();
    if (m_d->usingStabilizer
            && (currentSmoothingType != KisSmoothingOptions::STABILIZER)) {
        stabilizerEnd();
    } else if (!m_d->usingStabilizer
            && (currentSmoothingType == KisSmoothingOptions::STABILIZER)) {
        stabilizerStart(m_d->previousPaintInformation);
    }
}

void KisToolFreehandHelper::finishStroke()
{
    if (m_d->haveTangent) {
        m_d->haveTangent = false;

        QPointF newTangent = (m_d->previousPaintInformation.pos() - m_d->olderPaintInformation.pos()) /
            (m_d->previousPaintInformation.currentTime() - m_d->olderPaintInformation.currentTime());

        paintBezierSegment(m_d->olderPaintInformation,
                           m_d->previousPaintInformation,
                           m_d->previousTangent,
                           newTangent);
    }
}

void KisToolFreehandHelper::doAirbrushing()
{
    // Check that the stroke hasn't ended.
    if (!m_d->strokeInfos.isEmpty()) {

        // Add a new painting update at a point identical to the previous one, except for the time
        // and speed information.
        const KisPaintInformation &prevPaint = m_d->previousPaintInformation;
        KisPaintInformation nextPaint(prevPaint.pos(),
                                      prevPaint.pressure(),
                                      prevPaint.xTilt(),
                                      prevPaint.yTilt(),
                                      prevPaint.rotation(),
                                      prevPaint.tangentialPressure(),
                                      prevPaint.perspective(),
                                      elapsedStrokeTime(),
                                      0.0);
        paint(nextPaint);
    }
}

int KisToolFreehandHelper::computeAirbrushTimerInterval() const
{
    qreal realInterval = m_d->resources->airbrushingInterval() * AIRBRUSH_INTERVAL_FACTOR;
    return qMax(1, qFloor(realInterval));
}

void KisToolFreehandHelper::paintAt(int strokeInfoId,
                                    const KisPaintInformation &pi)
{
    m_d->hasPaintAtLeastOnce = true;
    m_d->strokesFacade->addJob(m_d->strokeId,
                               new FreehandStrokeStrategy::Data(strokeInfoId, pi));

}

void KisToolFreehandHelper::paintLine(int strokeInfoId,
                                      const KisPaintInformation &pi1,
                                      const KisPaintInformation &pi2)
{
    m_d->hasPaintAtLeastOnce = true;
    m_d->strokesFacade->addJob(m_d->strokeId,
                               new FreehandStrokeStrategy::Data(strokeInfoId, pi1, pi2));

}

void KisToolFreehandHelper::paintBezierCurve(int strokeInfoId,
                                             const KisPaintInformation &pi1,
                                             const QPointF &control1,
                                             const QPointF &control2,
                                             const KisPaintInformation &pi2)
{
#ifdef DEBUG_BEZIER_CURVES
    KisPaintInformation tpi1;
    KisPaintInformation tpi2;

    tpi1 = pi1;
    tpi2 = pi2;

    tpi1.setPressure(0.3);
    tpi2.setPressure(0.3);

    paintLine(tpi1, tpi2);

    tpi1.setPressure(0.6);
    tpi2.setPressure(0.3);

    tpi1.setPos(pi1.pos());
    tpi2.setPos(control1);
    paintLine(tpi1, tpi2);

    tpi1.setPos(pi2.pos());
    tpi2.setPos(control2);
    paintLine(tpi1, tpi2);
#endif

    m_d->hasPaintAtLeastOnce = true;
    m_d->strokesFacade->addJob(m_d->strokeId,
                               new FreehandStrokeStrategy::Data(strokeInfoId,
                                                                pi1, control1, control2, pi2));

}

void KisToolFreehandHelper::createPainters(QVector<KisFreehandStrokeInfo*> &strokeInfos,
                                           const KisDistanceInformation &startDist)
{
    strokeInfos << new KisFreehandStrokeInfo(startDist);
}

void KisToolFreehandHelper::paintAt(const KisPaintInformation &pi)
{
    paintAt(0, pi);
}

void KisToolFreehandHelper::paintLine(const KisPaintInformation &pi1,
                                      const KisPaintInformation &pi2)
{
    paintLine(0, pi1, pi2);
}

void KisToolFreehandHelper::paintBezierCurve(const KisPaintInformation &pi1,
                                             const QPointF &control1,
                                             const QPointF &control2,
                                             const KisPaintInformation &pi2)
{
    paintBezierCurve(0, pi1, control1, control2, pi2);
}
