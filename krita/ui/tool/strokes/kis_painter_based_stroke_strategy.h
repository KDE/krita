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

#ifndef __KIS_PAINTER_BASED_STROKE_STRATEGY_H
#define __KIS_PAINTER_BASED_STROKE_STRATEGY_H

#include "kis_stroke_strategy.h"
#include "kis_resources_snapshot.h"

class KisPainter;


class KRITAUI_EXPORT KisPainterBasedStrokeStrategy : public KisStrokeStrategy
{
public:
    KisPainterBasedStrokeStrategy(const QString &id,
                                  const QString &name,
                                  KisResourcesSnapshotSP resources,
                                  KisPainter *painter);

    ~KisPainterBasedStrokeStrategy();

    KisDabProcessingStrategy* createInitStrategy();
    KisDabProcessingStrategy* createFinishStrategy();
    KisDabProcessingStrategy* createCancelStrategy();

    KisDabProcessingStrategy::DabProcessingData* createInitData();
    KisDabProcessingStrategy::DabProcessingData* createFinishData();
    KisDabProcessingStrategy::DabProcessingData* createCancelData();

private:
    KisResourcesSnapshotSP m_resources;
    KisPainter *m_painter;
};

#endif /* __KIS_PAINTER_BASED_STROKE_STRATEGY_H */
