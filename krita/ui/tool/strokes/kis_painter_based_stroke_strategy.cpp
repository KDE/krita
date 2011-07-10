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

#include "kis_painter_based_stroke_strategy.h"

#include "kis_base_stroke_job_strategies.h"
#include "kis_painter.h"

KisPainterBasedStrokeStrategy::KisPainterBasedStrokeStrategy(const QString &id,
                                                             const QString &name,
                                                             KisResourcesSnapshotSP resources,
                                                             KisPainter *painter)
    : KisStrokeStrategy(id, name),
      m_resources(resources),
      m_painter(painter)
{
}

KisPainterBasedStrokeStrategy::~KisPainterBasedStrokeStrategy()
{
}

KisDabProcessingStrategy* KisPainterBasedStrokeStrategy::createInitStrategy()
{
    return new InitStrokeJobStrategy();
}

KisDabProcessingStrategy* KisPainterBasedStrokeStrategy::createFinishStrategy()
{
    return new FinishStrokeJobStrategy();
}

KisDabProcessingStrategy* KisPainterBasedStrokeStrategy::createCancelStrategy()
{
    return new CancelStrokeJobStrategy();
}

KisDabProcessingStrategy::DabProcessingData*
KisPainterBasedStrokeStrategy::createInitData()
{
    return new InitStrokeJobStrategy::Data(m_painter, m_resources,
                                           needsIndirectPainting(),
                                           name());
}

KisDabProcessingStrategy::DabProcessingData*
KisPainterBasedStrokeStrategy::createFinishData()
{
    return new FinishStrokeJobStrategy::Data(m_painter, m_resources);
}

KisDabProcessingStrategy::DabProcessingData*
KisPainterBasedStrokeStrategy::createCancelData()
{
    return new CancelStrokeJobStrategy::Data(m_painter, m_resources);
}
