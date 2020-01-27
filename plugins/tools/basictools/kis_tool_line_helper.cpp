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

#include <QtMath>

#include "kis_algebra_2d.h"
#include "kis_painting_information_builder.h"
#include "kis_image.h"

#include "kis_canvas_resource_provider.h"
#include <brushengine/kis_paintop_preset.h>

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

void KisToolLineHelper::repaintLine(KoCanvasResourceProvider *resourceManager,
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


    KisPaintOpPresetSP preset = resourceManager->resource(KisCanvasResourceProvider::CurrentPaintOpPreset)
            .value<KisPaintOpPresetSP>();

    if (preset->settings()->paintOpSize() <= 1) {
        KisPaintInformation begin = m_d->linePoints.first();
        KisPaintInformation end = m_d->linePoints.last();
        m_d->linePoints.clear();
        m_d->linePoints.append(begin);
        m_d->linePoints.append(end);
    }
    // Always adjust line sections to avoid jagged sections.
    adjustPointsToDDA(m_d->linePoints);

    QVector<KisPaintInformation>::const_iterator it = m_d->linePoints.constBegin();
    QVector<KisPaintInformation>::const_iterator end = m_d->linePoints.constEnd();

    initPaintImpl(startAngle, *it, resourceManager, image, node, strokesFacade);
    ++it;

    while (it != end) {
        paintLine(*(it - 1), *it);
        ++it;
    }
}

void KisToolLineHelper::start(KoPointerEvent *event, KoCanvasResourceProvider *resourceManager)
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
    clearPoints();
}


void KisToolLineHelper::cancel()
{
    if (!m_d->enabled) return;
    KIS_ASSERT_RECOVER_RETURN(isRunning());

    cancelPaint();
    clearPoints();
}


void KisToolLineHelper::clearPoints()
{
    m_d->linePoints.clear();
}


void KisToolLineHelper::clearPaint()
{
    if (!m_d->enabled) return;

    cancelPaint();
}

void KisToolLineHelper::adjustPointsToDDA(QVector<KisPaintInformation> &points)
{
    int x = qFloor(points.first().pos().x());
    int y = qFloor(points.first().pos().y());

    int x2 = qFloor(points.last().pos().x());
    int y2 = qFloor(points.last().pos().y());

    // Width and height of the line
    int xd = x2 - x;
    int yd = y2 - y;

    float m = 0;
    bool lockAxis = true;

    if (xd == 0) {
        m = 2.0;
    } else if ( yd != 0) {
        lockAxis = false;
        m = (float)yd / (float)xd;
    }

    float fx = x;
    float fy = y;

    int inc;
    int dist;

    if (fabs(m) > 1.0f) {
        inc = (yd > 0) ? 1 : -1;
        m = (lockAxis)? 0 : 1.0f / m;
        m *= inc;

        for (int i = 0; i < points.size(); i++){
            dist = abs(qFloor(points.at(i).pos().y()) - y);
            fy = y + (dist * inc);
            fx = qRound(x + (dist * m));
            points[i].setPos(QPointF(fx,fy));
        }

    } else {
        inc = (xd > 0) ? 1 : -1;
        m *= inc;

        for (int i = 0; i < points.size(); i++){
            dist = abs(qFloor(points.at(i).pos().x()) - x);
            fx = x + (dist * inc);
            fy = qRound(y + (dist * m));
            points[i].setPos(QPointF(fx,fy));
        }
    }
}
