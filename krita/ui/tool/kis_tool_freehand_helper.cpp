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

#include <klocale.h>

#include <KoPointerEvent.h>
#include <KoCanvasResourceManager.h>

#include "kis_distance_information.h"
#include "kis_painting_information_builder.h"
#include "kis_recording_adapter.h"
#include "kis_image.h"
#include "kis_painter.h"
#include "kis_smoothing_options.h"

#include <math.h>
#include <qnumeric.h> // for qIsNaN

//#define DEBUG_BEZIER_CURVES


struct KisToolFreehandHelper::Private
{
    KisPaintingInformationBuilder *infoBuilder;
    KisRecordingAdapter *recordingAdapter;
    KisStrokesFacade *strokesFacade;

    bool haveTangent;
    QPointF previousTangent;

    bool hasPaintAtLeastOnce;

    QTime strokeTime;
    QTimer strokeTimeoutTimer;

    QVector<PainterInfo*> painterInfos;
    KisResourcesSnapshotSP resources;
    KisStrokeId strokeId;

    KisPaintInformation previousPaintInformation;
    KisPaintInformation olderPaintInformation;

    KisSmoothingOptions smoothingOptions;

    QTimer airbrushingTimer;

    QList<KisPaintInformation> history;
    QList<qreal> velocityHistory;
};


KisToolFreehandHelper::KisToolFreehandHelper(KisPaintingInformationBuilder *infoBuilder,
                                             KisRecordingAdapter *recordingAdapter)
    : m_d(new Private)
{
    m_d->infoBuilder = infoBuilder;
    m_d->recordingAdapter = recordingAdapter;

    m_d->strokeTimeoutTimer.setSingleShot(true);
    connect(&m_d->strokeTimeoutTimer, SIGNAL(timeout()), SLOT(finishStroke()));

    connect(&m_d->airbrushingTimer, SIGNAL(timeout()), SLOT(doAirbrushing()));
}

KisToolFreehandHelper::~KisToolFreehandHelper()
{
    delete m_d;
}

void KisToolFreehandHelper::setSmoothness(const KisSmoothingOptions &smoothingOptions)
{
    m_d->smoothingOptions = smoothingOptions;
}

void KisToolFreehandHelper::initPaint(KoPointerEvent *event,
                                      KoCanvasResourceManager *resourceManager,
                                      KisImageWSP image,
                                      KisStrokesFacade *strokesFacade,
                                      KisPostExecutionUndoAdapter *undoAdapter,
                                      KisNodeSP overrideNode,
                                      KisDefaultBoundsBaseSP bounds)
{
    Q_UNUSED(overrideNode);

    m_d->strokesFacade = strokesFacade;

    m_d->haveTangent = false;
    m_d->previousTangent = QPointF();

    m_d->hasPaintAtLeastOnce = false;

    m_d->strokeTime.start();

    createPainters(m_d->painterInfos);
    m_d->resources = new KisResourcesSnapshot(image,
                                              undoAdapter,
                                              resourceManager,
                                              bounds);

    if(overrideNode) {
        m_d->resources->setCurrentNode(overrideNode);
    }

    bool indirectPainting = m_d->resources->needsIndirectPainting();

    if(m_d->recordingAdapter) {
        m_d->recordingAdapter->startStroke(image, m_d->resources);
    }

    KisStrokeStrategy *stroke =
            new FreehandStrokeStrategy(indirectPainting,
                                       m_d->resources, m_d->painterInfos, i18n("Freehand Stroke"));

    m_d->strokeId = m_d->strokesFacade->startStroke(stroke);

    m_d->previousPaintInformation =
            m_d->infoBuilder->startStroke(event, m_d->strokeTime.elapsed());

    m_d->history.clear();
    m_d->history.append(m_d->previousPaintInformation);
    m_d->velocityHistory.clear();
    m_d->velocityHistory.append(std::numeric_limits<qreal>::signaling_NaN());

    if(m_d->resources->needsAirbrushing()) {
        m_d->airbrushingTimer.setInterval(m_d->resources->airbrushingRate());
        m_d->airbrushingTimer.start();
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
            qDebug() << "WARINING: there is no intersection point "
                     << "in the basic smoothing algoriths";
        }

        controlTarget1 = intersection;
        controlTarget2 = intersection;
    }

    // shows how near to the controlTarget the value raises
    qreal coeff = 0.8;

    qreal velocity1 = QLineF(QPointF(), tangent1).length();
    qreal velocity2 = QLineF(QPointF(), tangent2).length();

    Q_ASSERT(velocity1 > 0);
    Q_ASSERT(velocity2 > 0);

    qreal similarity = qMin(velocity1/velocity2, velocity2/velocity1);

    // the controls should not differ more than 50%
    similarity = qMax(similarity, 0.5);

    // when the controls are symmetric, their size should be smaller
    // to avoid corner-like curves
    coeff *= 1 - qMax(0.0, similarity - 0.8);

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

    paintBezierCurve(m_d->painterInfos,
                     pi1,
                     control1,
                     control2,
                     pi2);
}

void KisToolFreehandHelper::paint(KoPointerEvent *event)
{
    KisPaintInformation info =
            m_d->infoBuilder->continueStroke(event,
                                             m_d->previousPaintInformation.pos(),
                                             m_d->strokeTime.elapsed());

    // Smooth the coordinates out using the history and the velocity. See
    // https://bugs.kde.org/show_bug.cgi?id=281267 and http://www24.atwiki.jp/sigetch_2007/pages/17.html.
    // This is also implemented in gimp, which is where I cribbed the code from.
    if (m_d->smoothingOptions.smoothingType == KisSmoothingOptions::WEIGHTED_SMOOTHING
        && m_d->smoothingOptions.smoothnessDistance > 0.0) {

        m_d->history.append(info);
        m_d->velocityHistory.append(std::numeric_limits<qreal>::signaling_NaN()); // Fake velocity!

        qreal x = 0.0;
        qreal y = 0.0;

        if (m_d->history.size() > 3) {
            const qreal avg_events_rate = 8; // ms
            const qreal sigma = m_d->smoothingOptions.smoothnessDistance / (3.0 * avg_events_rate); // '3.0' for (3 * sigma) range

            qreal gaussianWeight = 1 / (sqrt(2 * M_PI) * sigma);
            qreal gaussianWeight2 = sigma * sigma;
            qreal velocitySum = 0.0;
            qreal scaleSum = 0.0;
            qreal pressure = 0.0;
            qreal baseRate = 0.0;

            Q_ASSERT(m_d->history.size() == m_d->velocityHistory.size());

            for (int i = m_d->history.size() - 1; i >= 0; i--) {
                qreal rate = 0.0;

                const KisPaintInformation nextInfo = m_d->history.at(i);
                double velocity = m_d->velocityHistory.at(i);

                if (qIsNaN(velocity)) {

                    int previousTime = nextInfo.currentTime();
                    if (i > 0) {
                        previousTime = m_d->history.at(i - 1).currentTime();
                    }

                    int deltaTime = qMax(1, nextInfo.currentTime() - previousTime); // make sure deltaTime > 1
                    velocity = info.movement().norm() / deltaTime;
                    m_d->velocityHistory[i] = velocity;
                }

                qreal pressureGrad = 0.0;
                if (i < m_d->history.size() - 1) {
                    pressureGrad = nextInfo.pressure() - m_d->history.at(i + 1).pressure();

                    const qreal tailAgressiveness = 40.0 * m_d->smoothingOptions.tailAggressiveness;

                    if (pressureGrad > 0.0 ) {
                        pressureGrad *= tailAgressiveness * (1.0 - nextInfo.pressure());
                        velocity += pressureGrad * 3.0 * sigma; // (3 * sigma) --- holds > 90% of the region
                    }
                }

                if (gaussianWeight2 != 0.0) {
                    velocitySum += velocity;
                    rate = gaussianWeight * exp(-velocitySum * velocitySum / (2 * gaussianWeight2));
                }

                if (m_d->history.size() - i == 1) {
                    baseRate = rate;
                } else if (baseRate / rate > 100) {
                    break;
                }

                scaleSum += rate;
                x += rate * nextInfo.pos().x();
                y += rate * nextInfo.pos().y();

                if (m_d->smoothingOptions.smoothPressure) {
                    pressure += rate * nextInfo.pressure();
                }
            }

            if (scaleSum != 0.0) {
                x /= scaleSum;
                y /= scaleSum;

                if (m_d->smoothingOptions.smoothPressure) {
                    pressure /= scaleSum;
                }
            }

            if ((x != 0.0 && y != 0.0) || (x == info.pos().x() && y == info.pos().y())) {
                info.setMovement(toKisVector2D(info.pos() - QPointF(x, y)));
                info.setPos(QPointF(x, y));
                if (m_d->smoothingOptions.smoothPressure) {
                    info.setPressure(pressure);
                }
                m_d->history.last() = info;
            }
        }
    }

    if (m_d->smoothingOptions.smoothingType == KisSmoothingOptions::SIMPLE_SMOOTHING
        || m_d->smoothingOptions.smoothingType == KisSmoothingOptions::WEIGHTED_SMOOTHING)
    {
        // Now paint between the coordinates, using the bezier curve interpolation
        if (!m_d->haveTangent) {
            m_d->haveTangent = true;
            m_d->previousTangent =
                    (info.pos() - m_d->previousPaintInformation.pos()) /
                    (info.currentTime() - m_d->previousPaintInformation.currentTime());
        } else {
            QPointF newTangent = (info.pos() - m_d->olderPaintInformation.pos()) /
                    (info.currentTime() - m_d->olderPaintInformation.currentTime());

            paintBezierSegment(m_d->olderPaintInformation, m_d->previousPaintInformation,
                               m_d->previousTangent, newTangent);

            m_d->previousTangent = newTangent;
        }
        m_d->olderPaintInformation = m_d->previousPaintInformation;
        m_d->strokeTimeoutTimer.start(100);
    }
    else {
        paintLine(m_d->painterInfos, m_d->previousPaintInformation, info);
    }

    m_d->previousPaintInformation = info;

    if(m_d->airbrushingTimer.isActive()) {
        m_d->airbrushingTimer.start();
    }
}

void KisToolFreehandHelper::endPaint()
{
    if (!m_d->hasPaintAtLeastOnce) {
        paintAt(m_d->painterInfos, m_d->previousPaintInformation);
    } else if (m_d->smoothingOptions.smoothingType != KisSmoothingOptions::NO_SMOOTHING) {
        finishStroke();
    }
    m_d->strokeTimeoutTimer.stop();

    if(m_d->airbrushingTimer.isActive()) {
        m_d->airbrushingTimer.stop();
    }

    /**
     * There might be some timer events still pending, so
     * we should cancel them. Use this flag for the purpose.
     * Please note that we are not in MT here, so no mutex
     * is needed
     */
    m_d->painterInfos.clear();

    m_d->strokesFacade->endStroke(m_d->strokeId);

    if(m_d->recordingAdapter) {
        m_d->recordingAdapter->endStroke();
    }
}

const KisPaintOp* KisToolFreehandHelper::currentPaintOp() const
{
    return !m_d->painterInfos.isEmpty() ? m_d->painterInfos.first()->painter->paintOp() : 0;
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
    if(!m_d->painterInfos.isEmpty()) {
        paintAt(m_d->painterInfos, m_d->previousPaintInformation);
    }
}

void KisToolFreehandHelper::paintAt(PainterInfo *painterInfo,
                                    const KisPaintInformation &pi)
{
    m_d->hasPaintAtLeastOnce = true;
    m_d->strokesFacade->addJob(m_d->strokeId,
                               new FreehandStrokeStrategy::Data(m_d->resources->currentNode(),
                                                                painterInfo, pi));

    if(m_d->recordingAdapter) {
        m_d->recordingAdapter->addPoint(pi);
    }
}

void KisToolFreehandHelper::paintLine(PainterInfo *painterInfo,
                                      const KisPaintInformation &pi1,
                                      const KisPaintInformation &pi2)
{
    m_d->hasPaintAtLeastOnce = true;
    m_d->strokesFacade->addJob(m_d->strokeId,
                               new FreehandStrokeStrategy::Data(m_d->resources->currentNode(),
                                                                painterInfo, pi1, pi2));

    if(m_d->recordingAdapter) {
        m_d->recordingAdapter->addLine(pi1, pi2);
    }
}

void KisToolFreehandHelper::paintBezierCurve(PainterInfo *painterInfo,
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

    paintLine(m_d->painterInfos, tpi1, tpi2);

    tpi1.setPressure(0.6);
    tpi2.setPressure(0.3);

    tpi1.setPos(pi1.pos());
    tpi2.setPos(control1);
    paintLine(m_d->painterInfos, tpi1, tpi2);

    tpi1.setPos(pi2.pos());
    tpi2.setPos(control2);
    paintLine(m_d->painterInfos, tpi1, tpi2);
#endif

    m_d->hasPaintAtLeastOnce = true;
    m_d->strokesFacade->addJob(m_d->strokeId,
                               new FreehandStrokeStrategy::Data(m_d->resources->currentNode(),
                                                                painterInfo,
                                                                pi1, control1, control2, pi2));

    if(m_d->recordingAdapter) {
        m_d->recordingAdapter->addCurve(pi1, control1, control2, pi2);
    }
}

void KisToolFreehandHelper::createPainters(QVector<PainterInfo*> &painterInfos)
{
    painterInfos << new PainterInfo(new KisPainter(), new KisDistanceInformation());
}

void KisToolFreehandHelper::paintAt(const QVector<PainterInfo*> &painterInfos,
                                    const KisPaintInformation &pi)
{
    paintAt(painterInfos.first(), pi);
}

void KisToolFreehandHelper::paintLine(const QVector<PainterInfo*> &painterInfos,
                                      const KisPaintInformation &pi1,
                                      const KisPaintInformation &pi2)
{
    paintLine(painterInfos.first(), pi1, pi2);
}

void KisToolFreehandHelper::paintBezierCurve(const QVector<PainterInfo*> &painterInfos,
                                             const KisPaintInformation &pi1,
                                             const QPointF &control1,
                                             const QPointF &control2,
                                             const KisPaintInformation &pi2)
{
    paintBezierCurve(painterInfos.first(), pi1, control1, control2, pi2);
}

