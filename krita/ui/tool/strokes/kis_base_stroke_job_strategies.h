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

#ifndef __KIS_STROKE_JOB_STRATEGIES_H
#define __KIS_STROKE_JOB_STRATEGIES_H

#include "kis_dab_processing_strategy.h"
#include "kis_resources_snapshot.h"

class KisPainter;


class InitStrokeJobStrategy : public KisDabProcessingStrategy
{
public:
    class Data : public DabProcessingData {
    public:
        Data(KisPainter *_painter,
             KisResourcesSnapshotSP _resources,
             bool _needsIndirectPainting,
             const QString &_transactionText)
            : painter(_painter),
              resources(_resources),
              needsIndirectPainting(_needsIndirectPainting),
              transactionText(_transactionText)
        {}

        KisPainter *painter;
        KisResourcesSnapshotSP resources;
        bool needsIndirectPainting;
        QString transactionText;
    };

public:
    InitStrokeJobStrategy(bool isExclusive = false);

    void processDab(DabProcessingData *data);
};

class FinishStrokeJobStrategy : public KisDabProcessingStrategy
{
public:
    class Data : public DabProcessingData {
    public:
        Data(KisPainter *_painter,
             KisResourcesSnapshotSP _resources)
            : painter(_painter),
              resources(_resources)
        {}

        KisPainter *painter;
        KisResourcesSnapshotSP resources;
    };

public:
    FinishStrokeJobStrategy(bool isExclusive = false);

    void processDab(DabProcessingData *data);
};

class CancelStrokeJobStrategy : public KisDabProcessingStrategy
{
public:
    class Data : public DabProcessingData {
    public:
        Data(KisPainter *_painter,
             KisResourcesSnapshotSP _resources)
            : painter(_painter),
              resources(_resources)
        {}

        KisPainter *painter;
        KisResourcesSnapshotSP resources;
    };

public:
    CancelStrokeJobStrategy(bool isExclusive = false);

    void processDab(DabProcessingData *data);
};

#endif /* __KIS_STROKE_JOB_STRATEGIES_H */
