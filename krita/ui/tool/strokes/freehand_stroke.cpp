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



FreehandStrokeStrategy::FreehandStrokeStrategy(bool needsIndirectPainting,
                                               KisResourcesSnapshotSP resources,
                                               PainterInfo *painterInfo)
    : KisPainterBasedStrokeStrategy("FREEHAND_STROKE", "Freehand stroke",
                                    resources, painterInfo)
{
    init(needsIndirectPainting);
}

FreehandStrokeStrategy::FreehandStrokeStrategy(bool needsIndirectPainting,
                                               KisResourcesSnapshotSP resources,
                                               QVector<PainterInfo*> painterInfos)
    : KisPainterBasedStrokeStrategy("FREEHAND_STROKE", "Freehand stroke",
                                    resources, painterInfos)
{
    init(needsIndirectPainting);
}

void FreehandStrokeStrategy::init(bool needsIndirectPainting)
{
    setNeedsIndirectPainting(needsIndirectPainting);
    enableJob(KisSimpleStrokeStrategy::JOB_DOSTROKE);
}

void FreehandStrokeStrategy::doStrokeCallback(KisStrokeJobData *data)
{
    Data *d = dynamic_cast<Data*>(data);
    PainterInfo *info = d->painterInfo;

    switch(d->type) {
    case Data::POINT:
        *info->dragDistance = KisDistanceInformation(0,0);
        info->painter->paintAt(d->pi1);
        d->node->setDirty(info->painter->takeDirtyRegion());
        break;
    case Data::LINE:
        *info->dragDistance =
            info->painter->paintLine(d->pi1, d->pi2, *info->dragDistance);
        d->node->setDirty(info->painter->takeDirtyRegion());
        break;
    case Data::CURVE:
        *info->dragDistance =
            info->painter->paintBezierCurve(d->pi1,
                                            d->control1,
                                            d->control2,
                                            d->pi2,
                                            *info->dragDistance);
        d->node->setDirty(info->painter->takeDirtyRegion());
        break;
    };
}
