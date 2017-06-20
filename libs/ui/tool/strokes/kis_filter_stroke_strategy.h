/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef __KIS_FILTER_STROKE_STRATEGY_H
#define __KIS_FILTER_STROKE_STRATEGY_H

#include "kis_types.h"
#include "kis_painter_based_stroke_strategy.h"
#include "kis_lod_transform.h"


class KRITAUI_EXPORT KisFilterStrokeStrategy : public KisPainterBasedStrokeStrategy
{
public:
    class Data : public KisStrokeJobData {
    public:
        Data(const QRect &_processRect, bool concurrent)
            : KisStrokeJobData(concurrent ? CONCURRENT : SEQUENTIAL),
              processRect(_processRect) {}

        KisStrokeJobData* createLodClone(int levelOfDetail) override {
            return new Data(*this, levelOfDetail);
        }

        QRect processRect;

    private:
        Data(const Data &rhs, int levelOfDetail)
            : KisStrokeJobData(rhs)
         {
             KisLodTransform t(levelOfDetail);
             processRect = t.map(rhs.processRect);
         }

    };

    class CancelSilentlyMarker : public KisStrokeJobData {
    public:
        CancelSilentlyMarker()
            : KisStrokeJobData(SEQUENTIAL)
        {}

        KisStrokeJobData* createLodClone(int /*levelOfDetail*/) override {
            return new CancelSilentlyMarker(*this);
        }
    };

public:
    KisFilterStrokeStrategy(KisFilterSP filter,
                            KisFilterConfigurationSP filterConfig,
                            KisResourcesSnapshotSP resources);
    KisFilterStrokeStrategy(const KisFilterStrokeStrategy &rhs, int levelOfDetail);

    ~KisFilterStrokeStrategy() override;


    void initStrokeCallback() override;
    void doStrokeCallback(KisStrokeJobData *data) override;
    void cancelStrokeCallback() override;
    void finishStrokeCallback() override;

    KisStrokeStrategy* createLodClone(int levelOfDetail) override;

private:
    struct Private;
    Private* const m_d;
};

#endif /* __KIS_FILTER_STROKE_STRATEGY_H */
