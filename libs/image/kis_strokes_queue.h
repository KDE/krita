/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_STROKES_QUEUE_H
#define __KIS_STROKES_QUEUE_H

#include "kritaimage_export.h"
#include "kundo2magicstring.h"
#include "kis_types.h"
#include "kis_stroke_job_strategy.h"
#include "kis_stroke_strategy.h"
#include "kis_stroke_strategy_factory.h"
#include "kis_strokes_queue_undo_result.h"
#include "KisStrokesQueueMutatedJobInterface.h"
#include "KisUpdaterContextSnapshotEx.h"
#include "KisLodPreferences.h"

class KisUpdaterContext;
class KisStroke;
class KisStrokeStrategy;
class KisStrokeJobData;
class KisPostExecutionUndoAdapter;


class KRITAIMAGE_EXPORT KisStrokesQueue : public KisStrokesQueueMutatedJobInterface
{
public:
    KisStrokesQueue();
    ~KisStrokesQueue();

    KisStrokeId startStroke(KisStrokeStrategy *strokeStrategy);
    void addJob(KisStrokeId id, KisStrokeJobData *data);

    void endStroke(KisStrokeId id);
    bool cancelStroke(KisStrokeId id);

    bool tryCancelCurrentStrokeAsync();

    UndoResult tryUndoLastStrokeAsync();

    void processQueue(KisUpdaterContext &updaterContext,
                      bool externalJobsPending);
    bool needsExclusiveAccess() const;
    bool isEmpty() const;

    qint32 sizeMetric() const;
    KUndo2MagicString currentStrokeName() const;
    bool hasOpenedStrokes() const;

    bool wrapAroundModeSupported() const;
    qreal balancingRatioOverride() const;

    KisLodPreferences lodPreferences() const override;
    void setLodPreferences(const KisLodPreferences &value);
    void explicitRegenerateLevelOfDetail();
    void setLod0ToNStrokeStrategyFactory(const KisLodSyncStrokeStrategyFactory &factory);
    void setSuspendResumeUpdatesStrokeStrategyFactory(const KisSuspendResumeStrategyPairFactory &factory);
    KisPostExecutionUndoAdapter* lodNPostExecutionUndoAdapter() const;

    /**
     * Notifies the queue, that someone else (neither strokes nor the
     * queue itself have changed the image. It means that the caches
     * should be regenerated
     */
    void notifyUFOChangedImage();

    void debugDumpAllStrokes();

    // interface for KisStrokeStrategy only!
    void addMutatedJobs(KisStrokeId id, const QVector<KisStrokeJobData*> list) final override;

private:
    bool processOneJob(KisUpdaterContext &updaterContext,
                       bool externalJobsPending);
    bool checkStrokeState(bool hasStrokeJobsRunning,
                          int runningLevelOfDetail);
    bool checkExclusiveProperty(bool hasMergeJobs, bool hasStrokeJobs);
    bool checkSequentialProperty(KisUpdaterContextSnapshotEx snapshot, bool externalJobsPending);
    bool checkBarrierProperty(bool hasMergeJobs, bool hasStrokeJobs,
                              bool externalJobsPending);
    bool checkLevelOfDetailProperty(int runningLevelOfDetail);

    class LodNUndoStrokesFacade;
    KisStrokeId startLodNUndoStroke(KisStrokeStrategy *strokeStrategy);

private:
    struct Private;
    Private * const m_d;
};

#endif /* __KIS_STROKES_QUEUE_H */
