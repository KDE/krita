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

#include "freehand_stroke.h"

#include <QElapsedTimer>

#include "kis_canvas_resource_provider.h"
#include <brushengine/kis_paintop_preset.h>
#include <brushengine/kis_paintop_settings.h>
#include "kis_painter.h"
#include "kis_paintop.h"

#include "kis_update_time_monitor.h"

#include <brushengine/kis_stroke_random_source.h>


struct FreehandStrokeStrategy::Private
{
    Private(KisResourcesSnapshotSP _resources)
        : resources(_resources),
          needsAsynchronousUpdates(_resources->presetNeedsAsynchronousUpdates())
    {
    }

    KisStrokeRandomSource randomSource;
    KisResourcesSnapshotSP resources;

    QElapsedTimer timeSinceLastUpdate;
    int currentUpdatePeriod = 40;

    const bool needsAsynchronousUpdates = false;
};

FreehandStrokeStrategy::FreehandStrokeStrategy(bool needsIndirectPainting,
                                               const QString &indirectPaintingCompositeOp,
                                               KisResourcesSnapshotSP resources,
                                               PainterInfo *painterInfo,
                                               const KUndo2MagicString &name)
    : KisPainterBasedStrokeStrategy("FREEHAND_STROKE", name,
                                    resources, painterInfo),
      m_d(new Private(resources))
{
    init(needsIndirectPainting, indirectPaintingCompositeOp);
}

FreehandStrokeStrategy::FreehandStrokeStrategy(bool needsIndirectPainting,
                                               const QString &indirectPaintingCompositeOp,
                                               KisResourcesSnapshotSP resources,
                                               QVector<PainterInfo*> painterInfos,
                                               const KUndo2MagicString &name)
    : KisPainterBasedStrokeStrategy("FREEHAND_STROKE", name,
                                    resources, painterInfos),
      m_d(new Private(resources))
{
    init(needsIndirectPainting, indirectPaintingCompositeOp);
}

FreehandStrokeStrategy::FreehandStrokeStrategy(const FreehandStrokeStrategy &rhs, int levelOfDetail)
    : KisPainterBasedStrokeStrategy(rhs, levelOfDetail),
      m_d(new Private(*rhs.m_d))
{
    m_d->randomSource.setLevelOfDetail(levelOfDetail);
}

FreehandStrokeStrategy::~FreehandStrokeStrategy()
{
    KisUpdateTimeMonitor::instance()->endStrokeMeasure();
}

void FreehandStrokeStrategy::init(bool needsIndirectPainting,
                                  const QString &indirectPaintingCompositeOp)
{
    setNeedsIndirectPainting(needsIndirectPainting);
    setIndirectPaintingCompositeOp(indirectPaintingCompositeOp);
    setSupportsWrapAroundMode(true);
    enableJob(KisSimpleStrokeStrategy::JOB_DOSTROKE);

    if (m_d->needsAsynchronousUpdates) {
        m_d->timeSinceLastUpdate.start();
    }

    KisUpdateTimeMonitor::instance()->startStrokeMeasure();
}

void FreehandStrokeStrategy::doStrokeCallback(KisStrokeJobData *data)
{
    KisNodeSP updateNode;
    PainterInfo *info = 0;

    if (UpdateData *d = dynamic_cast<UpdateData*>(data)) {
        info = painterInfos()[d->painterInfoId];
        updateNode = d->node;

        // do nothing, just fall through till we reach the update code

    } else if (Data *d = dynamic_cast<Data*>(data)) {
        info = painterInfos()[d->painterInfoId];
        updateNode = d->node;

        KisUpdateTimeMonitor::instance()->reportPaintOpPreset(info->painter->preset());
        KisRandomSourceSP rnd = m_d->randomSource.source();

        switch(d->type) {
        case Data::POINT:
            d->pi1.setRandomSource(rnd);
            info->painter->paintAt(d->pi1, info->dragDistance);
            break;
        case Data::LINE:
            d->pi1.setRandomSource(rnd);
            d->pi2.setRandomSource(rnd);
            info->painter->paintLine(d->pi1, d->pi2, info->dragDistance);
            break;
        case Data::CURVE:
            d->pi1.setRandomSource(rnd);
            d->pi2.setRandomSource(rnd);
            info->painter->paintBezierCurve(d->pi1,
                                            d->control1,
                                            d->control2,
                                            d->pi2,
                                            info->dragDistance);
            break;
        case Data::POLYLINE:
            info->painter->paintPolyline(d->points, 0, d->points.size());
            break;
        case Data::POLYGON:
            info->painter->paintPolygon(d->points);
            break;
        case Data::RECT:
            info->painter->paintRect(d->rect);
            break;
        case Data::ELLIPSE:
            info->painter->paintEllipse(d->rect);
            break;
        case Data::PAINTER_PATH:
            info->painter->paintPainterPath(d->path);
            break;
        case Data::QPAINTER_PATH:
            info->painter->drawPainterPath(d->path, d->pen);
            break;
        case Data::QPAINTER_PATH_FILL: {
            info->painter->setBackgroundColor(d->customColor);
            info->painter->fillPainterPath(d->path);}
            info->painter->drawPainterPath(d->path, d->pen);
            break;
        };
    }

    KIS_SAFE_ASSERT_RECOVER_RETURN(info);
    KIS_SAFE_ASSERT_RECOVER_RETURN(updateNode);

    tryDoUpdate();
}

void FreehandStrokeStrategy::finishStrokeCallback()
{
    tryDoUpdate(true);
    KisPainterBasedStrokeStrategy::finishStrokeCallback();
}

void FreehandStrokeStrategy::tryDoUpdate(bool forceEnd)
{
    // We do not distinguish between updates for each painter info. Just
    // update all of them at once!

    Q_FOREACH (PainterInfo *info, painterInfos()) {
        KisPaintOp *paintop = info->painter->paintOp();
        if (m_d->needsAsynchronousUpdates && paintop &&
            (forceEnd || m_d->timeSinceLastUpdate.elapsed() > m_d->currentUpdatePeriod)) {

            m_d->timeSinceLastUpdate.restart();
            m_d->currentUpdatePeriod = paintop->doAsyncronousUpdate(forceEnd);
        }

        QVector<QRect> dirtyRects = info->painter->takeDirtyRegion();
        //KisUpdateTimeMonitor::instance()->reportJobFinished(data, dirtyRects);
        targetNode()->setDirty(dirtyRects);
    }
}

KisStrokeStrategy* FreehandStrokeStrategy::createLodClone(int levelOfDetail)
{
    if (!m_d->resources->presetAllowsLod()) return 0;

    FreehandStrokeStrategy *clone = new FreehandStrokeStrategy(*this, levelOfDetail);
    return clone;
}
