/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_tool_line_helper.h"

#include "kis_algebra_2d.h"
#include "kis_painting_information_builder.h"
#include "kis_image.h"

struct KisToolLineHelper::Private
{
    Private(KisPaintingInformationBuilder *_infoBuilder)
        : infoBuilder(_infoBuilder),
          useSensors(true),
          enabled(true)
    {
    }

    QVector<KisPaintInformation> linePoints;
    KisPaintingInformationBuilder *infoBuilder;
    bool useSensors;
    bool enabled;
};

KisToolLineHelper::KisToolLineHelper(KisPaintingInformationBuilder *infoBuilder,
                                     const KUndo2MagicString &transactionText)
    : KisToolFreehandHelper(infoBuilder,
                            transactionText,
                            new KisSmoothingOptions(false)),
      m_d(new Private(infoBuilder))
{
}

KisToolLineHelper::~KisToolLineHelper()
{
    delete m_d;
}

void KisToolLineHelper::setEnabled(bool value)
{
    m_d->enabled = value;
}

void KisToolLineHelper::setUseSensors(bool value)
{
    m_d->useSensors = value;
}

void KisToolLineHelper::repaintLine(KoCanvasResourceManager *resourceManager,
                                    KisImageWSP image, KisNodeSP node,
                                    KisStrokesFacade *strokesFacade)
{
    if (!m_d->enabled) return;

    cancelPaint();
    if (m_d->linePoints.isEmpty()) return;

    qreal startAngle = 0.0;
    if (m_d->linePoints.length() > 1) {
        startAngle = KisAlgebra2D::directionBetweenPoints(m_d->linePoints[0].pos(),
                                                          m_d->linePoints[1].pos(),
                                                          0.0);
    }

    QVector<KisPaintInformation>::const_iterator it = m_d->linePoints.constBegin();
    QVector<KisPaintInformation>::const_iterator end = m_d->linePoints.constEnd();

    initPaintImpl(startAngle, *it, resourceManager, image, node, strokesFacade);
    ++it;

    while (it != end) {
        paintLine(*(it - 1), *it);
        ++it;
    }
}

void KisToolLineHelper::start(KoPointerEvent *event, KoCanvasResourceManager *resourceManager)
{
    if (!m_d->enabled) return;

    // Ignore the elapsed stroke time, so that the line tool will behave as if the whole stroke is
    // drawn at once. This should prevent any possible spurious dabs caused by airbrushing features.
    KisPaintInformation pi =
            m_d->infoBuilder->startStroke(event, 0, resourceManager);

    if (!m_d->useSensors) {
        pi = KisPaintInformation(pi.pos());
    }

    m_d->linePoints.append(pi);
}

void KisToolLineHelper::addPoint(KoPointerEvent *event, const QPointF &overridePos)
{
    if (!m_d->enabled) return;

    // Ignore the elapsed stroke time, so that the line tool will behave as if the whole stroke is
    // drawn at once. This should prevent any possible spurious dabs caused by airbrushing features.
    KisPaintInformation pi =
            m_d->infoBuilder->continueStroke(event, 0);

    if (!m_d->useSensors) {
        pi = KisPaintInformation(pi.pos());
    }

    if (!overridePos.isNull()) {
        pi.setPos(overridePos);
    }

    if (m_d->linePoints.size() > 1) {
        const QPointF startPos = m_d->linePoints.first().pos();
        const QPointF endPos = pi.pos();
        const qreal maxDistance = kisDistance(startPos, endPos);
        const QPointF unit = (endPos - startPos) / maxDistance;

        QVector<KisPaintInformation>::iterator it = m_d->linePoints.begin();
        ++it;
        while (it != m_d->linePoints.end()) {
            qreal dist = kisDistance(startPos, it->pos());
            if (dist < maxDistance) {
                QPointF pos = startPos + unit * dist;
                it->setPos(pos);
                ++it;
            } else {
                it = m_d->linePoints.erase(it);
            }
        }
    }

    m_d->linePoints.append(pi);
}

void KisToolLineHelper::translatePoints(const QPointF &offset)
{
    if (!m_d->enabled) return;

    QVector<KisPaintInformation>::iterator it = m_d->linePoints.begin();
    while (it != m_d->linePoints.end()) {
        it->setPos(it->pos() + offset);
        ++it;
    }
}

void KisToolLineHelper::end()
{
    if (!m_d->enabled) return;
    KIS_ASSERT_RECOVER_RETURN(isRunning());

    endPaint();
    m_d->linePoints.clear();
}


void KisToolLineHelper::cancel()
{
    if (!m_d->enabled) return;
    KIS_ASSERT_RECOVER_RETURN(isRunning());

    cancelPaint();
    m_d->linePoints.clear();
}


void KisToolLineHelper::clearPaint()
{
    if (!m_d->enabled) return;

    cancelPaint();
}
