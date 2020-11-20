/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_SYNC_LOD_CACHE_STROKE_STRATEGY_H
#define __KIS_SYNC_LOD_CACHE_STROKE_STRATEGY_H

#include "kritaimage_export.h"
#include <KisRunnableBasedStrokeStrategy.h>

#include <QScopedPointer>

class KRITAIMAGE_EXPORT KisSyncLodCacheStrokeStrategy : public KisRunnableBasedStrokeStrategy
{
public:
    KisSyncLodCacheStrokeStrategy(KisImageWSP image, bool forgettable);
    ~KisSyncLodCacheStrokeStrategy() override;

    static QList<KisStrokeJobData*> createJobsData(KisImageWSP image);

    static void createJobsData(QVector<KisStrokeJobData *> &jobs, KisNodeSP imageRoot, int levelOfDetail, KisPaintDeviceList extraDevices = {});

private:
    void doStrokeCallback(KisStrokeJobData *data) override;
    void initStrokeCallback() override;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_SYNC_LOD_CACHE_STROKE_STRATEGY_H */
