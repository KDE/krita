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

    KUndo2MagicString name() const;
    bool hasJobs() const;
    qint32 numJobs() const;
    KisStrokeJob* popOneJob();

    void endStroke();
    void cancelStroke();

    bool supportsSuspension();
    void suspendStroke(KisStrokeSP recipient);

    bool isInitialized() const;
    bool isEnded() const;

    bool isExclusive() const;
    bool supportsWrapAroundMode() const;
    int worksOnLevelOfDetail() const;

    bool prevJobSequential() const;
    bool nextJobSequential() const;

    bool nextJobBarrier() const;

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
                 bool isCancellable);

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
    bool m_prevJobSequential;

    int m_worksOnLevelOfDetail;
    Type m_type;
    KisStrokeSP m_lodBuddy;
};

#endif /* __KIS_STROKE_H */
