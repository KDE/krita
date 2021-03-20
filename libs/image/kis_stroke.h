/*
 *  SPDX-FileCopyrightText: 2011 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_STROKE_H
#define __KIS_STROKE_H

#include <QQueue>
#include <QScopedPointer>

#include <kis_types.h>
#include "kritaimage_export.h"
#include "kis_stroke_job.h"

class KisStrokeStrategy;
class KUndo2MagicString;


class KRITAIMAGE_EXPORT KisStroke
{
public:
    enum Type {
        LEGACY,
        LOD0,
        LODN,
        SUSPEND,
        RESUME
    };

public:
    KisStroke(KisStrokeStrategy *strokeStrategy, Type type = LEGACY, int levelOfDetail = 0);
    ~KisStroke();

    void addJob(KisStrokeJobData *data);
    void addMutatedJobs(const QVector<KisStrokeJobData *> list);

    KUndo2MagicString name() const;
    QString id() const;

    bool hasJobs() const;
    qint32 numJobs() const;
    KisStrokeJob* popOneJob();

    void endStroke();
    void cancelStroke();

    bool canCancel() const;

    bool supportsSuspension();
    void suspendStroke(KisStrokeSP recipient);

    bool isInitialized() const;
    bool isEnded() const;
    bool isCancelled() const;

    bool isExclusive() const;
    bool supportsWrapAroundMode() const;
    int worksOnLevelOfDetail() const;
    bool canForgetAboutMe() const;
    qreal balancingRatioOverride() const;

    KisStrokeJobData::Sequentiality nextJobSequentiality() const;

    int nextJobLevelOfDetail() const;

    void setLodBuddy(KisStrokeSP buddy);
    KisStrokeSP lodBuddy() const;

    Type type() const;

private:
    void enqueue(KisStrokeJobStrategy *strategy,
                 KisStrokeJobData *data);

    // for suspend/resume jobs
    void prepend(KisStrokeJobStrategy *strategy,
                 KisStrokeJobData *data,
                 int levelOfDetail,
                 bool isOwnJob);

    KisStrokeJob* dequeue();

    void clearQueueOnCancel();
    bool sanityCheckAllJobsAreCancellable() const;

private:
    // for testing use only, do not use in real code
    friend class KisStrokeTest;
    friend class KisStrokeStrategyUndoCommandBasedTest;
    QQueue<KisStrokeJob*>& testingGetQueue() {
        return m_jobsQueue;
    }

private:
    // the strategies are owned by the stroke
    QScopedPointer<KisStrokeStrategy> m_strokeStrategy;
    QScopedPointer<KisStrokeJobStrategy> m_initStrategy;
    QScopedPointer<KisStrokeJobStrategy> m_dabStrategy;
    QScopedPointer<KisStrokeJobStrategy> m_cancelStrategy;
    QScopedPointer<KisStrokeJobStrategy> m_finishStrategy;
    QScopedPointer<KisStrokeJobStrategy> m_suspendStrategy;
    QScopedPointer<KisStrokeJobStrategy> m_resumeStrategy;

    QQueue<KisStrokeJob*> m_jobsQueue;
    bool m_strokeInitialized;
    bool m_strokeEnded;
    bool m_strokeSuspended;
    bool m_isCancelled; // cancelled strokes are always 'ended' as well

    int m_worksOnLevelOfDetail;
    Type m_type;
    KisStrokeSP m_lodBuddy;
};

#endif /* __KIS_STROKE_H */
