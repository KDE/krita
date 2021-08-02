/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_FILTER_STROKE_STRATEGY_H
#define __KIS_FILTER_STROKE_STRATEGY_H

#include "kis_stroke_strategy_undo_command_based.h"
#include "kis_types.h"
#include "kis_lod_transform.h"
#include "kis_resources_snapshot.h"

class KRITAUI_EXPORT KisFilterStrokeStrategy : public KisStrokeStrategyUndoCommandBased
{
public:
    class FilterJobData : public KisStrokeJobData {
    public:
        FilterJobData(int frameTime = -1)
            : KisStrokeJobData(CONCURRENT),
              frameTime(frameTime)
        {}

        KisStrokeJobData* createLodClone(int levelOfDetail) override {
            return new FilterJobData(*this, levelOfDetail);
        }

        int frameTime;

    private:
        FilterJobData(const FilterJobData &rhs, int levelOfDetail)
            : KisStrokeJobData(rhs)
            , frameTime(rhs.frameTime)
         {
             KisLodTransform t(levelOfDetail);
         }
    };

    class IdleBarrierData : public KisStrokeJobData {
    public:
        IdleBarrierData()
            : KisStrokeJobData(SEQUENTIAL),
              m_idleBarrierCookie(new std::tuple<>())
        {
        }

        KisStrokeJobData* createLodClone(int levelOfDetail) override {
            return new IdleBarrierData(*this, levelOfDetail);
        }

        using IdleBarrierCookie = QWeakPointer<std::tuple<>>;

        IdleBarrierCookie idleBarrierCookie() const {
            return m_idleBarrierCookie;
        }


    private:
        IdleBarrierData(IdleBarrierData &rhs, int /*levelOfDetail*/)
            : KisStrokeJobData(rhs)
         {
            // the cookie is used for preview only, therefore in
            // instant preview mode we pass it to the lodn stroke
            rhs.m_idleBarrierCookie.swap(m_idleBarrierCookie);
         }

        QSharedPointer<std::tuple<>> m_idleBarrierCookie;
    };

    struct ExternalCancelUpdatesStorage {
        QRect updateRect;
        QRect cancelledLod0UpdateRect;
        QAtomicInt shouldIssueCancellationUpdates;
    };

    using ExternalCancelUpdatesStorageSP = QSharedPointer<ExternalCancelUpdatesStorage>;

public:
    KisFilterStrokeStrategy(KisFilterSP filter,
                            KisFilterConfigurationSP filterConfig,
                            KisResourcesSnapshotSP resources);

    KisFilterStrokeStrategy(KisFilterSP filter,
                            KisFilterConfigurationSP filterConfig,
                            KisResourcesSnapshotSP resources,
                            ExternalCancelUpdatesStorageSP externalCancelUpdatesStorage);
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
