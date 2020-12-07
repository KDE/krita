/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_SYNC_LOD_CACHE_STROKE_STRATEGY_H
#define __KIS_SYNC_LOD_CACHE_STROKE_STRATEGY_H

#include <kis_simple_stroke_strategy.h>

#include <QScopedPointer>

class KisSyncLodCacheStrokeStrategy : public KisSimpleStrokeStrategy
{
public:
    KisSyncLodCacheStrokeStrategy(KisImageWSP image, bool forgettable);
    ~KisSyncLodCacheStrokeStrategy() override;

    static QList<KisStrokeJobData*> createJobsData(KisImageWSP image);

private:
    void doStrokeCallback(KisStrokeJobData *data) override;
    void finishStrokeCallback() override;
    void cancelStrokeCallback() override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_SYNC_LOD_CACHE_STROKE_STRATEGY_H */
