/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

    class ExtraCleanUpUpdates : public KisStrokeJobData {
    public:
        ExtraCleanUpUpdates(const QVector<QRect> &_rects)
            : KisStrokeJobData(SEQUENTIAL),
              rects(_rects)
        {}

        ExtraCleanUpUpdates(const ExtraCleanUpUpdates &rhs, int levelOfDetail)
            : KisStrokeJobData(rhs)
        {
            KisLodTransform t(levelOfDetail);

            Q_FOREACH (const QRect &rc, rhs.rects) {
                rects << t.map(rc);
            }
        }

        KisStrokeJobData* createLodClone(int levelOfDetail) override {
            return new ExtraCleanUpUpdates(*this, levelOfDetail);
        }

        QVector<QRect> rects;
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

    QSharedPointer<QAtomicInt> cancelSilentlyHandle() const;

private:
    struct Private;
    Private* const m_d;
};

#endif /* __KIS_FILTER_STROKE_STRATEGY_H */
