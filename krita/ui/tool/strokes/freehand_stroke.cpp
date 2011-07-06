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
#include "kis_base_stroke_job_strategies.h"
#include "kis_painter.h"


FreehandStrokeStrategy::FreehandStrokeStrategy(bool needsIndirectPainting,
                                               KisResourcesSnapshotSP resources,
                                               KisPainter *painter)
    : KisStrokeStrategy("FREEHAND_STROKE", "Freehand stroke"),
      m_resources(resources),
      m_painter(painter)
{
    setNeedsIndirectPainting(needsIndirectPainting);
}

KisDabProcessingStrategy* FreehandStrokeStrategy::createInitStrategy()
{
    return new InitStrokeJobStrategy();
}

KisDabProcessingStrategy* FreehandStrokeStrategy::createFinishStrategy()
{
    return new FinishStrokeJobStrategy();
}

KisDabProcessingStrategy* FreehandStrokeStrategy::createCancelStrategy()
{
    return new CancelStrokeJobStrategy();
}

KisDabProcessingStrategy* FreehandStrokeStrategy::createDabStrategy()
{
    return new FreehandStrokeJobStrategy();
}

KisDabProcessingStrategy::DabProcessingData*
FreehandStrokeStrategy::createInitData()
{
    return new InitStrokeJobStrategy::Data(m_painter, m_resources,
                                           needsIndirectPainting(),
                                           name());
}

KisDabProcessingStrategy::DabProcessingData*
FreehandStrokeStrategy::createFinishData()
{
    return new FinishStrokeJobStrategy::Data(m_painter, m_resources);
}

KisDabProcessingStrategy::DabProcessingData*
FreehandStrokeStrategy::createCancelData()
{
    return new CancelStrokeJobStrategy::Data(m_painter, m_resources);
}


////////////////////////////////////////////////////////////////////////

FreehandStrokeJobStrategy::FreehandStrokeJobStrategy()
  : KisDabProcessingStrategy(true, false)
{
}

void FreehandStrokeJobStrategy::processDab(DabProcessingData *data)
{
    Data *internalData = dynamic_cast<Data*>(data);

    switch(internalData->type) {
    case Data::POINT:
        internalData->dragDistance = KisDistanceInformation(0,0);
        internalData->painter->paintAt(internalData->pi1);
        internalData->node->setDirty(internalData->painter->takeDirtyRegion());
        break;
    case Data::LINE:
        internalData->dragDistance =
            internalData->painter->paintLine(internalData->pi1,
                                             internalData->pi2,
                                             internalData->dragDistance);
        internalData->node->setDirty(internalData->painter->takeDirtyRegion());
        break;
    case Data::CURVE:
        internalData->dragDistance =
            internalData->painter->paintBezierCurve(internalData->pi1,
                                                    internalData->control1,
                                                    internalData->control2,
                                                    internalData->pi2,
                                                    internalData->dragDistance);
        internalData->node->setDirty(internalData->painter->takeDirtyRegion());
        break;
    };
}
