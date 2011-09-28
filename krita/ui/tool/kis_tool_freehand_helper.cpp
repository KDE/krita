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

#include <KoPointerEvent.h>
#include <KoResourceManager.h>

#include "kis_distance_information.h"
#include "kis_painting_information_builder.h"
#include "kis_recording_adapter.h"
#include "kis_image.h"
#include "strokes/freehand_stroke.h"
#include "kis_painter.h"


struct KisToolFreehandHelper::Private
{
    KisPaintingInformationBuilder *infoBuilder;
    KisRecordingAdapter *recordingAdapter;
    KisStrokesFacade *strokesFacade;

    bool haveTangent;
    QPointF previousTangent;


    bool hasPaintAtLeastOnce;
    KisDistanceInformation dragDistance;

    QTime strokeTime;
    QTimer strokeTimeoutTimer;

    QVector<KisPainter*> painters;
    KisResourcesSnapshotSP resources;
    KisStrokeId strokeId;

    KisPaintInformation previousPaintInformation;
    KisPaintInformation olderPaintInformation;

    bool smooth;
    qreal smoothness;

    QTimer airbrushingTimer;
};


KisToolFreehandHelper::KisToolFreehandHelper(KisPaintingInformationBuilder *infoBuilder,
                                             KisRecordingAdapter *recordingAdapter)
    : m_d(new Private)
{
    m_d->infoBuilder = infoBuilder;
    m_d->recordingAdapter = recordingAdapter;

    m_d->smooth = true;
    m_d->smoothness = 1.0;

    m_d->strokeTimeoutTimer.setSingleShot(true);
    connect(&m_d->strokeTimeoutTimer, SIGNAL(timeout()), SLOT(finishStroke()));

    connect(&m_d->airbrushingTimer, SIGNAL(timeout()), SLOT(doAirbrushing()));
}

KisToolFreehandHelper::~KisToolFreehandHelper()
{
    delete m_d;
}

void KisToolFreehandHelper::setSmoothness(bool smooth, qreal smoothness)
{
    m_d->smooth = smooth;
    m_d->smoothness = smoothness;
}

void KisToolFreehandHelper::initPaint(KoPointerEvent *event,
                                      KoResourceManager *resourceManager,
                                      KisImageWSP image,
                                      KisStrokesFacade *strokesFacade,
                                      KisPostExecutionUndoAdapter *undoAdapter,
                                      KisNodeSP overrideNode)
{
    Q_UNUSED(overrideNode);

    m_d->strokesFacade = strokesFacade;


    m_d->haveTangent = false;
    m_d->previousTangent = QPointF();

    m_d->hasPaintAtLeastOnce = false;
    m_d->dragDistance.clear();

    m_d->strokeTime.start();

    createPainters(m_d->painters);
    m_d->resources = new KisResourcesSnapshot(image,
                                              undoAdapter,
                                              resourceManager);

    if(overrideNode) {
        m_d->resources->setCurrentNode(overrideNode);
    }

    bool indirectPainting = m_d->resources->needsIndirectPainting();

    if(m_d->recordingAdapter) {
        m_d->recordingAdapter->startStroke(image, m_d->resources);
    }

    KisStrokeStrategy *stroke =
        new FreehandStrokeStrategy(indirectPainting,
                                   m_d->resources, m_d->painters);

    m_d->strokeId = m_d->strokesFacade->startStroke(stroke);

    m_d->previousPaintInformation =
        m_d->infoBuilder->startStroke(event, m_d->strokeTime.elapsed());

    if(m_d->resources->needsAirbrushing()) {
        m_d->airbrushingTimer.setInterval(m_d->resources->airbrushingRate());
        m_d->airbrushingTimer.start();
    }
}

void KisToolFreehandHelper::paint(KoPointerEvent *event)
{
    KisPaintInformation info =
        m_d->infoBuilder->continueStroke(event,
                                         m_d->previousPaintInformation.pos(),
                                         m_d->strokeTime.elapsed());

    if (m_d->smooth) {
        if (!m_d->haveTangent) {
            m_d->haveTangent = true;
            m_d->previousTangent =
                (info.pos() - m_d->previousPaintInformation.pos()) * m_d->smoothness /
                (3.0 * (info.currentTime() - m_d->previousPaintInformation.currentTime()));
        } else {
            QPointF newTangent = (info.pos() - m_d->olderPaintInformation.pos()) * m_d->smoothness /
                                  (3.0 * (info.currentTime() - m_d->olderPaintInformation.currentTime()));
            qreal scaleFactor = (m_d->previousPaintInformation.currentTime() - m_d->olderPaintInformation.currentTime());
            QPointF control1 = m_d->olderPaintInformation.pos() + m_d->previousTangent * scaleFactor;
            QPointF control2 = m_d->previousPaintInformation.pos() - newTangent * scaleFactor;
            paintBezierCurve(m_d->painters,
                             m_d->olderPaintInformation,
                             control1,
                             control2,
                             m_d->previousPaintInformation);
            m_d->previousTangent = newTangent;
        }
        m_d->olderPaintInformation = m_d->previousPaintInformation;
        m_d->strokeTimeoutTimer.start(100);
    } else {
        paintLine(m_d->painters, m_d->previousPaintInformation, info);
    }

    m_d->previousPaintInformation = info;

    if(m_d->airbrushingTimer.isActive()) {
        m_d->airbrushingTimer.start();
    }
}

void KisToolFreehandHelper::endPaint()
{
    if (!m_d->hasPaintAtLeastOnce) {
        paintAt(m_d->painters, m_d->previousPaintInformation);
    } else if (m_d->smooth) {
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
    m_d->painters.clear();

    m_d->strokesFacade->endStroke(m_d->strokeId);

    if(m_d->recordingAdapter) {
        m_d->recordingAdapter->endStroke();
    }
}

const KisPaintOp* KisToolFreehandHelper::currentPaintOp() const
{
    return !m_d->painters.isEmpty() ? m_d->painters.first()->paintOp() : 0;
}


void KisToolFreehandHelper::finishStroke()
{
    if(m_d->haveTangent) {
        m_d->haveTangent = false;

        QPointF newTangent = (m_d->previousPaintInformation.pos() - m_d->olderPaintInformation.pos()) * m_d->smoothness / 3.0;
        qreal scaleFactor = (m_d->previousPaintInformation.currentTime() - m_d->olderPaintInformation.currentTime());
        QPointF control1 = m_d->olderPaintInformation.pos() + m_d->previousTangent * scaleFactor;
        QPointF control2 = m_d->previousPaintInformation.pos() - newTangent;
        paintBezierCurve(m_d->painters,
                         m_d->olderPaintInformation,
                         control1,
                         control2,
                         m_d->previousPaintInformation);
    }
}

void KisToolFreehandHelper::doAirbrushing()
{
    if(!m_d->painters.isEmpty()) {
        paintAt(m_d->painters, m_d->previousPaintInformation);
    }
}

void KisToolFreehandHelper::paintAt(KisPainter *painter,
                                    const KisPaintInformation &pi)
{
    m_d->hasPaintAtLeastOnce = true;
    m_d->strokesFacade->addJob(m_d->strokeId,
        new FreehandStrokeStrategy::Data(m_d->resources->currentNode(),
                                         painter, pi,
                                         m_d->dragDistance));

    if(m_d->recordingAdapter) {
        m_d->recordingAdapter->addPoint(pi);
    }
}

void KisToolFreehandHelper::paintLine(KisPainter *painter,
                                      const KisPaintInformation &pi1,
                                      const KisPaintInformation &pi2)
{
    m_d->hasPaintAtLeastOnce = true;
    m_d->strokesFacade->addJob(m_d->strokeId,
        new FreehandStrokeStrategy::Data(m_d->resources->currentNode(),
                                         painter, pi1, pi2,
                                         m_d->dragDistance));

    if(m_d->recordingAdapter) {
        m_d->recordingAdapter->addLine(pi1, pi2);
    }
}

void KisToolFreehandHelper::paintBezierCurve(KisPainter *painter,
                                             const KisPaintInformation &pi1,
                                             const QPointF &control1,
                                             const QPointF &control2,
                                             const KisPaintInformation &pi2)
{
    m_d->hasPaintAtLeastOnce = true;
    m_d->strokesFacade->addJob(m_d->strokeId,
        new FreehandStrokeStrategy::Data(m_d->resources->currentNode(),
                                         painter,
                                         pi1, control1, control2, pi2,
                                         m_d->dragDistance));

    if(m_d->recordingAdapter) {
        m_d->recordingAdapter->addCurve(pi1, control1, control2, pi2);
    }
}

void KisToolFreehandHelper::createPainters(QVector<KisPainter*> &painters)
{
    painters << new KisPainter();
}

void KisToolFreehandHelper::paintAt(const QVector<KisPainter*> &painters,
                                    const KisPaintInformation &pi)
{
    paintAt(painters.first(), pi);
}

void KisToolFreehandHelper::paintLine(const QVector<KisPainter*> &painters,
                                      const KisPaintInformation &pi1,
                                      const KisPaintInformation &pi2)
{
    paintLine(painters.first(), pi1, pi2);
}

void KisToolFreehandHelper::paintBezierCurve(const QVector<KisPainter*> &painters,
                                             const KisPaintInformation &pi1,
                                             const QPointF &control1,
                                             const QPointF &control2,
                                             const KisPaintInformation &pi2)
{
    paintBezierCurve(painters.first(), pi1, control1, control2, pi2);
}

