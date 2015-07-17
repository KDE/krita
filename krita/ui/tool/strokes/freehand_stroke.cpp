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

#include "kis_canvas_resource_provider.h"
#include "kis_paintop_preset.h"
#include "kis_paintop_settings.h"
#include "kis_painter.h"

#include "kis_update_time_monitor.h"

struct FreehandStrokeStrategy::Private
{
};

FreehandStrokeStrategy::FreehandStrokeStrategy(bool needsIndirectPainting,
                                               const QString &indirectPaintingCompositeOp,
                                               KisResourcesSnapshotSP resources,
                                               PainterInfo *painterInfo,
                                               const KUndo2MagicString &name)
    : KisPainterBasedStrokeStrategy("FREEHAND_STROKE", name,
                                    resources, painterInfo),
      m_d(new Private())
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
      m_d(new Private())
{
    init(needsIndirectPainting, indirectPaintingCompositeOp);
}

FreehandStrokeStrategy::FreehandStrokeStrategy(const FreehandStrokeStrategy &rhs)
    : KisPainterBasedStrokeStrategy(rhs),
      m_d(new Private(*rhs.m_d))
{
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

    KisUpdateTimeMonitor::instance()->startStrokeMeasure();
}

void FreehandStrokeStrategy::doStrokeCallback(KisStrokeJobData *data)
{
    Data *d = dynamic_cast<Data*>(data);
    PainterInfo *info = painterInfos()[d->painterInfoId];

    KisUpdateTimeMonitor::instance()->reportPaintOpPreset(info->painter->preset());

    switch(d->type) {
    case Data::POINT:
        info->painter->paintAt(d->pi1, info->dragDistance);
        break;
    case Data::LINE:
        info->painter->paintLine(d->pi1, d->pi2, info->dragDistance);
        break;
    case Data::CURVE:
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
    };

    QVector<QRect> dirtyRects = info->painter->takeDirtyRegion();
    KisUpdateTimeMonitor::instance()->reportJobFinished(data, dirtyRects);
    d->node->setDirty(dirtyRects);
}

KisStrokeStrategy* FreehandStrokeStrategy::createLodClone(int levelOfDetail)
{
    Q_UNUSED(levelOfDetail);

    FreehandStrokeStrategy *clone = new FreehandStrokeStrategy(*this);
    clone->setUndoEnabled(false);
    return clone;
}
